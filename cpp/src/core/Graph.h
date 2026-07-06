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

    const std::unordered_set<int>& neighbors(int v) const;
    int degree(int v) const;
    
    std::vector<int> vertices() const;
    std::vector<std::pair<int, int>> edges() const;
    
    std::string label(int v) const;
    
    int n() const;
    int m() const;
    int max_degree() const;

private:
    std::unordered_map<int, std::unordered_set<int>> adj;
    std::unordered_map<int, std::string> labels;
};

#endif // GRAPH_H
