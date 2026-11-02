// main.cpp - local HTTP server + REST API for the Smart Study Group Formation System
// Pure C++17, sockets only (Winsock on Windows, POSIX on Linux). No external libraries.
#ifdef _WIN32
  #define NOMINMAX
  #define WIN32_LEAN_AND_MEAN
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <set>
#include <algorithm>
#include <filesystem>
#include <ctime>
#include <atomic>

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib, "ws2_32.lib")
  typedef SOCKET sock_t;
  #define CLOSESOCK closesocket
  #define SEND_FLAGS 0
#else
  #include <sys/socket.h>
  #include <netinet/in.h>
  #include <arpa/inet.h>
  #include <unistd.h>
  typedef int sock_t;
  #define INVALID_SOCKET (-1)
  #define CLOSESOCK close
  #define SEND_FLAGS MSG_NOSIGNAL
#endif

#include "Json.h"
#include "Student.h"
#include "Compatibility.h"
#include "Graph.h"
#include "DSU.h"
#include "GreedyGrouping.h"
#include "BFSGrouping.h"

namespace fs = std::filesystem;

// =====================================================================
//  Application state
// =====================================================================
static StudentStore store("data/students.json");

struct GroupMeta { int id; std::vector<int> memberIds; };
static std::vector<GroupMeta> savedGroups;
static int nextGroupId = 1;
static std::string groupsAlgorithm = "", groupsTime = "";
static double groupsThreshold = 50; static int groupsMaxSize = 4;
static const char* GROUPS_FILE = "data/groups.json";
static std::atomic<bool> running(true);

static void saveGroups() {
    Json j = Json::object();
    j["nextId"] = nextGroupId; j["algorithm"] = groupsAlgorithm; j["generatedAt"] = groupsTime;
    j["threshold"] = groupsThreshold; j["maxSize"] = groupsMaxSize;
    Json arr = Json::array();
    for (auto& g : savedGroups) {
        Json x = Json::object(); x["id"] = g.id;
        Json m = Json::array(); for (int id : g.memberIds) m.push(Json(id));
        x["members"] = m; arr.push(x);
    }
    j["groups"] = arr;
    std::ofstream f(GROUPS_FILE, std::ios::trunc); f << j.dump();
}
static void loadGroups() {
    std::ifstream f(GROUPS_FILE);
    if (!f) return;
    std::stringstream ss; ss << f.rdbuf();
    bool ok; Json j = Json::parse(ss.str(), &ok);
    if (!ok) return;
    nextGroupId = j.getInt("nextId", 1); groupsAlgorithm = j.getStr("algorithm");
    groupsTime = j.getStr("generatedAt"); groupsThreshold = j.getNum("threshold", 50); groupsMaxSize = j.getInt("maxSize", 4);
    const Json* arr = j.find("groups");
    if (arr) for (auto& x : arr->a) {
        GroupMeta g; g.id = x.getInt("id");
        const Json* m = x.find("members");
        if (m) for (auto& v : m->a) g.memberIds.push_back((int)v.n);
        savedGroups.push_back(g);
    }
}
static std::string nowString() {
    char buf[64]; time_t t = time(nullptr);
    strftime(buf, sizeof buf, "%d %b %Y, %H:%M", localtime(&t));
    return buf;
}

// =====================================================================
//  Grouping helpers
// =====================================================================
static std::vector<std::vector<int>> runAlgorithm(const std::string& algo, const Graph& g, int maxSize) {
    auto& st = store.all();
    return algo == "bfs" ? bfsGroup(st, g, maxSize) : greedyGroup(st, g, maxSize);
}

