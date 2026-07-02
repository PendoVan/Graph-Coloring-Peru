"""Suite de pruebas: núcleo, algoritmos, generadores y mapa del Perú."""
import unittest

from src.algorithms import (chromatic_number, dsatur, greedy, greedy_clique,
                            order_distance, order_largest_first, order_natural,
                            order_smallest_last)
from src.core import Graph, is_proper, num_colors
from src.generators import geometric, gnp, p_for_avg_degree
from src.peru import build_peru_graph, centroids, geo_distance


def cycle(n):
    return Graph.from_edges(range(n), [(i, (i + 1) % n) for i in range(n)])


def complete(n):
    return Graph.from_edges(range(n), [(i, j) for i in range(n) for j in range(i)])


def bipartite(a, b):
    return Graph.from_edges(range(a + b), [(i, a + j) for i in range(a) for j in range(b)])


def petersen():
    e = ([(i, (i + 1) % 5) for i in range(5)]
         + [(5 + i, 5 + (i + 2) % 5) for i in range(5)]
         + [(i, 5 + i) for i in range(5)])
    return Graph.from_edges(range(10), e)


CASES = [("K4", complete(4), 4), ("C5", cycle(5), 3), ("C6", cycle(6), 2),
         ("K3,3", bipartite(3, 3), 2), ("Petersen", petersen(), 3)]
ORDERS = [order_natural, order_largest_first, order_smallest_last]


class TestCore(unittest.TestCase):
    def test_counts_and_loops(self):
        g = complete(4)
        self.assertEqual((g.n, g.m, g.max_degree()), (4, 6, 3))
        g.add_edge(9, 9)  # el lazo registra el vértice pero no crea arista
        self.assertEqual(g.degree(9), 0)

    def test_edges_unique(self):
        g = cycle(5)
        e = list(g.edges())
        self.assertEqual(len(e), 5)
        self.assertTrue(all(u < v for u, v in e))

    def test_is_proper(self):
        g = cycle(4)
        self.assertTrue(is_proper(g, {0: 1, 1: 2, 2: 1, 3: 2})[0])
        ok, edge = is_proper(g, {0: 1, 1: 1, 2: 1, 3: 2})
        self.assertFalse(ok)
        self.assertEqual(set(edge), {0, 1})
        self.assertFalse(is_proper(g, {0: 1, 1: 2, 2: 1, 3: 0})[0])  # sin colorear

    def test_num_colors_ignores_zero(self):
        self.assertEqual(num_colors({0: 1, 1: 2, 2: 0}), 2)


class TestGreedy(unittest.TestCase):
    def test_proper_and_bound(self):
        for name, g, chi in CASES:
            for order in ORDERS:
                col = greedy(g, order(g))
                self.assertTrue(is_proper(g, col)[0], f"{name}/{order.__name__}")
                self.assertLessEqual(num_colors(col), g.max_degree() + 1)
                self.assertGreaterEqual(num_colors(col), chi)

    def test_distance_order_is_permutation(self):
        g = petersen()
        order = order_distance(g, 0)
        self.assertEqual(sorted(order), list(range(10)))
        self.assertEqual(order[0], 0)
        self.assertTrue(is_proper(g, greedy(g, order))[0])

    def test_smallest_last_two_colors_on_tree(self):
        tree = Graph.from_edges(range(7), [(0, 1), (0, 2), (1, 3), (1, 4), (2, 5), (2, 6)])
        self.assertEqual(num_colors(greedy(tree, order_smallest_last(tree))), 2)


class TestDsatur(unittest.TestCase):
    def test_proper_and_bipartite_exact(self):
        for name, g, chi in CASES:
            col = dsatur(g)
            self.assertTrue(is_proper(g, col)[0], name)
            self.assertGreaterEqual(num_colors(col), chi)
        self.assertEqual(num_colors(dsatur(bipartite(4, 5))), 2)


class TestBacktracking(unittest.TestCase):
    def test_exact_chi(self):
        for name, g, chi in CASES:
            res = chromatic_number(g, timeout_s=30.0)
            self.assertTrue(res.exact, name)
            self.assertEqual(res.k, chi, name)
            self.assertTrue(is_proper(g, res.coloring)[0])

    def test_clique(self):
        self.assertEqual(len(greedy_clique(complete(5))), 5)
        self.assertLessEqual(len(greedy_clique(cycle(5))), 2)

    def test_max_n_guard(self):
        with self.assertRaises(ValueError):
            chromatic_number(Graph.from_edges(range(300), []), max_n=200)

    def test_timeout_returns_bounds(self):
        res = chromatic_number(cycle(5), timeout_s=0.0)
        self.assertFalse(res.exact)
        self.assertTrue(is_proper(cycle(5), res.coloring)[0])


class TestGenerators(unittest.TestCase):
    def test_gnp_extremes_and_reproducible(self):
        self.assertEqual(gnp(20, 1.0, seed=1).m, 190)
        self.assertEqual(gnp(50, 0.0, seed=1).m, 0)
        a, b = gnp(200, 0.05, seed=7), gnp(200, 0.05, seed=7)
        self.assertEqual(sorted(a.edges()), sorted(b.edges()))

    def test_gnp_expected_m(self):
        g = gnp(1000, p_for_avg_degree(1000, 10.0), seed=42)
        self.assertTrue(4000 < g.m < 6000)

    def test_geometric_grid_matches_bruteforce(self):
        import math
        n = 200
        g, pts = geometric(n, avg_degree=6.0, seed=9, return_coords=True)
        r2 = (math.sqrt(6.0 / (math.pi * n))) ** 2
        expected = {(min(i, j), max(i, j)) for i in range(n) for j in range(i + 1, n)
                    if (pts[i][0] - pts[j][0]) ** 2 + (pts[i][1] - pts[j][1]) ** 2 <= r2}
        self.assertEqual(set(g.edges()), expected)


class TestPeru(unittest.TestCase):
    def setUp(self):
        self.g = build_peru_graph()
        self.by_name = {self.g.label(v): v for v in self.g.vertices()}

    def test_size_and_degrees(self):
        self.assertEqual((self.g.n, self.g.m), (25, 53))
        for name, deg in {"Lima": 7, "Cusco": 7, "Huánuco": 7, "Callao": 1,
                          "Tumbes": 1, "Tacna": 2}.items():
            self.assertEqual(self.g.degree(self.by_name[name]), deg, name)

    def test_contains_k4(self):
        clique = [self.by_name[d] for d in ("Cusco", "Apurímac", "Ayacucho", "Arequipa")]
        for i, u in enumerate(clique):
            for w in clique[i + 1:]:
                self.assertIn(w, self.g.neighbors(u))

    def test_chi_is_4(self):
        res = chromatic_number(self.g, timeout_s=60.0)
        self.assertTrue(res.exact)
        self.assertEqual(res.k, 4)

    def test_geometry_present(self):
        self.assertEqual(len(centroids()), 25)
        self.assertGreater(geo_distance(self.by_name["Tumbes"], self.by_name["Tacna"]), 1500)


if __name__ == "__main__":
    unittest.main()
