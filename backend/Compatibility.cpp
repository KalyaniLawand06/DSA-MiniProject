#include "Compatibility.h"
#include <algorithm>
#include <map>
#include <set>

static std::vector<std::string> intersect(const std::vector<std::string>& a, const std::vector<std::string>& b) {
    std::vector<std::string> r;
    for (auto& x : a) for (auto& y : b) if (toLower(x) == toLower(y)) { r.push_back(x); break; }
    return r;
}
static bool hasItem(const std::vector<std::string>& v, const std::string& x) {
    for (auto& y : v) if (toLower(y) == toLower(x)) return true;
    return false;
}
static Json strArr(const std::vector<std::string>& v) {
    Json a = Json::array(); for (auto& x : v) a.push(Json(x)); return a;
}
std::string fmtHour(int h) {
    int hh = h % 24, h12 = hh % 12 == 0 ? 12 : hh % 12;
    return std::to_string(h12) + (hh >= 12 && h != 24 ? " PM" : " AM");
}

// ---------------------------------------------------------------
// Weighted formula: subjects 40 + timings 30 + goals 20 + skills 10
// ---------------------------------------------------------------
Compat computeCompat(const Student& a, const Student& b) {
    Compat c;
    // 1. common subjects (40)
    c.commonSubjects = intersect(a.subjects, b.subjects);
    size_t minSub = std::min(a.subjects.size(), b.subjects.size());
    if (minSub > 0) c.subjectScore = 40.0 * c.commonSubjects.size() / minSub;

    // 2. common timings (30): shared days x overlapping hours
    c.commonDays = intersect(a.days, b.days);
    int ov = std::min(a.endHour, b.endHour) - std::max(a.startHour, b.startHour);
    if (!c.commonDays.empty() && ov > 0) {
        c.overlapStart = std::max(a.startHour, b.startHour);
        c.overlapEnd = std::min(a.endHour, b.endHour);
        double dayRatio = (double)c.commonDays.size() / std::min(a.days.size(), b.days.size());
        int minDur = std::min(a.endHour - a.startHour, b.endHour - b.startHour);
        double hourRatio = (double)ov / minDur;
        c.timingScore = 30.0 * dayRatio * hourRatio;
    }

    // 3. shared learning goals (20)
    c.commonGoals = intersect(a.goals, b.goals);
    size_t minGoal = std::min(a.goals.size(), b.goals.size());
    if (minGoal > 0) c.goalScore = 20.0 * c.commonGoals.size() / minGoal;

    // 4. complementary skills (10): 7 for skills only one of them has, 3 for different levels
    std::set<std::string> uni;
    for (auto& s : a.skills) uni.insert(toLower(s));
    for (auto& s : b.skills) uni.insert(toLower(s));
    for (auto& s : a.skills) if (!hasItem(b.skills, s)) c.complementSkills.push_back(s);
    for (auto& s : b.skills) if (!hasItem(a.skills, s)) c.complementSkills.push_back(s);
    if (!uni.empty()) c.skillScore += 7.0 * c.complementSkills.size() / uni.size();
    if (a.level != b.level) c.skillScore += 3.0;

    c.total = c.subjectScore + c.timingScore + c.goalScore + c.skillScore;
    c.total = (int)(c.total * 10 + 0.5) / 10.0;
    c.meaningful = !c.commonSubjects.empty() && c.timingScore > 0;
    return c;
}

Json Compat::toJson() const {
    Json j = Json::object();
    j["total"] = total; j["subjectScore"] = subjectScore; j["timingScore"] = timingScore;
    j["goalScore"] = goalScore; j["skillScore"] = skillScore;
    j["commonSubjects"] = strArr(commonSubjects); j["commonDays"] = strArr(commonDays);
    j["commonGoals"] = strArr(commonGoals); j["complementSkills"] = strArr(complementSkills);
    j["overlap"] = timingScore > 0 ? fmtHour(overlapStart) + " - " + fmtHour(overlapEnd) : std::string("none");
    j["meaningful"] = meaningful;
    return j;
}

