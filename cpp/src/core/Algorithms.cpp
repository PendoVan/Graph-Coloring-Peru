#include "Algorithms.h"
#include <queue>
#include <set>
#include <algorithm>
#include <stdexcept>
#include <iostream>

using namespace std;

vector<int> order_natural(const Graph& g) {
    return g.vertices();
}

vector<int> order_largest_first(const Graph& g) {
    int max_deg = g.max_degree();
    vector<vector<int>> buckets(max_deg + 1);
    for (int v : g.vertices()) {
        buckets[g.degree(v)].push_back(v);
    }
    
    vector<int> seq;
    seq.reserve(g.n());
    for (int d = max_deg; d >= 0; --d) {
        for (int v : buckets[d]) {
            seq.push_back(v);
        }
    }
    return seq;
}

vector<int> order_smallest_last(const Graph& g) {
    unordered_map<int, int> deg;
    int max_deg = g.max_degree();
    vector<unordered_set<int>> buckets(max_deg + 1);
    
    for (int v : g.vertices()) {
        int d = g.degree(v);
        deg[v] = d;
        buckets[d].insert(v);
    }
    
    unordered_set<int> removed;
    vector<int> seq;
    seq.reserve(g.n());
    
    int i = 0;
    for (int count = 0; count < g.n(); ++count) {
        while (i <= max_deg && buckets[i].empty()) {
            ++i;
        }
        if (i > max_deg) break;
        
        int v = *buckets[i].begin();
        buckets[i].erase(v);
        removed.insert(v);
        seq.push_back(v);
        
        for (int u : g.neighbors(v)) {
            if (removed.find(u) == removed.end()) {
                int d = deg[u];
                buckets[d].erase(u);
                deg[u] = d - 1;
                buckets[d - 1].insert(u);
            }
        }
        i = max(0, i - 1);
    }
    
    reverse(seq.begin(), seq.end());
    return seq;
}

vector<int> order_distance(const Graph& g, int source) {
    unordered_set<int> seen;
    seen.insert(source);
    queue<int> q;
    q.push(source);
    
    vector<int> seq;
    seq.reserve(g.n());
    
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        seq.push_back(v);
        
        vector<int> neighbors(g.neighbors(v).begin(), g.neighbors(v).end());
        sort(neighbors.begin(), neighbors.end(), [&g](int a, int b) {
            return g.degree(a) > g.degree(b);
        });
        
        for (int u : neighbors) {
            if (seen.find(u) == seen.end()) {
                seen.insert(u);
                q.push(u);
            }
        }
    }
    
    for (int v : g.vertices()) {
        if (seen.find(v) == seen.end()) {
            seq.push_back(v);
        }
    }
    return seq;
}

unordered_map<int, int> bfs_levels(const Graph& g, int source) {
    unordered_map<int, int> dist;
    for (int v : g.vertices()) {
        dist[v] = -1;
    }
    dist[source] = 0;
    queue<int> q;
    q.push(source);
    
    while (!q.empty()) {
        int v = q.front();
        q.pop();
        
        for (int u : g.neighbors(v)) {
            if (dist[u] == -1) {
                dist[u] = dist[v] + 1;
                q.push(u);
            }
        }
    }
    return dist;
}

unordered_map<int, int> greedy(const Graph& g, const vector<int>& order) {
    unordered_map<int, int> color;
    for (int v : g.vertices()) color[v] = 0;
    
    for (int v : order) {
        unordered_set<int> used;
        for (int u : g.neighbors(v)) {
            if (color[u] > 0) used.insert(color[u]);
        }
        int c = 1;
        while (used.find(c) != used.end()) {
            ++c;
        }
        color[v] = c;
    }
    return color;
}

struct DsaturNode {
    int sat;
    int deg;
    int v;
    bool operator<(const DsaturNode& other) const {
        if (sat != other.sat) return sat < other.sat;
        if (deg != other.deg) return deg < other.deg;
        return v > other.v; // tie-breaker to ensure consistent ordering
    }
};

unordered_map<int, int> dsatur(const Graph& g) {
    unordered_map<int, int> color;
    unordered_map<int, unordered_set<int>> sat;
    priority_queue<DsaturNode> pq;
    
    for (int v : g.vertices()) {
        color[v] = 0;
        sat[v] = unordered_set<int>();
        pq.push({0, g.degree(v), v});
    }
    
    int pending = g.n();
    while (pending > 0 && !pq.empty()) {
        DsaturNode top = pq.top();
        pq.pop();
        
        int v = top.v;
        if (color[v] > 0 || top.sat != (int)sat[v].size()) {
            continue; // obsolete entry
        }
        
        int c = 1;
        while (sat[v].find(c) != sat[v].end()) {
            ++c;
        }
        color[v] = c;
        --pending;
        
        for (int u : g.neighbors(v)) {
            if (color[u] == 0 && sat[u].find(c) == sat[u].end()) {
                sat[u].insert(c);
                pq.push({(int)sat[u].size(), g.degree(u), u});
            }
        }
    }
    return color;
}

