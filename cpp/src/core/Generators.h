#ifndef GENERATORS_H
#define GENERATORS_H

#include "Graph.h"
#include <vector>
#include <utility>
#include <optional>

double p_for_avg_degree(int n, double d);

Graph gnp(int n, double p, std::optional<int> seed = std::nullopt);

struct GeometricResult {
    Graph g;
    std::vector<std::pair<double, double>> pts;
};

GeometricResult geometric(int n, double avg_degree = 6.0, std::optional<int> seed = std::nullopt, bool return_coords = false);

#endif // GENERATORS_H
