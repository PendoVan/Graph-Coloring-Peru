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
    int max_id = g.max_id();
    if (max_id < 0) return {};
    
    vector<int> deg(max_id + 1, 0);
    int max_deg = g.max_degree();
    
    // Array-based doubly linked list for O(1) removal and insertion
    vector<int> prev(max_id + 1, -1);
    vector<int> next(max_id + 1, -1);
    vector<int> head(max_deg + 1, -1);
    
    for (int v : g.vertices()) {
        int d = g.degree(v);
        deg[v] = d;
        if (head[d] != -1) prev[head[d]] = v;
        next[v] = head[d];
        head[d] = v;
    }
    
    vector<bool> removed(max_id + 1, false);
    vector<int> seq;
    seq.reserve(g.n());
    
    int i = 0;
    for (int count = 0; count < g.n(); ++count) {
        while (i <= max_deg && head[i] == -1) {
            ++i;
        }
        if (i > max_deg) break;
        
        int v = head[i];
        head[i] = next[v];
        if (head[i] != -1) prev[head[i]] = -1;
        
        removed[v] = true;
        seq.push_back(v);
        
        for (int u : g.neighbors(v)) {
            if (!removed[u]) {
                int d = deg[u];
                // Remove u from current bucket
                if (prev[u] != -1) next[prev[u]] = next[u];
                else head[d] = next[u];
                if (next[u] != -1) prev[next[u]] = prev[u];
                
                // Add u to bucket d-1
                deg[u] = d - 1;
                prev[u] = -1;
                next[u] = head[d - 1];
                if (head[d - 1] != -1) prev[head[d - 1]] = u;
                head[d - 1] = u;
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

vector<int> bfs_levels(const Graph& g, int source) {
    int max_id = g.max_id();
    if (max_id < 0) return {};
    
    vector<int> dist(max_id + 1, -1);
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

vector<int> greedy(const Graph& g, const vector<int>& order) {
    int max_id = g.max_id();
    if (max_id < 0) return {};
    
    vector<int> color(max_id + 1, 0);
    vector<int> used(max_id + 2, -1); // color -> vertex id to know if used in current step
    
    for (int v : order) {
        for (int u : g.neighbors(v)) {
            if (color[u] > 0) used[color[u]] = v;
        }
        int c = 1;
        while (c < used.size() && used[c] == v) {
            ++c;
        }
        if (c >= used.size()) {
            used.push_back(-1); // Expand if max colors exceeded
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

vector<int> dsatur(const Graph& g) {
    int max_id = g.max_id();
    if (max_id < 0) return {};
    
    int max_deg = g.max_degree();
    vector<int> color(max_id + 1, 0);
    vector<int> sat_deg(max_id + 1, 0);
    // Track if neighbor has color c: num_adj_color[u][c]
    // Since colors never exceed max_deg + 1, we can size it
    vector<vector<bool>> has_adj_color(max_id + 1, vector<bool>(max_deg + 2, false));
    
    priority_queue<DsaturNode> pq;
    
    for (int v : g.vertices()) {
        pq.push({0, g.degree(v), v});
    }
    
    int pending = g.n();
    while (pending > 0 && !pq.empty()) {
        DsaturNode top = pq.top();
        pq.pop();
        
        int v = top.v;
        if (color[v] > 0 || top.sat != sat_deg[v]) {
            continue; // obsolete entry
        }
        
        int c = 1;
        while (c <= max_deg + 1 && has_adj_color[v][c]) {
            ++c;
        }
        color[v] = c;
        --pending;
        
        for (int u : g.neighbors(v)) {
            if (color[u] == 0 && !has_adj_color[u][c]) {
                has_adj_color[u][c] = true;
                sat_deg[u]++;
                pq.push({sat_deg[u], g.degree(u), u});
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
                const auto& n_u = g.neighbors(u);
                if (std::find(n_u.begin(), n_u.end(), w) == n_u.end()) {
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
    Backtracker(const Graph& graph, int k_val, const vector<int>& order, double timeout_s, int lower_bound, int upper_bound, const vector<int>& upper_coloring) 
        : g(graph), k(k_val), ord(order), nodes(0), found(false),
          start_time(chrono::high_resolution_clock::now()), timeout(timeout_s),
          best_k(upper_bound), best_color(upper_coloring) {
              
        n = ord.size();
        int max_id = g.max_id();
        color.assign(max_id + 1, 0);
        dom.assign(max_id + 1, 0);
        
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
    
    vector<int> color;
    vector<uint64_t> dom;
    const Graph& g;
    int k;
    const vector<int>& ord;
    int n;
    int nodes;
    bool found;
    
    chrono::time_point<chrono::high_resolution_clock> start_time;
    double timeout;
    
    int best_k;
    vector<int> best_color;
};

ExactResult chromatic_number(const Graph& g, int max_n, double timeout_s) {
    auto start_time = chrono::high_resolution_clock::now();
    
    if (g.n() > max_n) {
        throw invalid_argument("n exceeds max_n; use Greedy or DSATUR");
    }
    if (g.n() == 0) {
        return {0, {}, true, 0, 0.0, 0, 0};
    }
    
    vector<int> upper_coloring = dsatur(g);
    int upper = 0;
    for (int c : upper_coloring) upper = max(upper, c);
    
    vector<int> clique = greedy_clique(g);
    int lower = max((int)clique.size(), 1);
    
    auto get_elapsed = [&]() {
        auto now = chrono::high_resolution_clock::now();
        return chrono::duration<double>(now - start_time).count();
    };
    
    if (lower == upper) {
        return {upper, upper_coloring, true, 0, get_elapsed(), lower, upper};
    }
    
    vector<bool> clique_set(g.max_id() + 1, false);
    for (int v : clique) clique_set[v] = true;
    vector<int> rest;
    for (int v : g.vertices()) {
        if (!clique_set[v]) {
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
