"""Aplicacion visual (Tkinter). Reune todo en una sola ventana con pestanas:
mapa interactivo del Peru, benchmark, uso de recursos, suite de pruebas y demo.

El mapa usa solo la biblioteca estandar; las graficas de benchmark/recursos
incrustan matplotlib si esta instalado (pip install matplotlib).
"""
from __future__ import annotations

import colorsys
import math
import queue
import sys
import threading
import time
import tkinter as tk
import unittest
from tkinter import ttk

from . import benchmark
from .algorithms import (bfs_levels, chromatic_number, dsatur, greedy,
                         order_distance, order_largest_first, order_natural,
                         order_smallest_last)
from .core import is_proper, num_colors
from .peru import (build_peru_graph, capitals, frequency_table, rings, routes)

PALETTE = ["#4e79a7", "#f28e2b", "#59a14f", "#e15759", "#b07aa1",
           "#edc948", "#76b7b2", "#ff9da7", "#9c755f", "#bab0ac"]

FREQ_ALGOS = {
    "Greedy (natural)": lambda g, s: greedy(g, order_natural(g)),
    "Greedy (mayor grado)": lambda g, s: greedy(g, order_largest_first(g)),
    "Greedy (smallest-last)": lambda g, s: greedy(g, order_smallest_last(g)),
    "Greedy (por distancia)": lambda g, s: greedy(g, order_distance(g, s)),
    "DSATUR": lambda g, s: dsatur(g),
    "Backtracking (exacto)": None,  # se maneja aparte por el timeout
}


def freq_color(c: int) -> str:
    return "#dddddd" if not c else PALETTE[(c - 1) % len(PALETTE)]


def dist_color(t: float) -> str:
    """Gradiente verde (cerca) -> rojo (lejos) para t en [0,1]."""
    r, g, b = colorsys.hsv_to_rgb((1 - t) * 0.33, 0.75, 0.95)
    return f"#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}"


