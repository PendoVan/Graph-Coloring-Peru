"""Grafo no dirigido disperso y verificacion de coloreo.

Representacion por listas de adyacencia (dict de conjuntos): espacio Theta(n+m),
optimo para grafos dispersos como el mapa del Peru. El color 0 significa
"sin colorear".
"""
from __future__ import annotations


class Graph:
    __slots__ = ("adj", "labels")

    def __init__(self) -> None:
        self.adj: dict[int, set[int]] = {}
        self.labels: dict[int, str] = {}

    def add_vertex(self, v: int, label: str | None = None) -> None:
        self.adj.setdefault(v, set())
        if label is not None:
            self.labels[v] = label

    def add_edge(self, u: int, v: int) -> None:
        self.add_vertex(u)
        self.add_vertex(v)
        if u != v:  # se ignoran los lazos: una antena no interfiere consigo misma
            self.adj[u].add(v)
            self.adj[v].add(u)

    @classmethod
    def from_edges(cls, vertices, edges, labels=None) -> "Graph":
        g = cls()
        for v in vertices:
            g.add_vertex(v, labels.get(v) if labels else None)
        for u, v in edges:
            g.add_edge(u, v)
        return g

    def neighbors(self, v: int) -> set[int]:
        return self.adj[v]

    def degree(self, v: int) -> int:
        return len(self.adj[v])

    def vertices(self):
        return self.adj.keys()

    def edges(self):
        # cada arista se emite una sola vez (cuando u < v); espacio extra O(1)
        for u in self.adj:
            for v in self.adj[u]:
                if u < v:
                    yield u, v

    def label(self, v: int) -> str:
        return self.labels.get(v, str(v))

    @property
    def n(self) -> int:
        return len(self.adj)

    @property
    def m(self) -> int:
        return sum(map(len, self.adj.values())) // 2

    def max_degree(self) -> int:
        return max(map(len, self.adj.values()), default=0)

    def __repr__(self) -> str:
        return f"Graph(n={self.n}, m={self.m})"


def is_proper(graph: Graph, color: dict[int, int]):
    """(True, None) si es coloreo propio; (False, arista) en el primer conflicto."""
    for u, v in graph.edges():
        if not color.get(u) or not color.get(v) or color[u] == color[v]:
            return False, (u, v)
    return True, None


def num_colors(color: dict[int, int]) -> int:
    return len({c for c in color.values() if c})
