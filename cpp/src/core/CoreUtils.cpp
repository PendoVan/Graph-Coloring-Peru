#include "CoreUtils.h"
#include <unordered_set>

ProperResult is_proper(const Graph& graph, const std::vector<int>& color) {
    for (const auto& edge : graph.edges()) {
        int u = edge.first;
        int v = edge.second;
        
        if (u >= color.size() || color[u] == 0 ||
            v >= color.size() || color[v] == 0 ||
            color[u] == color[v]) {
            return {false, {u, v}};
        }
    }
    return {true, {-1, -1}};
}

int num_colors(const std::vector<int>& color) {
    int max_c = 0;
    for (int c : color) {
        if (c > max_c) max_c = c;
    }
    return max_c;
}
