"""Grafo de interferencia = fronteras compartidas entre los 24 departamentos
+ Callao (25 nodos). Caso pequeno verificable a mano (equivalente al mapa
de Australia del enunciado). Coordenadas aproximadas para layout en GUI."""
from ..core.graph_model import Graph

DEPARTAMENTOS: dict[int, str] = {
    1: "Amazonas", 2: "Ancash", 3: "Apurimac", 4: "Arequipa", 5: "Ayacucho",
    6: "Cajamarca", 7: "Callao", 8: "Cusco", 9: "Huancavelica", 10: "Huanuco",
    11: "Ica", 12: "Junin", 13: "La Libertad", 14: "Lambayeque", 15: "Lima",
    16: "Loreto", 17: "Madre de Dios", 18: "Moquegua", 19: "Pasco", 20: "Piura",
    21: "Puno", 22: "San Martin", 23: "Tacna", 24: "Tumbes", 25: "Ucayali",
}

# (lat, lon) aproximados del centroide de cada departamento
COORDS: dict[int, tuple[float, float]] = {
    1: (-6.0, -78.0), 2: (-9.3, -77.5), 3: (-14.0, -73.0), 4: (-16.4, -71.5),
    5: (-13.5, -74.0), 6: (-7.0, -78.5), 7: (-12.05, -77.13), 8: (-13.5, -72.0),
    9: (-12.8, -75.0), 10: (-9.9, -76.2), 11: (-14.3, -75.7), 12: (-11.5, -75.3),
    13: (-8.0, -78.5), 14: (-6.7, -79.8), 15: (-11.8, -76.5), 16: (-4.5, -74.0),
    17: (-12.0, -70.0), 18: (-17.0, -70.9), 19: (-10.5, -75.7), 20: (-5.2, -80.3),
    21: (-15.3, -70.0), 22: (-7.0, -76.5), 23: (-17.9, -70.3), 24: (-3.7, -80.5),
    25: (-9.0, -74.5),
}

# fronteras compartidas (aproximacion geografica estandar)
_FRONTERAS: list[tuple[str, str]] = [
    ("Tumbes", "Piura"),
    ("Piura", "Lambayeque"), ("Piura", "Cajamarca"),
    ("Lambayeque", "Cajamarca"), ("Lambayeque", "La Libertad"),
    ("Cajamarca", "La Libertad"), ("Cajamarca", "Amazonas"),
    ("La Libertad", "Amazonas"), ("La Libertad", "San Martin"), ("La Libertad", "Ancash"),
    ("Amazonas", "San Martin"), ("Amazonas", "Loreto"),
    ("San Martin", "Loreto"), ("San Martin", "Huanuco"), ("San Martin", "Ucayali"),
    ("Loreto", "Ucayali"), ("Loreto", "Huanuco"),
    ("Ancash", "Huanuco"), ("Ancash", "Lima"), ("Ancash", "Pasco"),
    ("Huanuco", "Pasco"), ("Huanuco", "Ucayali"), ("Huanuco", "Lima"),
    ("Pasco", "Ucayali"), ("Pasco", "Junin"), ("Pasco", "Lima"),
    ("Junin", "Lima"), ("Junin", "Huancavelica"), ("Junin", "Ucayali"),
    ("Junin", "Cusco"), ("Junin", "Ayacucho"),
    ("Lima", "Huancavelica"), ("Lima", "Ica"), ("Lima", "Callao"),
    ("Huancavelica", "Ica"), ("Huancavelica", "Ayacucho"),
    ("Ica", "Ayacucho"), ("Ica", "Arequipa"),
    ("Ayacucho", "Apurimac"), ("Ayacucho", "Cusco"), ("Ayacucho", "Arequipa"),
    ("Apurimac", "Cusco"), ("Apurimac", "Arequipa"),
    ("Cusco", "Arequipa"), ("Cusco", "Puno"), ("Cusco", "Madre de Dios"), ("Cusco", "Ucayali"),
    ("Arequipa", "Puno"), ("Arequipa", "Moquegua"),
    ("Puno", "Moquegua"), ("Puno", "Madre de Dios"),
    ("Moquegua", "Tacna"),
]

_NOMBRE_A_ID = {v: k for k, v in DEPARTAMENTOS.items()}


def build_peru_graph() -> Graph:
    edges = [(_NOMBRE_A_ID[a], _NOMBRE_A_ID[b]) for a, b in _FRONTERAS]
    return Graph.from_edge_list(list(DEPARTAMENTOS.keys()), edges, DEPARTAMENTOS)


if __name__ == "__main__":
    g = build_peru_graph()
    print(g)  # esperado: n=25
    print("grado maximo:", g.max_degree())