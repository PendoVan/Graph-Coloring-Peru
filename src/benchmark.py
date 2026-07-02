"""Medicion de tiempo, calidad (colores) y memoria pico de cada algoritmo.

Metodologia: perf_counter con GC deshabilitado y varias repeticiones para el
tiempo; tracemalloc en una pasada aparte para la memoria (instrumentar el
asignador distorsiona los tiempos). La construccion del grafo no se cronometra.
"""
from __future__ import annotations

import gc
import statistics
import time
import tracemalloc
from dataclasses import dataclass

from . import generators
from .algorithms import (chromatic_number, dsatur, greedy, order_largest_first,
                         order_natural, order_smallest_last)
from .core import Graph, is_proper, num_colors
from .peru import build_peru_graph

GREEDY_ORDERS = {
    "greedy_natural": order_natural,
    "greedy_largest_first": order_largest_first,
    "greedy_smallest_last": order_smallest_last,
}


@dataclass
class Row:
    instance: str
    n: int
    m: int
    algorithm: str
    time_s: float
    time_sd: float
    colors: int
    valid: bool
    mem_mb: float
    note: str = ""


def _time(fn, reps: int):
    samples = []
    coloring: dict[int, int] = {}
    for _ in range(reps):
        gc.collect()
        gc.disable()
        t0 = time.perf_counter()
        coloring = fn()
        samples.append(time.perf_counter() - t0)
        gc.enable()
    sd = statistics.stdev(samples) if len(samples) > 1 else 0.0
    return statistics.fmean(samples), sd, coloring


def _mem(fn) -> float:
    gc.collect()
    tracemalloc.start()
    fn()
    peak = tracemalloc.get_traced_memory()[1]
    tracemalloc.stop()
    return peak / 2 ** 20


def bench_greedy_dsatur(instance: str, g: Graph, reps: int) -> list[Row]:
    rows = []
    jobs = {name: (lambda o=order: greedy(g, o(g))) for name, order in GREEDY_ORDERS.items()}
    jobs["dsatur"] = lambda: dsatur(g)
    for name, fn in jobs.items():
        mean, sd, col = _time(fn, reps)
        rows.append(Row(instance, g.n, g.m, name, mean, sd,
                        num_colors(col), is_proper(g, col)[0], _mem(fn)))
    return rows


def bench_backtracking(instance: str, g: Graph, timeout_s: float) -> Row:
    res = chromatic_number(g, max_n=max(g.n, 200), timeout_s=timeout_s)
    note = f"chi={res.k} exacto, {res.nodes} nodos" if res.exact \
        else f"chi en [{res.lower},{res.upper}] (timeout), {res.nodes} nodos"
    return Row(instance, g.n, g.m, "backtracking", res.elapsed, 0.0, res.k,
               is_proper(g, res.coloring)[0], 0.0, note)


def run(geo_sizes=(1000, 10000, 100000, 1000000), gnp_sizes=(1000,),
        reps=5, reps_big=3, bt_timeout=30.0, seed=42, on_progress=None) -> list[Row]:
    """Ejecuta la bateria completa y devuelve las filas. `on_progress(msg, frac)`
    permite a la GUI mostrar avance."""
    rows: list[Row] = []
    steps = 1 + len(gnp_sizes) + len(geo_sizes)
    done = 0

    def tick(msg):
        nonlocal done
        done += 1
        if on_progress:
            on_progress(msg, done / steps)

    peru = build_peru_graph()
    rows += bench_greedy_dsatur("Peru", peru, reps)
    rows.append(bench_backtracking("Peru", peru, bt_timeout))
    tick("Peru (n=25)")

    for n in gnp_sizes:
        g = generators.gnp(n, generators.p_for_avg_degree(n, 10.0), seed=seed)
        rows += bench_greedy_dsatur(f"gnp_{n}", g, reps)
        if n <= 2000:
            rows.append(bench_backtracking(f"gnp_{n}", g, bt_timeout))
        tick(f"G(n,p) n={n}")

    for n in geo_sizes:
        g = generators.geometric(n, avg_degree=6.0, seed=seed)
        rows += bench_greedy_dsatur(f"geo_{n}", g, reps if n < 100000 else reps_big)
        tick(f"geometrico n={n}")

    return rows


def to_csv(rows: list[Row], path) -> None:
    import csv
    from pathlib import Path
    Path(path).parent.mkdir(parents=True, exist_ok=True)
    with open(path, "w", newline="", encoding="utf-8") as f:
        w = csv.writer(f)
        w.writerow(["instancia", "n", "m", "algoritmo", "t_medio_s", "t_sd_s",
                    "colores", "valida", "mem_mb", "nota"])
        for r in rows:
            w.writerow([r.instance, r.n, r.m, r.algorithm, f"{r.time_s:.6g}",
                        f"{r.time_sd:.3g}", r.colors, int(r.valid),
                        f"{r.mem_mb:.3f}", r.note])


def main(argv=None) -> None:
    import argparse
    p = argparse.ArgumentParser(description="Benchmark de coloreo de grafos")
    p.add_argument("--geo-sizes", type=int, nargs="*", default=[1000, 10000, 100000, 1000000])
    p.add_argument("--gnp-sizes", type=int, nargs="*", default=[1000])
    p.add_argument("--reps", type=int, default=5)
    p.add_argument("--bt-timeout", type=float, default=30.0)
    p.add_argument("--seed", type=int, default=42)
    p.add_argument("--out", default="data/resultados.csv")
    a = p.parse_args(argv)
    rows = run(a.geo_sizes, a.gnp_sizes, reps=a.reps, bt_timeout=a.bt_timeout,
               seed=a.seed, on_progress=lambda msg, f: print(f"[{f*100:5.1f}%] {msg}"))
    for r in rows:
        print(f"  {r.instance:<10} {r.algorithm:<22} n={r.n:<8} colores={r.colors:<3} "
              f"t={r.time_s*1000:.3f}ms mem={r.mem_mb:.2f}MB {r.note}")
    to_csv(rows, a.out)
    print(f"CSV -> {a.out}")


if __name__ == "__main__":
    main()
