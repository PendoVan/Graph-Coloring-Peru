"""Genera las figuras del informe LaTeX en docs/figs/.

    python docs/figuras.py     (desde la raíz; requiere matplotlib)

Produce: el mapa del Perú coloreado por frecuencias, el mapa sombreado por
distancia, y las curvas de escalabilidad a partir de data/resultados.csv.
"""
import csv
import sys
from pathlib import Path

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt
from matplotlib.collections import PatchCollection
from matplotlib.patches import Polygon as MPoly

RAIZ = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(RAIZ))
FIGS = RAIZ / "docs" / "figs"
FIGS.mkdir(parents=True, exist_ok=True)

from src.algorithms import bfs_levels, dsatur                       # noqa: E402
from src.app import dist_color, freq_color                          # noqa: E402
from src.peru import (build_peru_graph, capitals, centroids,        # noqa: E402
                      rings, routes)


def _draw(color_of, titulo, salida):
    rng = rings()
    cen = centroids()
    fig, ax = plt.subplots(figsize=(7, 9))
    pats, cols = [], []
    for dep, dep_rings in rng.items():
        for ring in dep_rings:
            pats.append(MPoly(ring, closed=True))
            cols.append(color_of(dep))
    ax.add_collection(PatchCollection(pats, facecolor=cols, edgecolor="#333", lw=0.6))
    g = build_peru_graph()
    for dep in g.vertices():
        ax.text(*cen[dep], g.label(dep)[:4], ha="center", va="center", fontsize=6)
    ax.autoscale()
    ax.set_aspect(1.0)
    ax.axis("off")
    ax.set_title(titulo)
    fig.savefig(salida, dpi=130, bbox_inches="tight")
    plt.close(fig)
    print("->", salida.name)


def mapas():
    g = build_peru_graph()
    col = dsatur(g)
    _draw(lambda d: freq_color(col[d]),
          "Mapa del Perú coloreado por frecuencias (DSATUR, χ=4)",
          FIGS / "peru_frecuencias.png")

    src = next(v for v in g.vertices() if g.label(v) == "Lima")
    lvl = bfs_levels(g, src)
    mx = max(lvl.values()) or 1
    _draw(lambda d: dist_color(lvl[d] / mx),
          "Coloreo por distancia desde Lima (verde=cerca, rojo=lejos)",
          FIGS / "peru_distancia.png")


def carreteras():
    """Grafo de interferencia dibujado siguiendo la red vial real (OSM)."""
    g = build_peru_graph()
    col = dsatur(g)
    rng, caps, rts = rings(), capitals(), routes()
    fig, ax = plt.subplots(figsize=(7, 9))
    pats = [MPoly(ring, closed=True) for d in rng.values() for ring in d]
    ax.add_collection(PatchCollection(
        pats, facecolor=[freq_color(col[d]) for d in rng for _ in rng[d]],
        edgecolor="#555", lw=0.5, alpha=0.55))
    for (a, b), r in rts.items():
        xs = [p[0] for p in r["via"]]
        ys = [p[1] for p in r["via"]]
        if r.get("recta"):
            ax.plot(xs, ys, color="#2980b9", lw=1, ls="--")
        else:
            ax.plot(xs, ys, color="#c0392b", lw=1.6, solid_capstyle="round")
    for _, (lon, lat) in caps.values():
        ax.plot(lon, lat, "o", color="#111", ms=3)
    ax.autoscale()
    ax.set_aspect(1.0)
    ax.axis("off")
    ax.set_title("Grafo de interferencia ruteado por carreteras reales (OSM)\n"
                 "rojo = carretera nacional · azul discontinuo = sin conexión vial")
    fig.savefig(FIGS / "peru_carreteras.png", dpi=130, bbox_inches="tight")
    plt.close(fig)
    print("-> peru_carreteras.png")


def escalabilidad():
    csv_path = RAIZ / "data" / "resultados.csv"
    if not csv_path.exists():
        print("data/resultados.csv no existe; ejecute el benchmark primero")
        return
    rows = list(csv.DictReader(open(csv_path, encoding="utf-8")))
    geo = [r for r in rows if r["instancia"].startswith("geo_")]
    algos = sorted({r["algoritmo"] for r in geo})

    fig, ax = plt.subplots(figsize=(7, 5))
    for a in algos:
        pts = sorted((int(r["n"]), float(r["t_medio_s"]))
                     for r in geo if r["algoritmo"] == a)
        if pts:
            xs, ys = zip(*pts)
            ax.loglog(xs, ys, "o-", label=a, markersize=4)
    if geo:
        xs = sorted({int(r["n"]) for r in geo})
        y0 = min(float(r["t_medio_s"]) for r in geo if int(r["n"]) == xs[0])
        ax.loglog(xs, [y0 * x / xs[0] for x in xs], "k--", alpha=0.5,
                  label="pendiente Θ(n)")
    ax.set_xlabel("n (vértices)")
    ax.set_ylabel("tiempo medio (s)")
    ax.set_title("Escalabilidad: tiempo vs n (grafos geométricos)")
    ax.grid(True, which="both", alpha=0.3)
    ax.legend(fontsize=8)
    fig.tight_layout()
    fig.savefig(FIGS / "tiempo_vs_n.png", dpi=140)
    plt.close(fig)
    print("-> tiempo_vs_n.png")

    fig, ax = plt.subplots(figsize=(7, 5))
    instancias = list(dict.fromkeys(r["instancia"] for r in rows))
    algos_all = list(dict.fromkeys(r["algoritmo"] for r in rows))
    w = 0.8 / len(algos_all)
    for j, a in enumerate(algos_all):
        xs, ys = [], []
        for i, inst in enumerate(instancias):
            for r in rows:
                if r["instancia"] == inst and r["algoritmo"] == a:
                    xs.append(i + j * w)
                    ys.append(int(r["colores"]))
        if xs:
            ax.bar(xs, ys, w, label=a)
    ax.set_xticks([i + 0.4 - w / 2 for i in range(len(instancias))])
    ax.set_xticklabels(instancias, rotation=20, fontsize=8)
    ax.set_ylabel("número de colores (menos = mejor)")
    ax.set_title("Calidad de la coloración por algoritmo")
    ax.grid(True, axis="y", alpha=0.3)
    ax.legend(fontsize=7)
    fig.tight_layout()
    fig.savefig(FIGS / "colores_por_algoritmo.png", dpi=140)
    plt.close(fig)
    print("-> colores_por_algoritmo.png")


if __name__ == "__main__":
    mapas()
    carreteras()
    escalabilidad()
