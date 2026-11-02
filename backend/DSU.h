// DSU.h - Disjoint Set Union (Union-Find) with path compression and union by size
#pragma once
#include <vector>

class DSU {
public:
    explicit DSU(int n);
    int find(int x);                 // representative of x's set (with path compression)
    bool unite(int a, int b);        // merge two sets; false if already together
    int setSize(int x);              // number of elements in x's set
    int countSets() const { return sets; }
private:
    std::vector<int> parent, size;
    int sets;
};
