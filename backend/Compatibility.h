// Compatibility.h - pairwise compatibility score + group explanation
#pragma once
#include <vector>
#include <string>
#include "Student.h"

struct Compat {
    double subjectScore = 0, timingScore = 0, goalScore = 0, skillScore = 0, total = 0; // out of 40/30/20/10/100
    std::vector<std::string> commonSubjects, commonDays, commonGoals, complementSkills;
    int overlapStart = 0, overlapEnd = 0;   // overlapping hours on common days
    bool meaningful = false;                // has a common subject AND overlapping time
    Json toJson() const;
};

Compat computeCompat(const Student& a, const Student& b);

// subject held by the most members ("" if nobody shares one); count returned in 'count'
std::string coreSubject(const std::vector<const Student*>& g, int& count);
// subjects that EVERY member has
std::vector<std::string> subjectsCommonToAll(const std::vector<const Student*>& g);
int minPreferredSize(const std::vector<const Student*>& g);

// full explanation of a group (members, reasons, availability, levels, score...)
Json describeGroup(const std::vector<const Student*>& g);
std::string fmtHour(int h);
