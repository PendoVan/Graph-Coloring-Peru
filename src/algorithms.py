"""Algoritmos de coloreo de grafos.

Ordenamientos + greedy en Theta(n+m), DSATUR en O((n+m) log n) y backtracking
exacto (exponencial, con cotas y podas). El analisis de complejidad por funcion
esta en docs/analisis_complejidad.pdf.
"""
from __future__ import annotations

import heapq
import sys
import time
from collections import deque
from dataclasses import dataclass

from .core import Graph, num_colors


# --------------------------------------------------------------------------
# Ordenamientos de vertices (deciden en que orden greedy colorea)
# --------------------------------------------------------------------------
def order_natural(g: Graph) -> list[int]:
    return list(g.vertices())


def order_largest_first(g: Graph) -> list[int]:
    """Grado decreciente por bucket sort: Theta(n + Delta), evita el n log n."""
    buckets: list[list[int]] = [[] for _ in range(g.max_degree() + 1)]
    for v in g.vertices():
        buckets[g.degree(v)].append(v)
    return [v for d in range(len(buckets) - 1, -1, -1) for v in buckets[d]]


def order_smallest_last(g: Graph) -> list[int]:
    """Orden de degenerescencia (Matula-Beck): extrae repetidamente el vertice
    de grado minimo y devuelve la secuencia invertida. Theta(n+m) amortizado."""
    deg = {v: g.degree(v) for v in g.vertices()}
    buckets: list[set[int]] = [set() for _ in range(g.max_degree() + 1)]
    for v, d in deg.items():
        buckets[d].add(v)
    removed: set[int] = set()
    seq: list[int] = []
    i = 0
    for _ in range(g.n):
        while not buckets[i]:
            i += 1
        v = buckets[i].pop()
        removed.add(v)
        seq.append(v)
        for u in g.neighbors(v):
            if u not in removed:
                buckets[deg[u]].discard(u)
                deg[u] -= 1
                buckets[deg[u]].add(u)
        i = max(i - 1, 0)
    seq.reverse()
    return seq


def order_distance(g: Graph, source: int) -> list[int]:
    """Recorrido BFS desde `source`: los vertices se ordenan por distancia
    (numero de fronteras) al departamento elegido. Sobre un mapa plano la
    distancia en aristas sigue a la lejania geografica. Theta(n+m)."""
    seen = {source}
    queue = deque([source])
    seq: list[int] = []
    while queue:
        v = queue.popleft()
        seq.append(v)
        for u in sorted(g.neighbors(v), key=g.degree, reverse=True):
            if u not in seen:
                seen.add(u)
                queue.append(u)
    seq.extend(v for v in g.vertices() if v not in seen)  # componentes aisladas
    return seq


def bfs_levels(g: Graph, source: int) -> dict[int, int]:
    """Distancia en aristas de `source` a cada vertice (inalcanzables = -1)."""
    dist = {v: -1 for v in g.vertices()}
    dist[source] = 0
    queue = deque([source])
    while queue:
        v = queue.popleft()
        for u in g.neighbors(v):
            if dist[u] == -1:
                dist[u] = dist[v] + 1
                queue.append(u)
    return dist


# --------------------------------------------------------------------------
# Greedy
# --------------------------------------------------------------------------
def greedy(graph: Graph, order: list[int]) -> dict[int, int]:
    """Asigna a cada vertice, en el orden dado, el menor color no usado por sus
    vecinos ya coloreados. Theta(n+m); a lo mas Delta+1 colores."""
    color = {v: 0 for v in graph.vertices()}
    for v in order:
        used = {color[u] for u in graph.neighbors(v) if color[u]}
        c = 1
        while c in used:
            c += 1
        color[v] = c
    return color


# --------------------------------------------------------------------------
# DSATUR (Brelaz 1979)
# --------------------------------------------------------------------------
def dsatur(graph: Graph) -> dict[int, int]:
    """Colorea siempre el vertice de mayor saturacion (colores distintos en su
    vecindad), desempatando por grado. Heap binario con borrado perezoso:
    O((n+m) log n)."""
    color = {v: 0 for v in graph.vertices()}
    sat: dict[int, set[int]] = {v: set() for v in graph.vertices()}
    heap = [(0, -graph.degree(v), v) for v in graph.vertices()]
    heapq.heapify(heap)
    pending = graph.n
    while pending:
        neg_sat, _, v = heapq.heappop(heap)
        if color[v] or -neg_sat != len(sat[v]):
            continue  # entrada obsoleta (ya coloreado o saturacion vieja)
        c = 1
        while c in sat[v]:  # sat[v] es el conjunto de colores prohibidos
            c += 1
        color[v] = c
        pending -= 1
        for u in graph.neighbors(v):
            if not color[u] and c not in sat[u]:
                sat[u].add(c)
                heapq.heappush(heap, (-len(sat[u]), -graph.degree(u), u))
    return color