// Build JSON for a list of groups (each = list of Student pointers) + metrics
static Json groupsPayload(const std::vector<std::vector<const Student*>>& groups, const std::vector<int>& ids) {
    Json out = Json::object();
    Json arr = Json::array();
    double sum = 0; int assigned = 0;
    std::set<std::string> covered, allSubjects;
    for (auto& s : store.all()) for (auto& x : s.subjects) allSubjects.insert(toLower(x));
    for (size_t i = 0; i < groups.size(); i++) {
        Json d = describeGroup(groups[i]);
        d["id"] = ids[i]; d["number"] = (int)i + 1;
        sum += d.getNum("compatibility"); assigned += (int)groups[i].size();
        const Json* sh = d.find("sharedSubjects");
        if (sh) for (auto& x : sh->a) covered.insert(toLower(x.getStr("subject")));
        arr.push(d);
    }
    out["groups"] = arr;
    Json m = Json::object();
    m["groups"] = (int)groups.size(); m["studentsAssigned"] = assigned; m["totalStudents"] = (int)store.all().size();
    m["unassigned"] = (int)store.all().size() - assigned;
    m["avgCompatibility"] = groups.empty() ? 0.0 : (int)(sum / groups.size() * 10 + 0.5) / 10.0;
    m["subjectsCovered"] = (int)covered.size(); m["totalSubjects"] = (int)allSubjects.size();
    m["coveragePct"] = allSubjects.empty() ? 0.0 : (int)(100.0 * covered.size() / allSubjects.size() * 10 + 0.5) / 10.0;
    out["metrics"] = m;
    return out;
}
static Json indexGroupsPayload(const std::vector<std::vector<int>>& idx) {
    std::vector<std::vector<const Student*>> gs; std::vector<int> ids;
    for (size_t i = 0; i < idx.size(); i++) {
        std::vector<const Student*> p; for (int k : idx[i]) p.push_back(&store.all()[k]);
        gs.push_back(p); ids.push_back((int)i + 1);
    }
    return groupsPayload(gs, ids);
}
static Json savedGroupsPayload() {
    std::vector<std::vector<const Student*>> gs; std::vector<int> ids;
    for (auto& g : savedGroups) {
        std::vector<const Student*> p;
        for (int id : g.memberIds) { Student* s = store.find(id); if (s) p.push_back(s); }
        if (p.size() < 2) continue;
        gs.push_back(p); ids.push_back(g.id);
    }
    Json r = groupsPayload(gs, ids);
    r["algorithm"] = groupsAlgorithm; r["generatedAt"] = groupsTime;
    r["threshold"] = groupsThreshold; r["maxSize"] = groupsMaxSize;
    return r;
}
// drop deleted students and dissolve groups with fewer than 2 members
static void cleanGroups() {
    std::vector<GroupMeta> keep;
    for (auto& g : savedGroups) {
        GroupMeta n; n.id = g.id;
        for (int id : g.memberIds) if (store.find(id)) n.memberIds.push_back(id);
        if (n.memberIds.size() >= 2) keep.push_back(n);
    }
    savedGroups = keep; saveGroups();
}

// =====================================================================
//  REST API
// =====================================================================
struct Response { int status = 200; std::string type = "application/json"; std::string body; };

static Response jsonResp(const Json& j, int status = 200) { Response r; r.status = status; r.body = j.dump(); return r; }
static Response errResp(const std::string& msg, int status = 400) { Json j = Json::object(); j["error"] = msg; return jsonResp(j, status); }

static std::map<std::string, std::string> parseQuery(const std::string& q) {
    std::map<std::string, std::string> m; std::stringstream ss(q); std::string kv;
    while (std::getline(ss, kv, '&')) { size_t e = kv.find('='); if (e != std::string::npos) m[kv.substr(0, e)] = kv.substr(e + 1); }
    return m;
}
static std::vector<std::string> splitPath(const std::string& p) {
    std::vector<std::string> r; std::stringstream ss(p); std::string x;
    while (std::getline(ss, x, '/')) if (!x.empty()) r.push_back(x);
    return r;
}

