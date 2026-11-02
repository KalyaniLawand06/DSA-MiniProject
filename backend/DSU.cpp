#include "DSU.h"

DSU::DSU(int n) : parent(n), size(n, 1), sets(n) {
    for (int i = 0; i < n; i++) parent[i] = i;   // every element starts alone
}

int DSU::find(int x) {
    if (parent[x] != x)
        parent[x] = find(parent[x]);            // path compression
    return parent[x];
}

bool DSU::unite(int a, int b) {
    int ra = find(a), rb = find(b);
    if (ra == rb) return false;                 // already in the same group
    if (size[ra] < size[rb]) { int t = ra; ra = rb; rb = t; }
    parent[rb] = ra;                            // attach smaller tree under larger
    size[ra] += size[rb];
    sets--;
    return true;
}

int DSU::setSize(int x) { return size[find(x)]; }
