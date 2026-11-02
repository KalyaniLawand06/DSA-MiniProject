#include "BFSGrouping.h"
#include "Compatibility.h"
#include <queue>
#include <algorithm>

// BFS from one student returns everybody reachable through edges above the threshold.
std::vector<int> bfsComponent(const Graph& g, int start, std::vector<bool>& visited) {
    std::vector<int> order;
    std::queue<int> q;
    q.push(start); visited[start] = true;
    while (!q.empty()) {
        int u = q.front(); q.pop();
        order.push_back(u);
        for (auto& e : g.neighbors(u))
            if (!visited[e.to]) { visited[e.to] = true; q.push(e.to); }
    }
    return order;
}

int bfsComponentCount(const Graph& g) {
    std::vector<bool> visited(g.vertices(), false);
    int c = 0;
    for (int i = 0; i < g.vertices(); i++)
        if (!visited[i]) { bfsComponent(g, i, visited); c++; }
    return c;
}

// Step 1: BFS finds each connected component (a "cluster" of linked students)
// Step 2: a cluster bigger than the preferred size is cut into pieces in BFS order
//         (students found close together in the BFS stay together)
// Step 3: in every piece keep only the members who study the piece's main subject
std::vector<std::vector<int>> bfsGroup(const std::vector<Student>& students, const Graph& g, int maxSize) {
    std::vector<bool> visited(g.vertices(), false);
    std::vector<std::vector<int>> groups;

    for (int start = 0; start < g.vertices(); start++) {
        if (visited[start]) continue;
        std::vector<int> comp = bfsComponent(g, start, visited);
        if (comp.size() < 2) continue;

        // cut the BFS order into chunks
        std::vector<std::vector<int>> chunks;
        std::vector<int> cur;
        for (int v : comp) {
            std::vector<const Student*> p;
            for (int x : cur) p.push_back(&students[x]);
            p.push_back(&students[v]);
            int limit = std::min(maxSize, minPreferredSize(p));
            if ((int)p.size() > limit && !cur.empty()) { chunks.push_back(cur); cur.clear(); }
            cur.push_back(v);
        }
        if (!cur.empty()) chunks.push_back(cur);

        // keep only members who share the chunk's main subject
        for (auto& ch : chunks) {
            std::vector<const Student*> p;
            for (int x : ch) p.push_back(&students[x]);
            int cnt = 0; std::string core = coreSubject(p, cnt);
            if (core.empty()) continue;
            std::vector<int> kept;
            for (int x : ch)
                for (auto& s : students[x].subjects)
                    if (toLower(s) == toLower(core)) { kept.push_back(x); break; }
            if (kept.size() >= 2) { std::sort(kept.begin(), kept.end()); groups.push_back(kept); }
        }
    }
    return groups;
}
