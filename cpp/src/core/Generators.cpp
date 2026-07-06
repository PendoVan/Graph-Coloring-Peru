#define _USE_MATH_DEFINES
#include "Generators.h"
#include <random>
#include <cmath>
#include <map>
#include <unordered_map>

using namespace std;

// Need a hash for pair<int, int> to use unordered_map with pair keys
struct pair_hash {
    template <class T1, class T2>
    size_t operator()(const pair<T1, T2>& p) const {
        auto h1 = hash<T1>{}(p.first);
        auto h2 = hash<T2>{}(p.second);
        return h1 ^ (h2 << 1);
    }
};

double p_for_avg_degree(int n, double d) {
    return min(1.0, d / max(n - 1, 1));
}

Graph gnp(int n, double p, optional<int> seed) {
    Graph g;
    for (int v = 0; v < n; ++v) {
        g.add_vertex(v);
    }
    
    if (n < 2 || p <= 0.0) return g;
    if (p >= 1.0) {
        for (int v = 0; v < n; ++v) {
            for (int w = 0; w < v; ++w) {
                g.add_edge(v, w);
            }
        }
        return g;
    }
    
    mt19937 rng(seed.value_or(random_device{}()));
    uniform_real_distribution<double> dist(0.0, 1.0);
    
    double log_q = log1p(-p);
    int v = 1;
    int w = -1;
    
    while (v < n) {
        double r = dist(rng);
        w += 1 + static_cast<int>(log1p(-r) / log_q);
        while (w >= v && v < n) {
            w -= v;
            v += 1;
        }
        if (v < n) {
            g.add_edge(v, w);
        }
    }
    
    return g;
}

GeometricResult geometric(int n, double avg_degree, optional<int> seed, bool return_coords) {
    double radius = sqrt(avg_degree / (M_PI * n));
    mt19937 rng(seed.value_or(random_device{}()));
    uniform_real_distribution<double> dist(0.0, 1.0);
    
    vector<pair<double, double>> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
        pts.push_back({dist(rng), dist(rng)});
    }
    
    Graph g;
    for (int v = 0; v < n; ++v) {
        g.add_vertex(v);
    }
    
    unordered_map<pair<int, int>, vector<int>, pair_hash> grid;
    for (int i = 0; i < n; ++i) {
        int cx = static_cast<int>(pts[i].first / radius);
        int cy = static_cast<int>(pts[i].second / radius);
        grid[{cx, cy}].push_back(i);
    }
    
    double r2 = radius * radius;
    vector<pair<int, int>> stencil = {{0, 0}, {1, 0}, {0, 1}, {1, 1}, {1, -1}};
    
    for (const auto& cell : grid) {
        int cx = cell.first.first;
        int cy = cell.first.second;
        const auto& here = cell.second;
        
        for (const auto& delta : stencil) {
            int dx = delta.first;
            int dy = delta.second;
            
            vector<int> const* there = nullptr;
            if (dx == 0 && dy == 0) {
                there = &here;
            } else {
                auto it = grid.find({cx + dx, cy + dy});
                if (it != grid.end()) {
                    there = &(it->second);
                }
            }
            
            if (!there) continue;
            
            for (size_t a = 0; a < here.size(); ++a) {
                int i = here[a];
                double xi = pts[i].first;
                double yi = pts[i].second;
                
                size_t start = (dx == 0 && dy == 0) ? a + 1 : 0;
                for (size_t b = start; b < there->size(); ++b) {
                    int j = (*there)[b];
                    double xj = pts[j].first;
                    double yj = pts[j].second;
                    
                    if ((xi - xj) * (xi - xj) + (yi - yj) * (yi - yj) <= r2) {
                        g.add_edge(i, j);
                    }
                }
            }
        }
    }
    
    return {g, return_coords ? pts : vector<pair<double, double>>()};
}
