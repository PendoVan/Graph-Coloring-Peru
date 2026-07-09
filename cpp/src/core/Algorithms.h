#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Graph.h"
#include <vector>
#include <unordered_map>
#include <chrono>

struct ExactResult {
    int k;
    std::vector<int> coloring;
    bool exact;
    int nodes;
    double elapsed;
    int lower;
    int upper;
};

// Ordering algorithms
std::vector<int> order_natural(const Graph& g);
std::vector<int> order_largest_first(const Graph& g);
std::vector<int> order_smallest_last(const Graph& g);
std::vector<int> order_distance(const Graph& g, int source);
std::vector<int> bfs_levels(const Graph& g, int source);

// Coloring algorithms
std::vector<int> greedy(const Graph& g, const std::vector<int>& order);
std::vector<int> dsatur(const Graph& g);
std::vector<int> greedy_clique(const Graph& g, int max_seeds = 100);

// Backtracking
ExactResult chromatic_number(const Graph& g, int max_n = 200, double timeout_s = 30.0);

#endif // ALGORITHMS_H
