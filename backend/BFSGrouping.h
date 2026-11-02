// BFSGrouping.h - connected-component grouping using BFS
#pragma once
#include <vector>
#include "Graph.h"
std::vector<std::vector<int>> bfsGroup(const std::vector<Student>& students, const Graph& g, int maxSize);
// number of connected components (isolated students included), counted with BFS
int bfsComponentCount(const Graph& g);
