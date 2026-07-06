#include "Graph.h"

Graph::Graph() {}

void Graph::add_vertex(int v, const std::string& label) {
    if (adj.find(v) == adj.end()) {
        adj[v] = std::unordered_set<int>();
    }
    if (!label.empty()) {
        labels[v] = label;
    }
}

void Graph::add_edge(int u, int v) {
    add_vertex(u);
    add_vertex(v);
    if (u != v) {
        adj[u].insert(v);
        adj[v].insert(u);
    }
}

Graph Graph::from_edges(const std::vector<int>& vertices, const std::vector<std::pair<int, int>>& edges, const std::unordered_map<int, std::string>& labels) {
    Graph g;
    for (int v : vertices) {
        std::string lbl = "";
        if (labels.find(v) != labels.end()) {
            lbl = labels.at(v);
        }
        g.add_vertex(v, lbl);
    }
    for (const auto& edge : edges) {
        g.add_edge(edge.first, edge.second);
    }
    return g;
}

const std::unordered_set<int>& Graph::neighbors(int v) const {
    static const std::unordered_set<int> empty_set;
    auto it = adj.find(v);
    if (it != adj.end()) {
        return it->second;
    }
    return empty_set;
}

int Graph::degree(int v) const {
    auto it = adj.find(v);
    if (it != adj.end()) {
        return it->second.size();
    }
    return 0;
}

std::vector<int> Graph::vertices() const {
    std::vector<int> verts;
    verts.reserve(adj.size());
    for (const auto& pair : adj) {
        verts.push_back(pair.first);
    }
    return verts;
}

std::vector<std::pair<int, int>> Graph::edges() const {
    std::vector<std::pair<int, int>> edge_list;
    for (const auto& u_pair : adj) {
        int u = u_pair.first;
        for (int v : u_pair.second) {
            if (u < v) {
                edge_list.push_back({u, v});
            }
        }
    }
    return edge_list;
}

std::string Graph::label(int v) const {
    auto it = labels.find(v);
    if (it != labels.end()) {
        return it->second;
    }
    return std::to_string(v);
}

int Graph::n() const {
    return adj.size();
}

int Graph::m() const {
    int total_edges = 0;
    for (const auto& pair : adj) {
        total_edges += pair.second.size();
    }
    return total_edges / 2;
}

int Graph::max_degree() const {
    int max_deg = 0;
    for (const auto& pair : adj) {
        if (pair.second.size() > max_deg) {
            max_deg = pair.second.size();
        }
    }
    return max_deg;
}
