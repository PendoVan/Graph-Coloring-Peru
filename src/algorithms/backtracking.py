"""Backtracking exacto: O(k^n) peor caso. Ver Seccion 3.3 / 4.1 del Informe 1."""
import time
from ..core.graph_model import Graph


class TimeoutError_(Exception):
    pass


def _try_k_coloring(graph: Graph, k: int, orden: list[int],
                     deadline: float | None) -> dict[int, int] | None:
    color: dict[int, int] = {v: 0 for v in graph.vertices()}
    pos = {v: i for i, v in enumerate(orden)}
    n = len(orden)

    def factible(v: int, c: int) -> bool:
        for u in graph.neighbors(v):
            if color[u] == c:
                return False
        return True

    def colorear(i: int) -> bool:
        if deadline is not None and time.perf_counter() > deadline:
            raise TimeoutError_()
        if i == n:
            return True
        v = orden[i]
        for c in range(1, k + 1):
            if factible(v, c):
                color[v] = c
                if colorear(i + 1):
                    return True
                color[v] = 0  # deshacer (backtrack)
        return False

    return color if colorear(0) else None


def chromatic_number(graph: Graph, max_n: int = 200,
                      timeout_s: float | None = 30.0) -> tuple[int, dict[int, int]]:
    """Busca el minimo k factible probando k=1,2,3,...
    Guardas: max_n limita el tamano de instancia (costo exponencial);
    timeout_s aborta si una prueba de k tarda demasiado."""
    if graph.n > max_n:
        raise ValueError(f"n={graph.n} excede max_n={max_n}; usar Greedy/DSATUR")

    orden = sorted(graph.vertices(), key=lambda v: graph.degree(v), reverse=True)
    deadline = None if timeout_s is None else time.perf_counter() + timeout_s

    for k in range(1, graph.n + 1):
        resultado = _try_k_coloring(graph, k, orden, deadline)
        if resultado is not None:
            return k, resultado
    return graph.n, {v: i + 1 for i, v in enumerate(orden)}