#include "GreedyGrouping.h"
#include "DSU.h"
#include "Compatibility.h"
#include <algorithm>

// Step 1: take every edge (compatible pair) of the graph
// Step 2: sort edges by compatibility, highest first
// Step 3: walk through the edges; merge the two groups (DSU union) when
//         - they are different groups, and
//         - the merged group is not larger than the preferred group size, and
//         - ALL members of the merged group share at least one subject.
// Each student can only be in one DSU set, so nobody is placed in two groups.
std::vector<std::vector<int>> greedyGroup(const std::vector<Student>& students, const Graph& g, int maxSize) {
    int n = (int)students.size();
    std::vector<WEdge> edges = g.edges();
    std::sort(edges.begin(), edges.end(), [](const WEdge& a, const WEdge& b) {
        if (a.weight != b.weight) return a.weight > b.weight;     // descending
        if (a.u != b.u) return a.u < b.u;
        return a.v < b.v;
    });

    DSU dsu(n);
    std::vector<std::vector<int>> members(n);                      // members[root] = students of that set
    for (int i = 0; i < n; i++) members[i] = {i};

    for (auto& e : edges) {
        int ru = dsu.find(e.u), rv = dsu.find(e.v);
        if (ru == rv) continue;                                    // already in the same group

        std::vector<int> merged = members[ru];
        merged.insert(merged.end(), members[rv].begin(), members[rv].end());

        std::vector<const Student*> ptrs;
        for (int m : merged) ptrs.push_back(&students[m]);

        int limit = std::min(maxSize, minPreferredSize(ptrs));     // respect preferred group sizes
        if ((int)merged.size() > limit) continue;
        if (subjectsCommonToAll(ptrs).empty()) continue;           // must share a subject

        dsu.unite(ru, rv);
        int root = dsu.find(ru);
        members[root] = merged;
        if (root != ru) members[ru].clear();
        if (root != rv) members[rv].clear();
    }

    std::vector<std::vector<int>> groups;
    for (int i = 0; i < n; i++)
        if (dsu.find(i) == i && members[i].size() >= 2) {
            std::sort(members[i].begin(), members[i].end());
            groups.push_back(members[i]);
        }
    return groups;
}
