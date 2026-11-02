// Student.h - student record + persistent JSON store
#pragma once
#include <string>
#include <vector>
#include "Json.h"

struct Student {
    int id = 0;                       // internal database id
    std::string studentId;            // college id (e.g. 2024CE001)
    std::string name, department;
    int semester = 1;
    std::vector<std::string> subjects, days, goals, skills;
    int startHour = 16, endHour = 18; // preferred study timing (24h clock)
    std::string level = "Beginner";   // Beginner / Intermediate / Advanced
    int groupSize = 4;                // preferred group size

    Json toJson() const;
    static Student fromJson(const Json& j);
};

class StudentStore {
public:
    explicit StudentStore(const std::string& file) : path(file) {}
    void load();
    void save() const;
    std::string validate(const Student& s, int ignoreId) const; // "" when valid
    Student& add(Student s);
    bool update(int id, Student s);
    bool remove(int id);
    Student* find(int id);
    std::vector<Student>& all() { return students; }
private:
    std::string path;
    std::vector<Student> students;
    int nextId = 1;
};

std::string toLower(std::string s);