static Json buildStats() {
    auto& st = store.all();
    Json j = Json::object();
    j["totalStudents"] = (int)st.size();
    Json saved = savedGroupsPayload();
    const Json* mt = saved.find("metrics");
    j["totalGroups"] = mt->getInt("groups");
    j["successfulMatches"] = mt->getInt("studentsAssigned");
    j["unassigned"] = (int)st.size() - mt->getInt("studentsAssigned");
    j["avgCompatibility"] = mt->getNum("avgCompatibility");
    j["matchRate"] = st.empty() ? 0.0 : (int)(100.0 * mt->getInt("studentsAssigned") / st.size() * 10 + 0.5) / 10.0;
    j["algorithm"] = groupsAlgorithm; j["generatedAt"] = groupsTime;
    j["coveragePct"] = mt->getNum("coveragePct");

    std::map<std::string, int> cnt; std::map<std::string, std::string> shown;
    std::map<std::string, int> lv, dept;
    for (auto& s : st) {
        for (auto& x : s.subjects) { cnt[toLower(x)]++; shown[toLower(x)] = x; }
        lv[s.level]++; dept[s.department]++;
    }
    std::vector<std::pair<int, std::string>> pop;
    for (auto& kv : cnt) pop.push_back({kv.second, shown[kv.first]});
    std::sort(pop.begin(), pop.end(), [](auto& a, auto& b) { return a.first != b.first ? a.first > b.first : a.second < b.second; });
    Json pj = Json::array();
    for (size_t i = 0; i < pop.size() && i < 8; i++) { Json x = Json::object(); x["subject"] = pop[i].second; x["count"] = pop[i].first; pj.push(x); }
    j["popularSubjects"] = pj;
    Json lj = Json::object(); lj["Beginner"] = lv["Beginner"]; lj["Intermediate"] = lv["Intermediate"]; lj["Advanced"] = lv["Advanced"];
    j["levels"] = lj;
    Json rec = Json::array();
    for (int i = (int)st.size() - 1, c = 0; i >= 0 && c < 5; i--, c++) rec.push(st[i].toJson());
    j["recent"] = rec;
    return j;
}

