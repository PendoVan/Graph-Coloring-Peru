"""Coloreo de grafos para asignacion de frecuencias (FAP) — mapa del Peru.

    python main.py            abre la aplicacion visual (mapa, benchmark,
                              recursos, pruebas y demo en una sola ventana)
    python main.py --demo     ejecucion por consola sobre el mapa del Peru
    python main.py --bench    benchmark completo (ver --help de src.benchmark)
"""
import argparse
import io
import time
from contextlib import redirect_stdout

from src.algorithms import (chromatic_number, dsatur, greedy,
                            order_largest_first, order_smallest_last)
from src.core import is_proper, num_colors
from src.peru import build_peru_graph, frequency_table


def peru_demo_text() -> str:
    """Resuelve el mapa del Peru con los tres algoritmos y arma el reporte."""
    g = build_peru_graph()
    out = io.StringIO()
    with redirect_stdout(out):
        print(f"Mapa del Peru como grafo de interferencia (FAP): {g}")
        print("  nodo = departamento (antena) | arista = frontera (interferencia)")
        print("  color = frecuencia | objetivo: minimizar frecuencias = chi(G)\n")

        def linea(nombre, coloring, ms, extra=""):
            ok = is_proper(g, coloring)[0]
            print(f"  {nombre:<24} colores={num_colors(coloring)}  "
                  f"{'valida' if ok else 'INVALIDA':<8} {ms:7.3f} ms  {extra}")

        t = time.perf_counter()
        c1 = greedy(g, order_largest_first(g))
        linea("Greedy (mayor grado)", c1, (time.perf_counter() - t) * 1000)

        t = time.perf_counter()
        c2 = greedy(g, order_smallest_last(g))
        linea("Greedy (smallest-last)", c2, (time.perf_counter() - t) * 1000)

        t = time.perf_counter()
        c3 = dsatur(g)
        linea("DSATUR", c3, (time.perf_counter() - t) * 1000)

        res = chromatic_number(g, max_n=200, timeout_s=60.0)
        linea("Backtracking (exacto)", res.coloring, res.elapsed * 1000,
              f"chi(G)={res.k}, cotas=[{res.lower},{res.upper}], nodos={res.nodes}")

        print(f"\nchi(Peru) = {res.k}. Cota inferior: la clique K4 "
              "{Cusco, Apurimac, Ayacucho, Arequipa} obliga a 4 frecuencias.")
        print("\nAsignacion optima de frecuencias:")
        for c, deps in frequency_table(g, res.coloring).items():
            print(f"  f{c}: " + ", ".join(deps))
    return out.getvalue()


def main() -> None:
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--demo", action="store_true", help="ejecucion por consola")
    ap.add_argument("--gui", action="store_true", help="abrir la aplicacion (por defecto)")
    ap.add_argument("--bench", action="store_true", help="benchmark por consola")
    args, extra = ap.parse_known_args()

    if args.demo:
        print(peru_demo_text())
    elif args.bench:
        from src.benchmark import main as bench_main
        bench_main(extra)
    else:
        from src.app import launch
        launch()


if __name__ == "__main__":
    main()
