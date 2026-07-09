#ifndef GRAPH_H
#define GRAPH_H

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>

class Graph {
public:
    Graph();

    void add_vertex(int v, const std::string& label = "");
    void add_edge(int u, int v);
    
    static Graph from_edges(const std::vector<int>& vertices, const std::vector<std::pair<int, int>>& edges, const std::unordered_map<int, std::string>& labels = {});

    const std::vector<int>& neighbors(int v) const;
    int degree(int v) const;
    
    std::vector<int> vertices() const;
    std::vector<std::pair<int, int>> edges() const;
    
    std::string label(int v) const;
    
    int n() const;
    int m() const;
    int max_degree() const;
    int max_id() const;

private:
    std::vector<std::vector<int>> adj;
    std::vector<std::string> labels;
    std::vector<bool> active;
    int num_vertices = 0;
};

#endif // GRAPH_H
