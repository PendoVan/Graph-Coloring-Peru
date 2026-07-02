"""Generadores de instancias aleatorias dispersas para los experimentos.

Ambos producen grafos con m = Theta(n) en tiempo esperado Theta(n+m) y son
deterministas dada la semilla.
"""
from __future__ import annotations

import math
import random

from .core import Graph


def p_for_avg_degree(n: int, d: float) -> float:
    return min(1.0, d / max(n - 1, 1))


def gnp(n: int, p: float, seed: int | None = None) -> Graph:
    """Erdos-Renyi G(n,p) por muestreo de saltos (Batagelj-Brandes 2005): salta
    de una arista presente a la siguiente con distancia geometrica, sin recorrer
    los O(n^2) pares. Tiempo esperado Theta(n + m)."""
    g = Graph()
    for v in range(n):
        g.add_vertex(v)
    if n < 2 or p <= 0:
        return g
    if p >= 1:
        for v in range(n):
            for w in range(v):
                g.add_edge(v, w)
        return g
    rng = random.Random(seed)
    log_q = math.log1p(-p)
    v, w = 1, -1
    while v < n:
        w += 1 + int(math.log1p(-rng.random()) / log_q)
        while w >= v and v < n:
            w -= v
            v += 1
        if v < n:
            g.add_edge(v, w)
    return g


def geometric(n: int, avg_degree: float = 6.0, seed: int | None = None,
              return_coords: bool = False):
    """Grafo geometrico aleatorio: n puntos en [0,1]^2, arista si distancia <= r.
    Una rejilla de celdas r x r restringe las comparaciones a celdas vecinas, de
    modo que el costo esperado es Theta(n + m). Modela el FAP: antenas cercanas
    interfieren."""
    radius = math.sqrt(avg_degree / (math.pi * n))
    rng = random.Random(seed)
    pts = [(rng.random(), rng.random()) for _ in range(n)]
    g = Graph()
    for v in range(n):
        g.add_vertex(v)
    grid: dict[tuple[int, int], list[int]] = {}
    for i, (x, y) in enumerate(pts):
        grid.setdefault((int(x / radius), int(y / radius)), []).append(i)
    r2 = radius * radius
    stencil = ((0, 0), (1, 0), (0, 1), (1, 1), (1, -1))
    for (cx, cy), here in grid.items():
        for dx, dy in stencil:
            there = here if (dx, dy) == (0, 0) else grid.get((cx + dx, cy + dy))
            if not there:
                continue
            for a, i in enumerate(here):
                xi, yi = pts[i]
                start = a + 1 if (dx, dy) == (0, 0) else 0
                for j in there[start:]:
                    xj, yj = pts[j]
                    if (xi - xj) ** 2 + (yi - yj) ** 2 <= r2:
                        g.add_edge(i, j)
    return (g, pts) if return_coords else g