vector<int> greedy_clique(const Graph& g, int max_seeds) {
    if (g.n() == 0) return {};
    
    vector<int> by_deg = g.vertices();
    sort(by_deg.begin(), by_deg.end(), [&g](int a, int b) {
        if (g.degree(a) != g.degree(b)) return g.degree(a) > g.degree(b);
        return a < b;
    });
    
    vector<int> best = {by_deg[0]};
    int seeds_tested = 0;
    
    for (int seed : by_deg) {
        if (seeds_tested++ >= max_seeds) break;
        
        vector<int> clique = {seed};
        vector<int> neighbors(g.neighbors(seed).begin(), g.neighbors(seed).end());
        sort(neighbors.begin(), neighbors.end(), [&g](int a, int b) {
            if (g.degree(a) != g.degree(b)) return g.degree(a) > g.degree(b);
            return a < b;
        });
        
        for (int u : neighbors) {
            bool all_connected = true;
            for (int w : clique) {
                if (g.neighbors(u).find(w) == g.neighbors(u).end()) {
                    all_connected = false;
                    break;
                }
            }
            if (all_connected) {
                clique.push_back(u);
            }
        }
        
        if (clique.size() > best.size()) {
            best = clique;
        }
    }
    return best;
}

class Backtracker {
public:
    Backtracker(const Graph& graph, int k_val, const vector<int>& order, double timeout_s, int lower_bound, int upper_bound, const unordered_map<int, int>& upper_coloring) 
        : g(graph), k(k_val), ord(order), nodes(0), found(false),
          start_time(chrono::high_resolution_clock::now()), timeout(timeout_s),
          best_k(upper_bound), best_color(upper_coloring) {
              
        n = ord.size();
        for (int v : g.vertices()) color[v] = 0;
        
        uint64_t full = ((1ULL << k) - 1) << 1;
        for (int v : g.vertices()) dom[v] = full;
    }
    
    vector<int> propagate(int v, int c) {
        uint64_t bit = 1ULL << c;
        vector<int> touched;
        for (int u : g.neighbors(v)) {
            if (color[u] == 0 && (dom[u] & bit)) {
                dom[u] ^= bit;
                touched.push_back(u);
                if (dom[u] == 0) {
                    for (int w : touched) dom[w] |= bit;
                    return {-1}; // indicator for conflict
                }
            }
        }
        return touched;
    }
    
    bool extend(int i, int max_used) {
        nodes++;
        auto now = chrono::high_resolution_clock::now();
        chrono::duration<double> elapsed = now - start_time;
        if (timeout > 0 && elapsed.count() > timeout) {
            throw runtime_error("timeout");
        }
        
        if (i == n) {
            found = true;
            best_color = color;
            return true;
        }
        
        int v = ord[i];
        int max_allowed_c = min(max_used + 1, k);
        uint64_t mask = ((1ULL << max_allowed_c) - 1) << 1;
        uint64_t allowed = dom[v] & mask;
        
        while (allowed) {
            uint64_t bit = allowed & -allowed;
            allowed ^= bit;
            
            // Fast bit log2
            int c = 0;
            uint64_t temp = bit;
            while (temp >>= 1) c++;
            
            color[v] = c;
            vector<int> touched = propagate(v, c);
            
            if (touched.empty() || touched[0] != -1) {
                if (extend(i + 1, max(max_used, c))) return true;
                for (int u : touched) dom[u] |= bit;
            }
            color[v] = 0;
        }
        return false;
    }
    
    unordered_map<int, int> color;
    unordered_map<int, uint64_t> dom;
    const Graph& g;
    int k;
    const vector<int>& ord;
    int n;
    int nodes;
    bool found;
    
    chrono::time_point<chrono::high_resolution_clock> start_time;
    double timeout;
    
    int best_k;
    unordered_map<int, int> best_color;
};

ExactResult chromatic_number(const Graph& g, int max_n, double timeout_s) {
    auto start_time = chrono::high_resolution_clock::now();
    
    if (g.n() > max_n) {
        throw invalid_argument("n exceeds max_n; use Greedy or DSATUR");
    }
    if (g.n() == 0) {
        return {0, {}, true, 0, 0.0, 0, 0};
    }
    
    unordered_map<int, int> upper_coloring = dsatur(g);
    int upper = 0;
    for (auto& p : upper_coloring) upper = max(upper, p.second);
    
    vector<int> clique = greedy_clique(g);
    int lower = max((int)clique.size(), 1);
    
    auto get_elapsed = [&]() {
        auto now = chrono::high_resolution_clock::now();
        return chrono::duration<double>(now - start_time).count();
    };
    
    if (lower == upper) {
        return {upper, upper_coloring, true, 0, get_elapsed(), lower, upper};
    }
    
    unordered_set<int> clique_set(clique.begin(), clique.end());
    vector<int> rest;
    for (int v : g.vertices()) {
        if (clique_set.find(v) == clique_set.end()) {
            rest.push_back(v);
        }
    }
    sort(rest.begin(), rest.end(), [&g](int a, int b) {
        if (g.degree(a) != g.degree(b)) return g.degree(a) > g.degree(b);
        return a < b;
    });
    
    vector<int> order = clique;
    order.insert(order.end(), rest.begin(), rest.end());
    
    int total_nodes = 0;
    int q = min((int)clique.size(), upper);
    
    try {
        for (int k = lower; k < upper; ++k) {
            Backtracker bt(g, k, order, timeout_s, lower, upper, upper_coloring);
            
            bool possible = true;
            for (int i = 0; i < q; ++i) {
                bt.color[order[i]] = i + 1;
                vector<int> touched = bt.propagate(order[i], i + 1);
                if (!touched.empty() && touched[0] == -1) {
                    possible = false;
                    break;
                }
            }
            
            if (possible) {
                if (bt.extend(q, q)) {
                    total_nodes += bt.nodes;
                    return {k, bt.best_color, true, total_nodes, get_elapsed(), k, k};
                }
            }
            total_nodes += bt.nodes;
        }
        return {upper, upper_coloring, true, total_nodes, get_elapsed(), upper, upper};
    } catch (const runtime_error& e) { // timeout
        return {upper, upper_coloring, false, total_nodes, get_elapsed(), lower, upper};
    }
}
