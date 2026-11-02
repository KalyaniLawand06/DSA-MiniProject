#include "Graph.h"
#include "Compatibility.h"

Graph::Graph(const std::vector<Student>& students, double threshold) {
    n = (int)students.size();
    adj.assign(n, {});
    for (int i = 0; i < n; i++)
        for (int j = i + 1; j < n; j++) {
            Compat c = computeCompat(students[i], students[j]);
            // only "meaningful" pairs (common subject + overlapping time) above the threshold become edges
            if (c.meaningful && c.total >= threshold) addEdge(i, j, c.total);
        }
}

void Graph::addEdge(int u, int v, double w) {
    adj[u].push_back({v, w});                    // undirected: store in both lists
    adj[v].push_back({u, w});
}

std::vector<WEdge> Graph::edges() const {
    std::vector<WEdge> r;
    for (int u = 0; u < n; u++)
        for (auto& e : adj[u]) if (u < e.to) r.push_back({u, e.to, e.weight});
    return r;
}
int Graph::edgeCount() const { return (int)edges().size(); }