# ==========================================================================
# Pestana 1: mapa interactivo del Peru
# ==========================================================================
class MapTab(ttk.Frame):
    def __init__(self, master, graph):
        super().__init__(master, padding=6)
        self.graph = graph
        self.rings = rings()
        self.caps = capitals()                 # id -> (nombre capital, (lon,lat))
        self.route_data = routes()             # (u,v) -> polilínea por carretera
        self.names = {v: graph.label(v) for v in graph.vertices()}
        self.id_of = {n: v for v, n in self.names.items()}
        self.source = self.id_of["Lima"]
        self.coloring: dict[int, int] = {}
        self.mode = tk.StringVar(value="Frecuencias (coloreo FAP)")
        self.algo = tk.StringVar(value="DSATUR")
        self.show_graph = tk.BooleanVar(value=True)
        self.show_labels = tk.BooleanVar(value=True)
        self._anim = None
        self._lvl = None                       # niveles BFS cacheados (modo distancia)
        self._build()
        self.after(120, self.draw)

    # ---- construccion de widgets ----
    def _build(self):
        left = ttk.Frame(self)
        left.pack(side=tk.LEFT, fill=tk.Y, padx=(0, 6))

        ttk.Label(left, text="Modo", font=("", 10, "bold")).pack(anchor="w")
        for m in ("Frecuencias (coloreo FAP)", "Distancia (desde depto elegido)"):
            ttk.Radiobutton(left, text=m, value=m, variable=self.mode,
                            command=self._mode_changed).pack(anchor="w")

        ttk.Separator(left).pack(fill=tk.X, pady=6)
        ttk.Label(left, text="Algoritmo (modo frecuencias)",
                  font=("", 9, "bold")).pack(anchor="w")
        ttk.Combobox(left, textvariable=self.algo, state="readonly", width=26,
                     values=list(FREQ_ALGOS)).pack(anchor="w", pady=2)

        ttk.Label(left, text="Departamento base:").pack(anchor="w", pady=(6, 0))
        self.src_box = ttk.Combobox(left, state="readonly", width=26,
                                    values=sorted(self.names.values()))
        self.src_box.set("Lima")
        self.src_box.pack(anchor="w", pady=2)
        self.src_box.bind("<<ComboboxSelected>>",
                          lambda e: self.set_source(self.id_of[self.src_box.get()]))

        fila = ttk.Frame(left)
        fila.pack(anchor="w", pady=(6, 0))
        ttk.Label(fila, text="timeout BT (s):").pack(side=tk.LEFT)
        self.timeout = tk.DoubleVar(value=10.0)
        ttk.Spinbox(fila, from_=1, to=120, width=6,
                    textvariable=self.timeout).pack(side=tk.LEFT, padx=4)

        ttk.Checkbutton(left, text="Mostrar carreteras (grafo)",
                        variable=self.show_graph, command=self.draw).pack(anchor="w", pady=(6, 0))
        ttk.Checkbutton(left, text="Mostrar nombres",
                        variable=self.show_labels, command=self.draw).pack(anchor="w")

        ttk.Button(left, text="▶ Colorear", command=self.colorear).pack(fill=tk.X, pady=(8, 2))
        ttk.Button(left, text="⏱ Animar por distancia", command=self.animar).pack(fill=tk.X, pady=2)
        ttk.Button(left, text="↺ Reiniciar", command=self.reiniciar).pack(fill=tk.X, pady=2)

        # ---- panel derecho de TAMAÑO FIJO (no se reajusta al cambiar valores) ----
        right = ttk.Frame(self, width=250)
        right.pack(side=tk.RIGHT, fill=tk.Y)
        right.pack_propagate(False)

        box = ttk.LabelFrame(right, text="Resultado")
        box.pack(fill=tk.X, padx=2, pady=2)
        self.r_vars = {k: tk.StringVar(value="—")
                       for k in ("Base", "Algoritmo", "Colores", "Tiempo", "Propio")}
        for i, (k, var) in enumerate(self.r_vars.items()):
            ttk.Label(box, text=k + ":", width=9, anchor="w").grid(row=i, column=0, sticky="w")
            ttk.Label(box, textvariable=var, width=20, anchor="w").grid(row=i, column=1, sticky="w")

        listbox = ttk.LabelFrame(right, text="Departamentos por frecuencia")
        listbox.pack(fill=tk.BOTH, expand=True, padx=2, pady=(6, 2))
        self.freq = ttk.Treeview(listbox, columns=("f", "deptos"), show="headings", height=16)
        self.freq.heading("f", text="frec.")
        self.freq.heading("deptos", text="departamentos")
        self.freq.column("f", width=42, anchor="center", stretch=False)
        self.freq.column("deptos", width=185, anchor="w")
        sb = ttk.Scrollbar(listbox, orient="vertical", command=self.freq.yview)
        self.freq.configure(yscrollcommand=sb.set)
        sb.pack(side=tk.RIGHT, fill=tk.Y)
        self.freq.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)

        self.canvas = tk.Canvas(self, bg="white", highlightthickness=0)
        self.canvas.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.canvas.bind("<Configure>", lambda e: self.draw())

        self.hint = ttk.Label(self, text="Clic en un departamento para elegirlo como base.",
                              relief=tk.SUNKEN, anchor="w")
        self.hint.pack(side=tk.BOTTOM, fill=tk.X)
        self.r_vars["Base"].set(self.names[self.source])

    # ---- proyeccion lon/lat -> pantalla ----
    def _transform(self, w, h):
        pts = [p for ring in (r for dep in self.rings.values() for r in dep) for p in ring]
        lons = [p[0] for p in pts]
        lats = [p[1] for p in pts]
        lon0, lon1 = min(lons), max(lons)
        lat0, lat1 = min(lats), max(lats)
        kx = math.cos(math.radians((lat0 + lat1) / 2))  # correccion de aspecto
        margin = 24
        s = min((w - 2 * margin) / ((lon1 - lon0) * kx), (h - 2 * margin) / (lat1 - lat0))
        ox = margin + (w - 2 * margin - (lon1 - lon0) * kx * s) / 2
        oy = margin + (h - 2 * margin - (lat1 - lat0) * s) / 2

        def tf(lon, lat):
            return ox + (lon - lon0) * kx * s, oy + (lat1 - lat) * s
        return tf

    # ---- dibujo ----
    def draw(self):
        cv = self.canvas
        cv.delete("all")
        w, h = max(cv.winfo_width(), 60), max(cv.winfo_height(), 60)
        self.tf = self._transform(w, h)
        self._lvl = bfs_levels(self.graph, self.source) \
            if self.mode.get().startswith("Distancia") else None
        self.poly_ids = {}
        for dep, dep_rings in self.rings.items():
            ids = []
            for ring in dep_rings:
                flat = [c for lon, lat in ring for c in self.tf(lon, lat)]
                ids.append(cv.create_polygon(flat, fill=self._fill(dep), outline="#555",
                                             width=1, tags=(f"dep{dep}", "depto")))
            self.poly_ids[dep] = ids
            cv.tag_bind(f"dep{dep}", "<Button-1>", lambda e, d=dep: self.set_source(d))
        if self.show_graph.get():
            self._draw_roads()
        if self.show_labels.get():
            for dep, (_, (lon, lat)) in self.caps.items():
                x, y = self.tf(lon, lat)
                cv.create_text(x, y - 7, text=self.names[dep][:4], font=("", 7))
        self._mark_source()
        self._legend()

    def _draw_roads(self):
        """Aristas del grafo dibujadas siguiendo la red vial real (rojo);
        discontinuo azul si no hay conexión vial (Loreto)."""
        cv = self.canvas
        for (a, b), r in self.route_data.items():
            flat = [c for lon, lat in r["via"] for c in self.tf(lon, lat)]
            if r.get("recta"):
                cv.create_line(flat, fill="#2980b9", width=1, dash=(4, 3))
            else:
                cv.create_line(flat, fill="#c0392b", width=2, capstyle=tk.ROUND)
        for dep, (_, (lon, lat)) in self.caps.items():
            x, y = self.tf(lon, lat)
            cv.create_oval(x - 3, y - 3, x + 3, y + 3, fill="#111", outline="white")

    def _fill(self, dep):
        if self._lvl is not None:
            mx = max(self._lvl.values()) or 1
            return dist_color(self._lvl[dep] / mx)
        return freq_color(self.coloring.get(dep, 0))

    def _mark_source(self):
        for pid in self.poly_ids.get(self.source, []):
            self.canvas.itemconfig(pid, outline="#111", width=3)

    def _legend(self):
        cv = self.canvas
        if self.mode.get().startswith("Distancia"):
            cv.create_text(30, 10, text="cerca", anchor="w", font=("", 8))
            for i in range(60):
                cv.create_rectangle(70 + i * 3, 4, 73 + i * 3, 16,
                                    fill=dist_color(i / 59), outline="")
            cv.create_text(70 + 60 * 3 + 6, 10, text="lejos", anchor="w", font=("", 8))
        else:
            k = num_colors(self.coloring)
            for c in range(1, min(k, 10) + 1):
                x = 10 + (c - 1) * 54
                cv.create_rectangle(x, 4, x + 14, 18, fill=freq_color(c), outline="#333")
                cv.create_text(x + 30, 11, text=f"f{c}", font=("", 8))

    # ---- interaccion ----
    def _mode_changed(self):
        self.draw()
        if self.mode.get().startswith("Distancia"):
            self._set_result(Algoritmo="Distancia (BFS)", Colores="—", Tiempo="—",
                             Propio="verde=cerca")

    def set_source(self, dep):
        self.source = dep
        self.src_box.set(self.names[dep])
        self.r_vars["Base"].set(self.names[dep])
        self.hint.config(text=f"Base: {self.names[dep]} — capital {self.caps[dep][0]}")
        self.draw()

    def reiniciar(self):
        if self._anim:
            self.after_cancel(self._anim)
            self._anim = None
        self.coloring = {}
        self._fill_table()
        for k in ("Algoritmo", "Colores", "Tiempo", "Propio"):
            self.r_vars[k].set("—")
        self.draw()

    def colorear(self):
        if self._anim:
            self.after_cancel(self._anim)
            self._anim = None
        if self.mode.get().startswith("Distancia"):
            self.draw()
            return
        name = self.algo.get()
        t0 = time.perf_counter()
        note = ""
        if name == "Backtracking (exacto)":
            res = chromatic_number(self.graph, max_n=300, timeout_s=self.timeout.get())
            self.coloring = res.coloring
            note = (f"χ={res.k} ({res.nodes} nodos)" if res.exact
                    else f"χ∈[{res.lower},{res.upper}] timeout")
        else:
            self.coloring = FREQ_ALGOS[name](self.graph, self.source)
        self._show_result(name, (time.perf_counter() - t0) * 1000, note)
        self.draw()

    def animar(self):
        """Revela los departamentos en oleadas por distancia (BFS) desde la base,
        respetando el modo actual (no cambia a frecuencias)."""
        if self._anim:
            self.after_cancel(self._anim)
            self._anim = None
        order = order_distance(self.graph, self.source)
        distancia = self.mode.get().startswith("Distancia")
        if distancia:
            lvl = bfs_levels(self.graph, self.source)
            mx = max(lvl.values()) or 1
            target = {d: dist_color(lvl[d] / mx) for d in order}
            self._set_result(Algoritmo="Animación distancia", Colores="—",
                             Tiempo="—", Propio="verde=cerca")
        else:
            self.coloring = greedy(self.graph, order_distance(self.graph, self.source))
            target = {d: freq_color(self.coloring[d]) for d in order}
            self._show_result("Greedy por distancia",
                              0.0, "coloreo en oleadas", fill_table=False)
        self.draw()
        for pid in (p for ids in self.poly_ids.values() for p in ids):
            self.canvas.itemconfig(pid, fill="#eeeeee")  # empezar en blanco
        self._mark_source()

        def step(i):
            if i >= len(order):
                self._anim = None
                if not distancia:
                    self._fill_table()
                return
            for pid in self.poly_ids.get(order[i], []):
                self.canvas.itemconfig(pid, fill=target[order[i]])
            self._anim = self.after(110, lambda: step(i + 1))
        step(0)

    # ---- panel derecho ----
    def _set_result(self, **kw):
        for k, v in kw.items():
            self.r_vars[k].set(v)

    def _show_result(self, name, ms, note, fill_table=True):
        ok = is_proper(self.graph, self.coloring)[0] if self.coloring else False
        self._set_result(Algoritmo=name, Colores=str(num_colors(self.coloring)),
                         Tiempo=f"{ms:.3f} ms", Propio=("sí" if ok else "—") +
                         (f" · {note}" if note else ""))
        if fill_table:
            self._fill_table()

    def _fill_table(self):
        for it in self.freq.get_children():
            self.freq.delete(it)
        if not self.coloring:
            return
        for c, deps in frequency_table(self.graph, self.coloring).items():
            self.freq.insert("", tk.END, values=(f"f{c}", ", ".join(deps)))


