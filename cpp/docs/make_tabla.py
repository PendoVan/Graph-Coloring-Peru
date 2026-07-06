"""Convierte data/resultados.csv en docs/resultados_tabla.tex (tabla booktabs).

    python docs/make_tabla.py     (desde la raíz del repositorio)
"""
import csv
from pathlib import Path

RAIZ = Path(__file__).resolve().parent.parent
CSV = RAIZ / "data" / "resultados.csv"
SALIDA = RAIZ / "docs" / "resultados_tabla.tex"

NOMBRES = {"greedy_natural": "Greedy natural",
           "greedy_largest_first": "Greedy mayor grado",
           "greedy_smallest_last": "Greedy smallest-last",
           "dsatur": "DSATUR", "backtracking": "Backtracking"}


def fmt_t(seg: float) -> str:
    if seg < 1e-3:
        return f"{seg*1e6:.0f}\\,$\\mu$s"
    if seg < 1.0:
        return f"{seg*1e3:.2f}\\,ms"
    return f"{seg:.2f}\\,s"


def main() -> None:
    filas = list(csv.DictReader(open(CSV, encoding="utf-8")))
    out = [
        "% Generada por docs/make_tabla.py",
        "\\begin{table}[h]\\centering\\small",
        "\\caption{Resultados experimentales (media de repeticiones, semilla 42; "
        "backtracking con timeout de 30\\,s).}",
        "\\begin{tabular}{@{}llrrrrl@{}}",
        "\\toprule",
        "\\textbf{Instancia} & \\textbf{Algoritmo} & $n$ & $m$ & "
        "\\textbf{Colores} & \\textbf{Tiempo} & \\textbf{Mem.}\\\\",
        "\\midrule",
    ]
    prev = None
    for r in filas:
        if prev and r["instancia"] != prev:
            out.append("\\addlinespace")
        prev = r["instancia"]
        mem = f"{float(r['mem_mb']):.2f}\\,MB" if float(r["mem_mb"]) else "---"
        col = r["colores"] + (f" ({r['nota'].split(',')[0]})" if r["nota"] else "")
        out.append(f"{r['instancia'].replace('_','\\_')} & {NOMBRES[r['algoritmo']]} "
                   f"& {r['n']} & {r['m']} & {col} & {fmt_t(float(r['t_medio_s']))} "
                   f"& {mem}\\\\")
    out += ["\\bottomrule", "\\end{tabular}", "\\end{table}", ""]
    SALIDA.write_text("\n".join(out), encoding="utf-8")
    print(f"-> {SALIDA} ({len(filas)} filas)")


if __name__ == "__main__":
    main()
