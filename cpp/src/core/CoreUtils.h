#ifndef COREUTILS_H
#define COREUTILS_H

#include "Graph.h"
#include <unordered_map>
#include <utility>

struct ProperResult {
    bool valid;
    std::pair<int, int> edge;
};

ProperResult is_proper(const Graph& graph, const std::unordered_map<int, int>& color);
int num_colors(const std::unordered_map<int, int>& color);

#endif // COREUTILS_H