# ==========================================================================
# Utilidad: incrustar una figura matplotlib (o avisar si no esta)
# ==========================================================================
def embed_figure(parent):
    try:
        from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
        from matplotlib.figure import Figure
    except ImportError:
        ttk.Label(parent, text="Instale matplotlib para ver las gráficas:\n"
                              "pip install matplotlib", justify=tk.CENTER).pack(expand=True)
        return None, None
    fig = Figure(figsize=(6, 4.2), dpi=100)
    canvas = FigureCanvasTkAgg(fig, master=parent)
    canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)
    return fig, canvas


# ==========================================================================
# Pestana 2: benchmark
# ==========================================================================
class BenchTab(ttk.Frame):
    def __init__(self, master, app):
        super().__init__(master, padding=6)
        self.app = app
        self.q: queue.Queue = queue.Queue()
        self.preset = tk.StringVar(value="Rápido (hasta 20 000)")
        self._build()

    def _build(self):
        top = ttk.Frame(self)
        top.pack(fill=tk.X)
        ttk.Label(top, text="Tamaño:").pack(side=tk.LEFT)
        ttk.Combobox(top, textvariable=self.preset, state="readonly", width=24,
                     values=["Rápido (hasta 20 000)", "Medio (hasta 100 000)",
                             "Completo (hasta 1 000 000)"]).pack(side=tk.LEFT, padx=4)
        self.btn = ttk.Button(top, text="Ejecutar benchmark", command=self.run)
        self.btn.pack(side=tk.LEFT, padx=4)
        ttk.Button(top, text="Guardar CSV",
                   command=self.save_csv).pack(side=tk.LEFT)
        self.prog = ttk.Progressbar(top, length=220, mode="determinate")
        self.prog.pack(side=tk.LEFT, padx=8)
        self.status = ttk.Label(top, text="")
        self.status.pack(side=tk.LEFT)

        body = ttk.Frame(self)
        body.pack(fill=tk.BOTH, expand=True, pady=6)
        cols = ("instancia", "n", "m", "algoritmo", "colores", "tiempo", "mem")
        self.table = ttk.Treeview(body, columns=cols, show="headings", height=12)
        widths = (90, 80, 90, 150, 60, 90, 80)
        for c, w in zip(cols, widths):
            self.table.heading(c, text=c)
            self.table.column(c, width=w, anchor="center")
        self.table.pack(side=tk.LEFT, fill=tk.BOTH, expand=True)
        self.fig, self.mpl = embed_figure(body)

    def _sizes(self):
        p = self.preset.get()
        if p.startswith("Rápido"):
            return (500, 2000, 20000), (1000,), 3
        if p.startswith("Medio"):
            return (1000, 10000, 100000), (1000,), 3
        return (1000, 10000, 100000, 1000000), (1000,), 3

    def run(self):
        self.btn.config(state=tk.DISABLED)
        for it in self.table.get_children():
            self.table.delete(it)
        geo, gnp, reps = self._sizes()

        def work():
            try:
                rows = benchmark.run(geo_sizes=geo, gnp_sizes=gnp, reps=reps,
                                     bt_timeout=8.0,
                                     on_progress=lambda m, f: self.q.put(("p", (m, f))))
                self.q.put(("done", rows))
            except Exception as exc:  # noqa: BLE001
                self.q.put(("err", str(exc)))

        threading.Thread(target=work, daemon=True).start()
        self.after(100, self._poll)

    def _poll(self):
        try:
            kind, payload = self.q.get_nowait()
        except queue.Empty:
            self.after(100, self._poll)
            return
        if kind == "p":
            msg, frac = payload
            self.prog["value"] = frac * 100
            self.status.config(text=msg)
            self.after(100, self._poll)
        elif kind == "err":
            self.btn.config(state=tk.NORMAL)
            self.status.config(text=f"Error: {payload}")
        else:
            self.btn.config(state=tk.NORMAL)
            self.status.config(text="Listo.")
            self.app.bench_rows = payload
            self._fill(payload)

    def _fill(self, rows):
        for r in rows:
            t = f"{r.time_s*1000:.3f} ms" if r.time_s < 1 else f"{r.time_s:.3f} s"
            self.table.insert("", tk.END, values=(
                r.instance, r.n, r.m, r.algorithm, r.colors, t,
                f"{r.mem_mb:.2f} MB" if r.mem_mb else "—"))
        self._plot(rows)

    def _plot(self, rows):
        if not self.fig:
            return
        self.fig.clear()
        ax = self.fig.add_subplot(111)
        geo = [r for r in rows if r.instance.startswith("geo_")]
        algos = sorted({r.algorithm for r in geo})
        for a in algos:
            pts = sorted((r.n, r.time_s) for r in geo if r.algorithm == a)
            if pts:
                xs, ys = zip(*pts)
                ax.loglog(xs, ys, "o-", label=a, markersize=4)
        ax.set_xlabel("n (vértices)")
        ax.set_ylabel("tiempo (s)")
        ax.set_title("Escalabilidad (grafos geométricos)")
        ax.grid(True, which="both", alpha=0.3)
        ax.legend(fontsize=7)
        self.fig.tight_layout()
        self.mpl.draw()

    def save_csv(self):
        rows = getattr(self.app, "bench_rows", None)
        if not rows:
            self.status.config(text="Ejecute el benchmark primero.")
            return
        benchmark.to_csv(rows, "data/resultados.csv")
        self.status.config(text="Guardado en data/resultados.csv")