# --------------------------------------------------------------------------
# Backtracking exacto
# --------------------------------------------------------------------------
@dataclass
class ExactResult:
    k: int                     # colores de la coloracion devuelta
    coloring: dict[int, int]
    exact: bool                # True si se probo k == chi(G)
    nodes: int                 # nodos del arbol de busqueda explorados
    elapsed: float
    lower: int                 # cota inferior demostrada de chi(G)
    upper: int                 # cota superior demostrada de chi(G)


class _Timeout(Exception):
    pass


def greedy_clique(graph: Graph, max_seeds: int = 100) -> list[int]:
    """Clique grande hallada golosamente: da la cota inferior chi(G) >= |clique|."""
    if graph.n == 0:
        return []
    by_deg = sorted(graph.vertices(), key=lambda v: (-graph.degree(v), v))
    best = [by_deg[0]]
    for seed in by_deg[:max_seeds]:
        clique = [seed]
        for u in sorted(graph.neighbors(seed), key=lambda v: (-graph.degree(v), v)):
            adj_u = graph.neighbors(u)
            if all(w in adj_u for w in clique):
                clique.append(u)
        if len(clique) > len(best):
            best = clique
    return best


def _search(graph, k, order, q, deadline, nodes):
    """Busca una k-coloracion. Podas: clique pre-coloreada 1..q, cota canonica
    (color <= 1+max usado) y forward checking con dominios de bits."""
    n = len(order)
    color = {v: 0 for v in graph.vertices()}
    full = ((1 << k) - 1) << 1
    dom = {v: full for v in graph.vertices()}

    def propagate(v, c):
        bit = 1 << c
        touched = []
        for u in graph.neighbors(v):
            if not color[u] and dom[u] & bit:
                dom[u] ^= bit
                touched.append(u)
                if not dom[u]:
                    for w in touched:
                        dom[w] |= bit
                    return None
        return touched

    for i in range(q):  # pre-colorear la clique con colores 1..q
        color[order[i]] = i + 1
        if propagate(order[i], i + 1) is None:
            return None

    def extend(i, max_used):
        nodes[0] += 1
        if deadline and time.perf_counter() > deadline:
            raise _Timeout
        if i == n:
            return True
        v = order[i]
        allowed = dom[v] & (((1 << min(max_used + 1, k)) - 1) << 1)
        while allowed:
            bit = allowed & -allowed
            allowed ^= bit
            c = bit.bit_length() - 1
            color[v] = c
            touched = propagate(v, c)
            if touched is not None:
                if extend(i + 1, max(max_used, c)):
                    return True
                for u in touched:
                    dom[u] |= bit
            color[v] = 0
        return False

    return dict(color) if extend(q, q) else None


def chromatic_number(graph: Graph, max_n: int = 200,
                     timeout_s: float | None = 30.0) -> ExactResult:
    """chi(G) exacto. Arranca en la cota inferior por clique y sube hasta la cota
    superior de DSATUR; si coinciden, no hay busqueda. Peor caso exponencial;
    max_n y timeout_s lo acotan y devuelven la mejor coloracion conocida."""
    t0 = time.perf_counter()
    if graph.n > max_n:
        raise ValueError(f"n={graph.n} excede max_n={max_n}; use Greedy o DSATUR")
    if graph.n == 0:
        return ExactResult(0, {}, True, 0, 0.0, 0, 0)

    upper_coloring = dsatur(graph)
    upper = num_colors(upper_coloring)
    clique = greedy_clique(graph)
    lower = max(len(clique), 1)
    if lower == upper:
        return ExactResult(upper, upper_coloring, True, 0,
                           time.perf_counter() - t0, lower, upper)

    rest = sorted((v for v in graph.vertices() if v not in set(clique)),
                  key=lambda v: (-graph.degree(v), v))
    order = clique + rest
    deadline = None if timeout_s is None else t0 + timeout_s
    nodes = [0]
    limit = sys.getrecursionlimit()
    sys.setrecursionlimit(max(limit, graph.n + 200))
    try:
        for k in range(lower, upper):
            found = _search(graph, k, order, min(len(clique), k), deadline, nodes)
            if found is not None:
                return ExactResult(k, found, True, nodes[0],
                                   time.perf_counter() - t0, k, k)
        return ExactResult(upper, upper_coloring, True, nodes[0],
                           time.perf_counter() - t0, upper, upper)
    except _Timeout:
        return ExactResult(upper, upper_coloring, False, nodes[0],
                           time.perf_counter() - t0, lower, upper)
    finally:
        sys.setrecursionlimit(limit)