static Response api(const std::string& method, const std::string& path, const std::string& queryStr, const std::string& body) {
    std::vector<std::string> p = splitPath(path);   // p[0] == "api"
    auto q = parseQuery(queryStr);
    if (p.size() < 2) return errResp("Unknown API route", 404);
    const std::string& res = p[1];
    auto& st = store.all();

    // ---------- students ----------
    if (res == "students") {
        if (p.size() == 2 && method == "GET") {
            Json a = Json::array(); for (auto& s : st) a.push(s.toJson());
            return jsonResp(a);
        }
        if (p.size() == 2 && method == "POST") {
            bool ok; Json j = Json::parse(body, &ok);
            if (!ok) return errResp("Invalid JSON body");
            Student s = Student::fromJson(j);
            std::string e = store.validate(s, -1);
            if (!e.empty()) return errResp(e);
            return jsonResp(store.add(s).toJson(), 201);
        }
        if (p.size() == 3) {
            int id = atoi(p[2].c_str());
            Student* cur = store.find(id);
            if (!cur) return errResp("Student not found", 404);
            if (method == "GET") return jsonResp(cur->toJson());
            if (method == "PUT") {
                bool ok; Json j = Json::parse(body, &ok);
                if (!ok) return errResp("Invalid JSON body");
                Student s = Student::fromJson(j);
                std::string e = store.validate(s, id);
                if (!e.empty()) return errResp(e);
                store.update(id, s);
                return jsonResp(store.find(id)->toJson());
            }
            if (method == "DELETE") {
                store.remove(id); cleanGroups();
                Json j = Json::object(); j["deleted"] = id; return jsonResp(j);
            }
        }
    }
    // ---------- dashboard ----------
    if (res == "stats" && method == "GET") return jsonResp(buildStats());

    // ---------- compatibility of one student with everybody else ----------
    if (res == "compatibility" && p.size() == 3 && method == "GET") {
        Student* me = store.find(atoi(p[2].c_str()));
        if (!me) return errResp("Student not found", 404);
        std::vector<std::pair<double, Json>> rows;
        for (auto& o : st) {
            if (o.id == me->id) continue;
            Compat c = computeCompat(*me, o);
            Json r = c.toJson(); r["student"] = o.toJson();
            rows.push_back({c.total, r});
        }
        std::stable_sort(rows.begin(), rows.end(), [](auto& a, auto& b) { return a.first > b.first; });
        Json a = Json::array(); for (auto& r : rows) a.push(r.second);
        Json out = Json::object(); out["student"] = me->toJson(); out["matches"] = a;
        return jsonResp(out);
    }

    // ---------- graph (adjacency list + DSU / BFS component counts) ----------
    if (res == "graph" && method == "GET") {
        double th = q.count("threshold") ? atof(q["threshold"].c_str()) : 50;
        Graph g(st, th);
        Json out = Json::object();
        Json nodes = Json::array();
        for (auto& s : st) { Json x = Json::object(); x["id"] = s.id; x["name"] = s.name; x["level"] = s.level; nodes.push(x); }
        Json edges = Json::array();
        DSU dsu((int)st.size());
        for (auto& e : g.edges()) {
            Json x = Json::object(); x["from"] = st[e.u].id; x["to"] = st[e.v].id; x["weight"] = e.weight; edges.push(x);
            dsu.unite(e.u, e.v);
        }
        Json adj = Json::array();
        for (int u = 0; u < g.vertices(); u++) {
            Json x = Json::object(); x["id"] = st[u].id; x["name"] = st[u].name;
            Json nb = Json::array();
            for (auto& e : g.neighbors(u)) { Json y = Json::object(); y["id"] = st[e.to].id; y["name"] = st[e.to].name; y["weight"] = e.weight; nb.push(y); }
            x["neighbors"] = nb; adj.push(x);
        }
        out["nodes"] = nodes; out["edges"] = edges; out["adjacency"] = adj; out["threshold"] = th;
        out["edgeCount"] = g.edgeCount();
        out["dsuComponents"] = dsu.countSets(); out["bfsComponents"] = bfsComponentCount(g);
        return jsonResp(out);
    }

    // ---------- generate / compare groups ----------
    if (res == "groups") {
        if (p.size() == 3 && p[2] == "generate" && method == "POST") {
            bool ok; Json j = Json::parse(body, &ok);
            if (!ok) return errResp("Invalid JSON body");
            std::string algo = j.getStr("algorithm", "greedy");
            double th = j.getNum("threshold", 50); int maxSize = j.getInt("maxSize", 4);
            if (algo != "greedy" && algo != "bfs") return errResp("Algorithm must be 'greedy' or 'bfs'");
            if (maxSize < 2 || maxSize > 8) return errResp("Maximum group size must be 2 to 8");
            if (st.size() < 2) return errResp("Register at least 2 students first");
            Graph g(st, th);
            auto idx = runAlgorithm(algo, g, maxSize);
            savedGroups.clear();
            for (auto& grp : idx) { GroupMeta m; m.id = nextGroupId++; for (int k : grp) m.memberIds.push_back(st[k].id); savedGroups.push_back(m); }
            groupsAlgorithm = algo; groupsThreshold = th; groupsMaxSize = maxSize; groupsTime = nowString();
            saveGroups();
            return jsonResp(savedGroupsPayload());
        }
        if (p.size() == 3 && p[2] == "compare" && method == "POST") {
            bool ok; Json j = Json::parse(body, &ok);
            if (!ok) return errResp("Invalid JSON body");
            double th = j.getNum("threshold", 50); int maxSize = j.getInt("maxSize", 4);
            Graph g(st, th);
            Json out = Json::object();
            out["greedy"] = indexGroupsPayload(runAlgorithm("greedy", g, maxSize));
            out["bfs"] = indexGroupsPayload(runAlgorithm("bfs", g, maxSize));
            out["threshold"] = th; out["maxSize"] = maxSize; out["edges"] = g.edgeCount();
            return jsonResp(out);
        }
        if (p.size() == 2 && method == "GET") return jsonResp(savedGroupsPayload());
        if (p.size() == 2 && method == "DELETE") { savedGroups.clear(); groupsAlgorithm = ""; saveGroups(); return jsonResp(savedGroupsPayload()); }
        if (p.size() == 5 && p[3] == "members" && method == "DELETE") {
            int gid = atoi(p[2].c_str()), sid = atoi(p[4].c_str());
            for (auto& g : savedGroups) if (g.id == gid) {
                auto it = std::find(g.memberIds.begin(), g.memberIds.end(), sid);
                if (it == g.memberIds.end()) return errResp("Student is not in this group", 404);
                g.memberIds.erase(it);
                cleanGroups();       // groups with <2 members are dissolved
                return jsonResp(savedGroupsPayload());
            }
            return errResp("Group not found", 404);
        }
    }

    if (res == "shutdown" && method == "POST") { running = false; Json j = Json::object(); j["stopping"] = true; return jsonResp(j); }
    return errResp("Unknown API route", 404);
}