# ==========================================================================
# Pestana 3: recursos (memoria y tiempo)
# ==========================================================================
class ResourceTab(ttk.Frame):
    def __init__(self, master, app):
        super().__init__(master, padding=6)
        self.app = app
        top = ttk.Frame(self)
        top.pack(fill=tk.X)
        ttk.Button(top, text="Medir recursos (Perú + geométrico 5 000)",
                   command=self.measure).pack(side=tk.LEFT)
        self.info = ttk.Label(top, text="")
        self.info.pack(side=tk.LEFT, padx=10)
        self.fig, self.mpl = embed_figure(self)
        self.after(1500, self._tick)

    def _rss_mb(self):
        """Memoria pico del proceso, multiplataforma (Linux/macOS: resource;
        Windows: psapi vía ctypes)."""
        try:
            import resource
            rss = resource.getrusage(resource.RUSAGE_SELF).ru_maxrss
            return rss / 2 ** 20 if sys.platform == "darwin" else rss / 1024
        except ImportError:  # Windows
            try:
                import ctypes
                from ctypes import wintypes

                class _PMC(ctypes.Structure):
                    _fields_ = [("cb", wintypes.DWORD),
                                ("PageFaultCount", wintypes.DWORD),
                                ("PeakWorkingSetSize", ctypes.c_size_t),
                                ("WorkingSetSize", ctypes.c_size_t),
                                ("QuotaPeakPagedPoolUsage", ctypes.c_size_t),
                                ("QuotaPagedPoolUsage", ctypes.c_size_t),
                                ("QuotaPeakNonPagedPoolUsage", ctypes.c_size_t),
                                ("QuotaNonPagedPoolUsage", ctypes.c_size_t),
                                ("PagefileUsage", ctypes.c_size_t),
                                ("PeakPagefileUsage", ctypes.c_size_t)]
                c = _PMC()
                c.cb = ctypes.sizeof(c)
                ctypes.windll.psapi.GetProcessMemoryInfo(
                    ctypes.windll.kernel32.GetCurrentProcess(), ctypes.byref(c), c.cb)
                return c.PeakWorkingSetSize / 2 ** 20
            except Exception:
                return 0.0

    def _tick(self):
        self.info.config(text=f"Memoria del proceso (pico): {self._rss_mb():.1f} MB")
        self.after(1500, self._tick)

    def measure(self):
        from .generators import geometric
        graphs = {"Perú (n=25)": build_peru_graph(),
                  "geométrico (n=5000)": geometric(5000, seed=42)}
        jobs = {
            "greedy_natural": lambda g: greedy(g, order_natural(g)),
            "greedy_largest_first": lambda g: greedy(g, order_largest_first(g)),
            "greedy_smallest_last": lambda g: greedy(g, order_smallest_last(g)),
            "dsatur": dsatur,
        }
        data = {}
        for gname, g in graphs.items():
            data[gname] = {}
            for aname, fn in jobs.items():
                t, _, _ = benchmark._time(lambda: fn(g), 3)
                mem = benchmark._mem(lambda: fn(g))
                data[gname][aname] = (t * 1000, mem)
        self._plot(data)

    def _plot(self, data):
        if not self.fig:
            return
        self.fig.clear()
        gnames = list(data)
        anames = list(next(iter(data.values())))
        ax1 = self.fig.add_subplot(121)
        ax2 = self.fig.add_subplot(122)
        width = 0.8 / len(gnames)
        for j, gname in enumerate(gnames):
            xs = [i + j * width for i in range(len(anames))]
            ax1.bar(xs, [data[gname][a][0] for a in anames], width, label=gname)
            ax2.bar(xs, [data[gname][a][1] for a in anames], width, label=gname)
        for ax, title, ylabel in ((ax1, "Tiempo", "ms"), (ax2, "Memoria pico", "MB")):
            ax.set_xticks([i + 0.4 - width / 2 for i in range(len(anames))])
            ax.set_xticklabels([a.replace("greedy_", "g.") for a in anames],
                               rotation=30, ha="right", fontsize=7)
            ax.set_title(title)
            ax.set_ylabel(ylabel)
            ax.legend(fontsize=6)
            ax.grid(True, axis="y", alpha=0.3)
        self.fig.tight_layout()
        self.mpl.draw()


