"""Validacion de coloracion propia: O(n+m)."""
from .graph_model import Graph


def is_proper(graph: Graph, color: dict[int, int]) -> tuple[bool, tuple[int, int] | None]:
    """Retorna (True, None) si phi(u) != phi(v) para toda arista (u,v).
    Si falla, retorna (False, (u,v)) con la primera arista en conflicto."""
    for u, v in graph.edges():
        if color.get(u, 0) == 0 or color.get(v, 0) == 0:
            return False, (u, v)
        if color[u] == color[v]:
            return False, (u, v)
    return True, None


def num_colors(color: dict[int, int]) -> int:
    return len(set(color.values()))


def summary(graph: Graph, color: dict[int, int], algo_name: str = "") -> str:
    ok, conflict = is_proper(graph, color)
    k = num_colors(color)
    status = "VALIDA" if ok else f"INVALIDA en {conflict}"
    return f"[{algo_name}] n={graph.n} m={graph.m} colores={k} -> {status}"