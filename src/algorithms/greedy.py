"""Greedy_Coloring: O(n+m). Ver Seccion 3.1 / 4.1 del Informe 1."""
from ..core.graph_model import Graph


def _order_natural(g: Graph) -> list[int]:
    return list(g.vertices())


def _order_largest_first(g: Graph) -> list[int]:
    return sorted(g.vertices(), key=lambda v: g.degree(v), reverse=True)


def _order_welsh_powell(g: Graph) -> list[int]:
    # equivalente a largest-first; se mantiene separado por nombre academico
    return _order_largest_first(g)


_ORDERS = {
    "natural": _order_natural,
    "largest_first": _order_largest_first,
    "welsh_powell": _order_welsh_powell,
}


def greedy_coloring(graph: Graph, order: str = "largest_first") -> dict[int, int]:
    if order not in _ORDERS:
        raise ValueError(f"orden desconocido: {order}")
    secuencia = _ORDERS[order](graph)

    color: dict[int, int] = {v: 0 for v in graph.vertices()}
    for v in secuencia:
        usados = {color[u] for u in graph.neighbors(v) if color[u] != 0}
        c = 1
        while c in usados:
            c += 1
        color[v] = c
    return color