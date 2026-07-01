"""Grafo disperso mediante listas de adyacencia. Espacio Theta(n+m)."""
from __future__ import annotations


class Graph:
    __slots__ = ("_adj", "_labels")

    def __init__(self):
        self._adj: dict[int, set[int]] = {}
        self._labels: dict[int, str] = {}

    # --- construccion ---
    def add_vertex(self, v: int, label: str | None = None) -> None:
        if v not in self._adj:
            self._adj[v] = set()
        if label is not None:
            self._labels[v] = label

    def add_edge(self, u: int, v: int) -> None:
        if u == v:
            return
        self.add_vertex(u)
        self.add_vertex(v)
        self._adj[u].add(v)
        self._adj[v].add(u)

    @classmethod
    def from_edge_list(cls, vertices: list[int], edges: list[tuple[int, int]],
                        labels: dict[int, str] | None = None) -> "Graph":
        g = cls()
        for v in vertices:
            g.add_vertex(v, labels.get(v) if labels else None)
        for u, v in edges:
            g.add_edge(u, v)
        return g

    @classmethod
    def from_networkx(cls, nx_graph) -> "Graph":
        g = cls()
        for v in nx_graph.nodes:
            g.add_vertex(v)
        for u, v in nx_graph.edges:
            g.add_edge(u, v)
        return g

    def to_networkx(self):
        import networkx as nx
        G = nx.Graph()
        G.add_nodes_from(self.vertices())
        G.add_edges_from(self.edges())
        return G

    # --- consultas O(1) / O(deg) ---
    def neighbors(self, v: int):
        return self._adj[v]

    def degree(self, v: int) -> int:
        return len(self._adj[v])

    def vertices(self):
        return self._adj.keys()

    def edges(self):
        seen = set()
        for u in self._adj:
            for v in self._adj[u]:
                if (v, u) not in seen:
                    seen.add((u, v))
                    yield (u, v)

    def label(self, v: int) -> str:
        return self._labels.get(v, str(v))

    @property
    def n(self) -> int:
        return len(self._adj)

    @property
    def m(self) -> int:
        return sum(len(s) for s in self._adj.values()) // 2

    def max_degree(self) -> int:
        return max((len(s) for s in self._adj.values()), default=0)

    def __repr__(self):
        return f"Graph(n={self.n}, m={self.m})"