# ==========================================================================
# Pestana 4: suite de pruebas
# ==========================================================================
class _GuiResult(unittest.TestResult):
    def __init__(self, sink):
        super().__init__()
        self.sink = sink
        self._t0 = 0.0

    def startTest(self, test):
        super().startTest(test)
        self._t0 = time.perf_counter()

    def addSuccess(self, test):
        super().addSuccess(test)
        self.sink(("ok", str(test), time.perf_counter() - self._t0, ""))

    def addFailure(self, test, err):
        super().addFailure(test, err)
        self.sink(("fail", str(test), time.perf_counter() - self._t0,
                   self._exc_msg(err)))

    def addError(self, test, err):
        super().addError(test, err)
        self.sink(("error", str(test), time.perf_counter() - self._t0,
                   self._exc_msg(err)))

    @staticmethod
    def _exc_msg(err):
        return str(err[1]).splitlines()[0] if err[1] else ""


class TestTab(ttk.Frame):
    def __init__(self, master, app):
        super().__init__(master, padding=6)
        self.q: queue.Queue = queue.Queue()
        top = ttk.Frame(self)
        top.pack(fill=tk.X)
        self.btn = ttk.Button(top, text="Ejecutar pruebas", command=self.run)
        self.btn.pack(side=tk.LEFT)
        self.summary = ttk.Label(top, text="")
        self.summary.pack(side=tk.LEFT, padx=10)
        cols = ("estado", "prueba", "ms", "detalle")
        self.table = ttk.Treeview(self, columns=cols, show="headings")
        for c, w in zip(cols, (70, 320, 60, 320)):
            self.table.heading(c, text=c)
            self.table.column(c, width=w, anchor="w")
        self.table.tag_configure("ok", background="#e7f6e7")
        self.table.tag_configure("bad", background="#fbe4e4")
        self.table.pack(fill=tk.BOTH, expand=True, pady=6)

    def run(self):
        self.btn.config(state=tk.DISABLED)
        for it in self.table.get_children():
            self.table.delete(it)
        self.summary.config(text="Ejecutando…")

        def work():
            suite = unittest.TestLoader().discover("tests")
            res = _GuiResult(lambda ev: self.q.put(ev))
            t0 = time.perf_counter()
            suite.run(res)
            self.q.put(("summary", res.testsRun, len(res.failures),
                        len(res.errors), time.perf_counter() - t0))

        threading.Thread(target=work, daemon=True).start()
        self.after(80, self._poll)

    def _poll(self):
        try:
            ev = self.q.get_nowait()
        except queue.Empty:
            self.after(80, self._poll)
            return
        if ev[0] == "summary":
            _, run, fails, errs, dt = ev
            ok = run - fails - errs
            self.summary.config(text=f"{ok}/{run} pasaron · {fails} fallos · "
                                     f"{errs} errores · {dt:.2f}s")
            self.btn.config(state=tk.NORMAL)
            return
        status, name, dt, detail = ev
        icon = {"ok": "✔", "fail": "✘", "error": "✘"}.get(status, "?")
        tag = "ok" if status == "ok" else "bad"
        self.table.insert("", tk.END, tags=(tag,),
                          values=(icon, name, f"{dt*1000:.1f}", detail))
        self.after(80, self._poll)