// ---------------- helpers used by both grouping algorithms ----------------
std::string coreSubject(const std::vector<const Student*>& g, int& count) {
    std::map<std::string, int> cnt; std::map<std::string, std::string> shown;
    for (auto* s : g) for (auto& sub : s->subjects) { cnt[toLower(sub)]++; shown[toLower(sub)] = sub; }
    std::string best; count = 0;
    for (auto& kv : cnt) if (kv.second > count) { count = kv.second; best = shown[kv.first]; }
    return count >= 2 ? best : "";
}
std::vector<std::string> subjectsCommonToAll(const std::vector<const Student*>& g) {
    std::vector<std::string> r;
    if (g.empty()) return r;
    for (auto& sub : g[0]->subjects) {
        bool all = true;
        for (auto* s : g) if (!hasItem(s->subjects, sub)) all = false;
        if (all) r.push_back(sub);
    }
    return r;
}
int minPreferredSize(const std::vector<const Student*>& g) {
    int m = 99; for (auto* s : g) m = std::min(m, s->groupSize); return m;
}

// ---------------- group explanation (all values computed, nothing hardcoded) ----------------
Json describeGroup(const std::vector<const Student*>& g) {
    Json j = Json::object();
    int n = (int)g.size();
    // members
    Json mem = Json::array();
    for (auto* s : g) mem.push(s->toJson());
    j["members"] = mem; j["size"] = n;

    // subjects
    std::vector<std::string> common = subjectsCommonToAll(g);
    int cc = 0; std::string core = coreSubject(g, cc);
    std::map<std::string, int> cnt; std::map<std::string, std::string> shown;
    for (auto* s : g) for (auto& sub : s->subjects) { cnt[toLower(sub)]++; shown[toLower(sub)] = sub; }
    Json shared = Json::array(); std::vector<std::string> sharedNames;
    for (auto& kv : cnt) if (kv.second >= 2) { Json x = Json::object(); x["subject"] = shown[kv.first]; x["count"] = kv.second; shared.push(x); sharedNames.push_back(shown[kv.first]); }
    j["commonSubjects"] = strArr(common); j["sharedSubjects"] = shared; j["coreSubject"] = core;
    { std::string up = core; for (auto& ch : up) ch = (char)toupper((unsigned char)ch);
      j["name"] = up.empty() ? std::string("STUDY PARTNERS") : up + " STUDY PARTNERS"; }

    // availability: members per day, best overlapping hour window
    static const char* DAYS[] = {"Mon","Tue","Wed","Thu","Fri","Sat","Sun"};
    Json dayCounts = Json::array(); int bestDayCount = 0; std::vector<std::string> bestDays;
    for (auto d : DAYS) {
        int c = 0; for (auto* s : g) if (hasItem(s->days, d)) c++;
        Json x = Json::object(); x["day"] = d; x["count"] = c; dayCounts.push(x);
        if (c > bestDayCount) { bestDayCount = c; bestDays.clear(); }
        if (c == bestDayCount && c > 0) bestDays.push_back(d);
    }
    j["dayCounts"] = dayCounts;
    int bestHourCount = 0, bs = 0, be = 0, runStart = -1;
    int cntH[24] = {0};
    for (auto* s : g) for (int h = s->startHour; h < s->endHour && h < 24; h++) cntH[h]++;
    for (int h = 0; h < 24; h++) bestHourCount = std::max(bestHourCount, cntH[h]);
    int bestLen = 0;
    for (int h = 0; h <= 24; h++) {
        bool on = h < 24 && cntH[h] == bestHourCount && bestHourCount > 0;
        if (on && runStart < 0) runStart = h;
        if (!on && runStart >= 0) { if (h - runStart > bestLen) { bestLen = h - runStart; bs = runStart; be = h; } runStart = -1; }
    }
    Json av = Json::object();
    av["bestDays"] = strArr(bestDays); av["bestDayCount"] = bestDayCount;
    av["window"] = bestLen > 0 ? fmtHour(bs) + " - " + fmtHour(be) : std::string("none");
    av["windowCount"] = bestHourCount;
    j["availability"] = av;

    // levels
    int lb = 0, li = 0, la = 0;
    for (auto* s : g) { if (s->level == "Beginner") lb++; else if (s->level == "Intermediate") li++; else la++; }
    Json lv = Json::object(); lv["Beginner"] = lb; lv["Intermediate"] = li; lv["Advanced"] = la;
    j["levels"] = lv;

    // skills held by the members (each with how many members offer it)
    std::map<std::string, int> sk; std::map<std::string, std::string> skShown;
    for (auto* s : g) for (auto& x : s->skills) { sk[toLower(x)]++; skShown[toLower(x)] = x; }
    std::vector<std::string> skills; for (auto& kv : sk) skills.push_back(skShown[kv.first]);
    j["skills"] = strArr(skills);

    // shared goals
    std::map<std::string, int> gc; std::map<std::string, std::string> gShown;
    for (auto* s : g) for (auto& x : s->goals) { gc[toLower(x)]++; gShown[toLower(x)] = x; }
    std::vector<std::string> goals;
    for (auto& kv : gc) if (kv.second >= 2) goals.push_back(gShown[kv.first]);
    j["sharedGoals"] = strArr(goals);

    // average pairwise compatibility + best pair
    double sum = 0; int pairs = 0, bi = 0, bj = 0; double bestPair = -1;
    for (int x = 0; x < n; x++) for (int y = x + 1; y < n; y++) {
        double t = computeCompat(*g[x], *g[y]).total; sum += t; pairs++;
        if (t > bestPair) { bestPair = t; bi = x; bj = y; }
    }
    double avg = pairs ? sum / pairs : 0;
    avg = (int)(avg * 10 + 0.5) / 10.0;
    j["compatibility"] = avg;

    // ---- dynamic reasons ----
    Json reasons = Json::array();
    if (!common.empty()) {
        std::string l; for (size_t i = 0; i < common.size(); i++) l += (i ? ", " : "") + common[i];
        reasons.push(Json("All " + std::to_string(n) + " members share " + l + "."));
    }
    if (!core.empty() && (common.empty() || cc < n))
        reasons.push(Json(std::to_string(cc) + " of " + std::to_string(n) + " members study " + core + "."));
    if (bestDayCount > 0) {
        std::string l; for (size_t i = 0; i < bestDays.size(); i++) l += (i ? ", " : "") + bestDays[i];
        reasons.push(Json(std::to_string(bestDayCount) + " of " + std::to_string(n) + " members are available on " + l + "."));
    }
    if (bestLen > 0)
        reasons.push(Json(std::to_string(bestHourCount) + " of " + std::to_string(n) + " members overlap between " + fmtHour(bs) + " and " + fmtHour(be) + "."));
    if (lb > 0 && (li + la) > 0)
        reasons.push(Json(std::to_string(li + la) + " Intermediate/Advanced member(s) can help " + std::to_string(lb) + " Beginner(s)."));
    else if (lb + li + la == n)
        reasons.push(Json("Members are at a similar experience level, so they can learn at the same pace."));
    if (skills.size() >= 2) {
        std::string l; for (size_t i = 0; i < skills.size() && i < 6; i++) l += (i ? ", " : "") + skills[i];
        reasons.push(Json("Members offer complementary skills: " + l + "."));
    }
    if (!goals.empty()) {
        std::string l; for (size_t i = 0; i < goals.size(); i++) l += (i ? ", " : "") + goals[i];
        reasons.push(Json("Shared learning goals: " + l + "."));
    }
    if (pairs > 0)
        reasons.push(Json("Strongest pair: " + g[bi]->name + " & " + g[bj]->name + " (" + std::to_string((int)(bestPair + 0.5)) + "%)."));
    j["reasons"] = reasons;
    return j;
}
