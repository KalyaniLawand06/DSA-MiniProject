#include "Student.h"
#include <fstream>
#include <sstream>
#include <algorithm>

std::string toLower(std::string s) {
    for (auto& c : s) c = (char)tolower((unsigned char)c);
    return s;
}
static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
    return a == std::string::npos ? "" : s.substr(a, b - a + 1);
}
// trim each entry, drop empties and duplicates (case-insensitive)
static std::vector<std::string> cleanList(const std::vector<std::string>& in) {
    std::vector<std::string> out;
    for (auto& x : in) {
        std::string t = trim(x);
        if (t.empty()) continue;
        bool dup = false;
        for (auto& y : out) if (toLower(y) == toLower(t)) dup = true;
        if (!dup) out.push_back(t);
    }
    return out;
}
static Json listJson(const std::vector<std::string>& v) {
    Json a = Json::array();
    for (auto& x : v) a.push(Json(x));
    return a;
}

Json Student::toJson() const {
    Json j = Json::object();
    j["id"] = id; j["studentId"] = studentId; j["name"] = name; j["department"] = department;
    j["semester"] = semester; j["subjects"] = listJson(subjects); j["days"] = listJson(days);
    j["startHour"] = startHour; j["endHour"] = endHour; j["goals"] = listJson(goals);
    j["level"] = level; j["skills"] = listJson(skills); j["groupSize"] = groupSize;
    return j;
}

Student Student::fromJson(const Json& j) {
    Student s;
    s.id = j.getInt("id", 0);
    s.studentId = trim(j.getStr("studentId"));
    s.name = trim(j.getStr("name"));
    s.department = trim(j.getStr("department"));
    s.semester = j.getInt("semester", 1);
    s.subjects = cleanList(j.getStrList("subjects"));
    s.days = cleanList(j.getStrList("days"));
    s.goals = cleanList(j.getStrList("goals"));
    s.skills = cleanList(j.getStrList("skills"));
    s.startHour = j.getInt("startHour", 16);
    s.endHour = j.getInt("endHour", 18);
    s.level = trim(j.getStr("level", "Beginner"));
    s.groupSize = j.getInt("groupSize", 4);
    return s;
}

void StudentStore::load() {
    std::ifstream f(path);
    students.clear(); nextId = 1;
    if (!f) { save(); return; }
    std::stringstream ss; ss << f.rdbuf();
    bool ok = false;
    Json j = Json::parse(ss.str(), &ok);
    if (!ok) { fprintf(stderr, "Warning: %s is not valid JSON, starting empty\n", path.c_str()); return; }
    nextId = j.getInt("nextId", 1);
    const Json* arr = j.find("students");
    if (arr && arr->type == Json::Arr)
        for (auto& x : arr->a) { Student s = Student::fromJson(x); students.push_back(s); nextId = std::max(nextId, s.id + 1); }
}

void StudentStore::save() const {
    Json j = Json::object();
    j["nextId"] = nextId;
    Json arr = Json::array();
    for (auto& s : students) arr.push(s.toJson());
    j["students"] = arr;
    std::ofstream f(path, std::ios::trunc);
    f << j.dump();
}

std::string StudentStore::validate(const Student& s, int ignoreId) const {
    if (s.name.empty()) return "Full name is required.";
    if (s.studentId.empty()) return "Student ID is required.";
    if (s.department.empty()) return "Department is required.";
    if (s.semester < 1 || s.semester > 8) return "Semester must be between 1 and 8.";
    if (s.subjects.empty()) return "Select at least one subject.";
    if (s.days.empty()) return "Select at least one study day.";
    if (s.startHour < 0 || s.endHour > 24 || s.startHour >= s.endHour) return "Study timing is invalid (end must be after start).";
    if (s.level != "Beginner" && s.level != "Intermediate" && s.level != "Advanced") return "Experience level is invalid.";
    if (s.groupSize < 2 || s.groupSize > 8) return "Preferred group size must be between 2 and 8.";
    for (auto& o : students)
        if (o.id != ignoreId && toLower(o.studentId) == toLower(s.studentId)) return "Student ID '" + s.studentId + "' is already registered.";
    return "";
}

Student& StudentStore::add(Student s) {
    s.id = nextId++;
    students.push_back(s);
    save();
    return students.back();
}
bool StudentStore::update(int id, Student s) {
    Student* old = find(id);
    if (!old) return false;
    s.id = id; *old = s; save();
    return true;
}
bool StudentStore::remove(int id) {
    for (size_t i = 0; i < students.size(); i++)
        if (students[i].id == id) { students.erase(students.begin() + i); save(); return true; }
    return false;
}
Student* StudentStore::find(int id) {
    for (auto& s : students) if (s.id == id) return &s;
    return nullptr;
}