// =====================================================================
//  Static file serving
// =====================================================================
static std::string urlDecode(const std::string& s) {
    std::string r;
    for (size_t i = 0; i < s.size(); i++) {
        if (s[i] == '%' && i + 2 < s.size()) { r += (char)strtol(s.substr(i + 1, 2).c_str(), nullptr, 16); i += 2; }
        else if (s[i] == '+') r += ' ';
        else r += s[i];
    }
    return r;
}
static std::string mimeOf(const std::string& path) {
    std::string ext = fs::path(path).extension().string();
    static std::map<std::string, std::string> m = {
        {".html", "text/html; charset=utf-8"}, {".css", "text/css; charset=utf-8"}, {".js", "application/javascript; charset=utf-8"},
        {".json", "application/json"}, {".svg", "image/svg+xml"}, {".png", "image/png"}, {".jpg", "image/jpeg"}, {".ico", "image/x-icon"}};
    return m.count(ext) ? m[ext] : "application/octet-stream";
}
static Response serveStatic(std::string url) {
    url = urlDecode(url);
    if (url == "/" || url.empty()) url = "/index.html";
    Response r;
    if (url.find("..") != std::string::npos) { r.status = 403; r.type = "text/plain"; r.body = "Forbidden"; return r; }
    std::ifstream f("frontend" + url, std::ios::binary);
    if (!f) { r.status = 404; r.type = "text/plain"; r.body = "404 Not Found"; return r; }
    std::stringstream ss; ss << f.rdbuf();
    r.type = mimeOf(url); r.body = ss.str();
    return r;
}