# ==========================================================================
# Pestana 5: demo (mapa del Peru resuelto)
# ==========================================================================
class DemoTab(ttk.Frame):
    def __init__(self, master, app):
        super().__init__(master, padding=6)
        self.q: queue.Queue = queue.Queue()
        top = ttk.Frame(self)
        top.pack(fill=tk.X)
        ttk.Button(top, text="Ejecutar demo (Perú)", command=self.run).pack(side=tk.LEFT)
        self.text = tk.Text(self, wrap="word", font=("monospace", 10))
        self.text.pack(fill=tk.BOTH, expand=True, pady=6)

    def run(self):
        from main import peru_demo_text
        self.text.delete("1.0", tk.END)
        self.text.insert(tk.END, "Ejecutando…\n")
        threading.Thread(target=lambda: self.q.put(peru_demo_text()),
                         daemon=True).start()
        self.after(80, self._poll)

    def _poll(self):
        try:
            txt = self.q.get_nowait()
        except queue.Empty:
            self.after(80, self._poll)
            return
        self.text.delete("1.0", tk.END)
        self.text.insert(tk.END, txt)


# ==========================================================================
class App(tk.Tk):
    def __init__(self):
        super().__init__()
        self.title("Coloreo de Grafos — Asignación de Frecuencias (FAP) · Mapa del Perú · Grupo 10")
        self.geometry("1260x800")
        self.minsize(1040, 640)
        self.bench_rows = []
        graph = build_peru_graph()
        nb = ttk.Notebook(self)
        nb.pack(fill=tk.BOTH, expand=True)
        nb.add(MapTab(nb, graph), text="Mapa del Perú")
        nb.add(BenchTab(nb, self), text="Benchmark")
        nb.add(ResourceTab(nb, self), text="Recursos")
        nb.add(TestTab(nb, self), text="Pruebas")
        nb.add(DemoTab(nb, self), text="Demo")


def launch():
    App().mainloop()


if __name__ == "__main__":
    launch()
