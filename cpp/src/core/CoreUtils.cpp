#include "CoreUtils.h"
#include <unordered_set>

ProperResult is_proper(const Graph& graph, const std::unordered_map<int, int>& color) {
    for (const auto& edge : graph.edges()) {
        int u = edge.first;
        int v = edge.second;
        
        auto it_u = color.find(u);
        auto it_v = color.find(v);
        
        if (it_u == color.end() || it_u->second == 0 ||
            it_v == color.end() || it_v->second == 0 ||
            it_u->second == it_v->second) {
            return {false, {u, v}};
        }
    }
    return {true, {-1, -1}};
}

int num_colors(const std::unordered_map<int, int>& color) {
    std::unordered_set<int> unique_colors;
    for (const auto& pair : color) {
        if (pair.second != 0) {
            unique_colors.insert(pair.second);
        }
    }
    return unique_colors.size();
}
