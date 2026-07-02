"""Mapa del Peru: carga los 25 departamentos (poligonos, centroides y fronteras)
desde data/peru_map.json y construye el grafo de interferencia.

El grafo, la adyacencia y los centroides derivan del mismo GeoJSON oficial: dos
departamentos son adyacentes si sus poligonos comparten frontera. chi(Peru)=4
(contiene el K4 {Cusco, Apurimac, Ayacucho, Arequipa} y es un mapa plano).
"""
from __future__ import annotations

import json
import math
from functools import lru_cache
from pathlib import Path

from .core import Graph

_DATA = Path(__file__).resolve().parent.parent / "data" / "peru_map.json"


@lru_cache(maxsize=1)
def load_map() -> dict:
    raw = json.loads(_DATA.read_text(encoding="utf-8"))
    return {
        "deptos": {int(k): v for k, v in raw["deptos"].items()},
        "aristas": [tuple(a) for a in raw["aristas"]],
        "capitales": {int(k): v for k, v in raw.get("capitales", {}).items()},
        "rutas": raw.get("rutas", {}),
    }


def build_peru_graph() -> Graph:
    m = load_map()
    labels = {i: d["nombre"] for i, d in m["deptos"].items()}
    return Graph.from_edges(labels.keys(), m["aristas"], labels)


def centroids() -> dict[int, tuple[float, float]]:
    return {i: tuple(d["centroide"]) for i, d in load_map()["deptos"].items()}


def rings() -> dict[int, list[list[list[float]]]]:
    return {i: d["anillos"] for i, d in load_map()["deptos"].items()}


def capitals() -> dict[int, tuple[str, tuple[float, float]]]:
    """id -> (nombre de la capital, (lon, lat))."""
    return {i: (c["nombre"], tuple(c["coord"]))
            for i, c in load_map()["capitales"].items()}


def routes() -> dict[tuple[int, int], dict]:
    """(u,v) con u<v -> {"via": polilínea siguiendo carreteras, "recta": bool}.
    Cada arista de interferencia se ruteó sobre la red vial nacional (OSM);
    las que no tienen conexión vial (p.ej. Loreto/Iquitos) quedan como recta."""
    out = {}
    for k, r in load_map()["rutas"].items():
        a, b = map(int, k.split("-"))
        out[(a, b)] = r
    return out


def geo_distance(a: int, b: int) -> float:
    """Distancia great-circle (km) entre los centroides de dos departamentos."""
    c = centroids()
    (lon1, lat1), (lon2, lat2) = c[a], c[b]
    p1, p2 = math.radians(lat1), math.radians(lat2)
    dp, dl = math.radians(lat2 - lat1), math.radians(lon2 - lon1)
    h = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
    return 2 * 6371 * math.asin(math.sqrt(h))


def frequency_table(graph: Graph, color: dict[int, int]) -> dict[int, list[str]]:
    """Color (frecuencia) -> lista ordenada de departamentos que lo usan."""
    table: dict[int, list[str]] = {}
    for v in sorted(graph.vertices(), key=graph.label):
        table.setdefault(color[v], []).append(graph.label(v))
    return dict(sorted(table.items()))
