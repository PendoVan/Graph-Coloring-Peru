#include "Graph.h"

Graph::Graph() {}

void Graph::add_vertex(int v, const std::string& label) {
    if (v >= adj.size()) {
        adj.resize(v + 1);
        labels.resize(v + 1);
        active.resize(v + 1, false);
    }
    if (!active[v]) {
        active[v] = true;
        num_vertices++;
    }
    if (!label.empty()) {
        labels[v] = label;
    }
}

void Graph::add_edge(int u, int v) {
    add_vertex(u);
    add_vertex(v);
    if (u != v) {
        // avoid duplicates if multigraph is not intended, but usually it's fine for simple graphs 
        // to just check if it already exists or rely on generator
        if (std::find(adj[u].begin(), adj[u].end(), v) == adj[u].end()) {
            adj[u].push_back(v);
            adj[v].push_back(u);
        }
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

const std::vector<int>& Graph::neighbors(int v) const {
    static const std::vector<int> empty_set;
    if (v < adj.size() && active[v]) {
        return adj[v];
    }
    return empty_set;
}

int Graph::degree(int v) const {
    if (v < adj.size() && active[v]) {
        return adj[v].size();
    }
    return 0;
}

std::vector<int> Graph::vertices() const {
    std::vector<int> verts;
    verts.reserve(num_vertices);
    for (int i = 0; i < active.size(); ++i) {
        if (active[i]) verts.push_back(i);
    }
    return verts;
}

std::vector<std::pair<int, int>> Graph::edges() const {
    std::vector<std::pair<int, int>> edge_list;
    for (int u = 0; u < adj.size(); ++u) {
        if (active[u]) {
            for (int v : adj[u]) {
                if (u < v) {
                    edge_list.push_back({u, v});
                }
            }
        }
    }
    return edge_list;
}

std::string Graph::label(int v) const {
    if (v < labels.size() && !labels[v].empty()) {
        return labels[v];
    }
    return std::to_string(v);
}

int Graph::n() const {
    return num_vertices;
}

int Graph::m() const {
    int total_edges = 0;
    for (int i = 0; i < adj.size(); ++i) {
        if (active[i]) {
            total_edges += adj[i].size();
        }
    }
    return total_edges / 2;
}

int Graph::max_degree() const {
    int max_deg = 0;
    for (int i = 0; i < adj.size(); ++i) {
        if (active[i] && adj[i].size() > max_deg) {
            max_deg = adj[i].size();
        }
    }
    return max_deg;
}

int Graph::max_id() const {
    int max_i = -1;
    for (int i = adj.size() - 1; i >= 0; --i) {
        if (active[i]) { return i; }
    }
    return max_i;
}
