// GreedyGrouping.h - greedy grouping: strongest pairs first, merged with DSU
#pragma once
#include <vector>
#include "Graph.h"
// returns groups as lists of student indices (positions in the students vector)
std::vector<std::vector<int>> greedyGroup(const std::vector<Student>& students, const Graph& g, int maxSize);