// =====================================================================
//  Minimal HTTP/1.1 server
// =====================================================================
static const char* statusText(int s) {
    switch (s) { case 200: return "OK"; case 201: return "Created"; case 400: return "Bad Request"; case 403: return "Forbidden"; case 404: return "Not Found"; default: return "Error"; }
}
static void sendAll(sock_t c, const std::string& data) {
    size_t sent = 0;
    while (sent < data.size()) {
        int n = (int)send(c, data.data() + sent, (int)(data.size() - sent), SEND_FLAGS);
        if (n <= 0) return;
        sent += n;
    }
}
static void handleClient(sock_t c) {
    // 1-second receive timeout so idle "pre-connect" sockets never block the server
#ifdef _WIN32
    DWORD tv = 1000; setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
#else
    timeval tv{1, 0}; setsockopt(c, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof tv);
#endif
    std::string req; char buf[8192]; size_t headerEnd = std::string::npos;
    while (headerEnd == std::string::npos && req.size() < 65536) {
        int n = (int)recv(c, buf, sizeof buf, 0);
        if (n <= 0) return;
        req.append(buf, n);
        headerEnd = req.find("\r\n\r\n");
    }
    if (headerEnd == std::string::npos) return;
    std::string head = req.substr(0, headerEnd);
    std::istringstream hs(head);
    std::string method, target, version;
    hs >> method >> target >> version;
    size_t contentLength = 0; std::string line;
    std::getline(hs, line);
    while (std::getline(hs, line)) {
        std::string low = toLower(line);
        if (low.rfind("content-length:", 0) == 0) contentLength = (size_t)atoll(line.c_str() + 15);
    }
    std::string body = req.substr(headerEnd + 4);
    while (body.size() < contentLength && contentLength < 2000000) {
        int n = (int)recv(c, buf, sizeof buf, 0);
        if (n <= 0) break;
        body.append(buf, n);
    }
    std::string path = target, query;
    size_t qm = target.find('?');
    if (qm != std::string::npos) { path = target.substr(0, qm); query = target.substr(qm + 1); }

    Response r;
    try {
        if (path.rfind("/api/", 0) == 0) r = api(method, path, query, body);
        else if (method == "GET") r = serveStatic(path);
        else { r.status = 404; r.type = "text/plain"; r.body = "Not found"; }
    } catch (const std::exception& e) { r = errResp(std::string("Server error: ") + e.what(), 500); }

    std::ostringstream out;
    out << "HTTP/1.1 " << r.status << " " << statusText(r.status) << "\r\n"
        << "Content-Type: " << r.type << "\r\nContent-Length: " << r.body.size() << "\r\n"
        << "Cache-Control: no-store\r\nConnection: close\r\n\r\n";
    sendAll(c, out.str() + r.body);
}

int main(int argc, char** argv) {
    int port = 8080;
    for (int i = 1; i < argc; i++) {
        std::string a = argv[i];
        if (a == "--port" && i + 1 < argc) port = atoi(argv[++i]);
        else if (a == "--root" && i + 1 < argc) fs::current_path(argv[++i]);
    }
    // allow double-clicking the exe from inside backend/
    if (!fs::exists("frontend") && fs::exists("../frontend")) fs::current_path("..");
    if (!fs::exists("frontend/index.html")) { std::cerr << "ERROR: 'frontend' folder not found. Run from the project folder (use RUN.bat).\n"; return 1; }
    fs::create_directories("data");

    store.load(); loadGroups();

#ifdef _WIN32
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) { std::cerr << "ERROR: Winsock failed to start.\n"; return 1; }
#endif
    sock_t srv = socket(AF_INET, SOCK_STREAM, 0);
    if (srv == INVALID_SOCKET) { std::cerr << "ERROR: cannot create socket.\n"; return 1; }
#ifndef _WIN32
    int yes = 1; setsockopt(srv, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof yes);
#endif
    sockaddr_in addr{}; addr.sin_family = AF_INET; addr.sin_port = htons((unsigned short)port);
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);       // localhost only (no firewall prompt)
    if (bind(srv, (sockaddr*)&addr, sizeof addr) != 0) {
        std::cerr << "ERROR: cannot start the server on port " << port << ". Is another copy already running? Run STOP.bat and try again.\n";
        return 2;
    }
    listen(srv, 16);
    std::cout << "==============================================\n"
              << "  Smart Study Group Formation System (C++)\n"
              << "  Server running at http://localhost:" << port << "\n"
              << "  Students loaded: " << store.all().size() << "\n"
              << "  Close this window or run STOP.bat to stop.\n"
              << "==============================================\n" << std::flush;

    while (running) {
        sock_t c = accept(srv, nullptr, nullptr);
        if (c == INVALID_SOCKET) continue;
        handleClient(c);
        CLOSESOCK(c);
    }
    CLOSESOCK(srv);
#ifdef _WIN32
    WSACleanup();
#endif
    std::cout << "Server stopped.\n";
    return 0;
}
