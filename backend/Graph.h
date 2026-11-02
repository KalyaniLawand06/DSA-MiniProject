// Graph.h - undirected weighted graph stored as an ADJACENCY LIST
#pragma once
#include <vector>
#include "Student.h"

struct Edge { int to; double weight; };          // one entry of an adjacency list
struct WEdge { int u, v; double weight; };       // a full edge (used for sorting)

class Graph {
public:
    // vertices = students; an edge joins two students whose compatibility >= threshold
    Graph(const std::vector<Student>& students, double threshold);
    int vertices() const { return n; }
    const std::vector<Edge>& neighbors(int u) const { return adj[u]; }
    std::vector<WEdge> edges() const;            // each undirected edge once (u < v)
    int edgeCount() const;
private:
    int n;
    std::vector<std::vector<Edge>> adj;          // adj[u] = list of (neighbor, weight)
    void addEdge(int u, int v, double w);
};
