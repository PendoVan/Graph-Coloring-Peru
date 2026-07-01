"""DSATUR (Brelaz 1979): O((n+m) log n) con heap binario + borrado perezoso."""
import heapq
from ..core.graph_model import Graph


def dsatur_coloring(graph: Graph) -> dict[int, int]:
    color: dict[int, int] = {v: 0 for v in graph.vertices()}
    sat: dict[int, int] = {v: 0 for v in graph.vertices()}
    sat_colors: dict[int, set[int]] = {v: set() for v in graph.vertices()}

    # heap de maximos simulado con negativos; entrada = (-sat, -grado, v)
    heap = [(0, -graph.degree(v), v) for v in graph.vertices()]
    heapq.heapify(heap)
    coloreados = set()

    while len(coloreados) < graph.n:
        neg_sat, neg_deg, v = heapq.heappop(heap)
        if v in coloreados:
            continue
        # entrada obsoleta (borrado perezoso): revalidar contra estado actual
        if neg_sat != -sat[v] or neg_deg != -graph.degree(v):
            heapq.heappush(heap, (-sat[v], -graph.degree(v), v))
            continue

        usados = {color[u] for u in graph.neighbors(v) if color[u] != 0}
        c = 1
        while c in usados:
            c += 1
        color[v] = c
        coloreados.add(v)

        for u in graph.neighbors(v):
            if color[u] == 0 and c not in sat_colors[u]:
                sat_colors[u].add(c)
                sat[u] += 1
                heapq.heappush(heap, (-sat[u], -graph.degree(u), u))

    return color