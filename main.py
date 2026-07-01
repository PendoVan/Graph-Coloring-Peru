"""Criterio de cierre Fase A: los 3 algoritmos producen coloracion propia
sobre el grafo del Peru y se compara su numero de colores."""
import time
from src.generators.peru_map import build_peru_graph
from src.algorithms.greedy import greedy_coloring
from src.algorithms.dsatur import dsatur_coloring
from src.algorithms.backtracking import chromatic_number
from src.core.validator import summary

if __name__ == "__main__":
    g = build_peru_graph()
    print(g, "\n")

    t0 = time.perf_counter()
    c_greedy = greedy_coloring(g, order="largest_first")
    t1 = time.perf_counter()
    print(summary(g, c_greedy, "Greedy"), f"tiempo={t1 - t0:.6f}s")

    t0 = time.perf_counter()
    c_dsatur = dsatur_coloring(g)
    t1 = time.perf_counter()
    print(summary(g, c_dsatur, "DSATUR"), f"tiempo={t1 - t0:.6f}s")

    t0 = time.perf_counter()
    k, c_exact = chromatic_number(g, max_n=200, timeout_s=60.0)
    t1 = time.perf_counter()
    print(summary(g, c_exact, "Backtracking"), f"chi(G)={k} tiempo={t1 - t0:.6f}s")