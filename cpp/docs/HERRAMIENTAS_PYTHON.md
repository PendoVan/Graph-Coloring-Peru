# Herramientas de Python usadas en el proyecto

Resumen de todo lo que se usó para construir el programa: lenguaje, módulos de la
biblioteca estándar (no requieren instalación), la única dependencia externa para
ejecutar, y las herramientas usadas **solo para preparar datos y el informe**.

---

## Lenguaje

- **Python 3.10+**. Se usan anotaciones de tipo modernas (`dict[int, set[int]]`,
  `X | None`) y `dataclasses`.

---

## Biblioteca estándar (incluida con Python, sin `pip`)

| Módulo | Dónde | Para qué |
|--------|-------|----------|
| `tkinter`, `tkinter.ttk` | `app.py` | Ventana, pestañas y **Canvas** donde se dibuja el mapa interactivo. |
| `json` | `peru.py` | Cargar `data/peru_map.json` (polígonos, fronteras, capitales, rutas). |
| `heapq` | `algorithms.py` | Heap binario de **DSATUR** (cola de prioridad por saturación). |
| `collections.deque` | `algorithms.py` | Cola del **BFS** (`order_distance`, `bfs_levels`). |
| `dataclasses` | `algorithms.py`, `benchmark.py` | `ExactResult` y `Row` (contenedores de resultados). |
| `functools.lru_cache` | `peru.py` | Cargar el mapa una sola vez. |
| `math` | varios | Proyección lon/lat, distancia de Haversine, radio del grafo geométrico. |
| `random` | `generators.py` | Grafos aleatorios G(n,p) y geométricos (con semilla). |
| `time.perf_counter` | varios | Medición de tiempo de alta resolución. |
| `tracemalloc` | `benchmark.py` | Medir la **memoria pico** de cada algoritmo. |
| `gc` | `benchmark.py` | Desactivar el recolector de basura durante cada medición. |
| `statistics` | `benchmark.py` | Media y desviación estándar de los tiempos. |
| `threading`, `queue` | `app.py` | Ejecutar benchmark/pruebas/demo **sin congelar** la interfaz. |
| `unittest` | `tests/`, `app.py` | Suite de pruebas; también se ejecuta **dentro** de la app. |
| `csv` | `benchmark.py` | Exportar `data/resultados.csv`. |
| `argparse` | `main.py`, `benchmark.py` | Opciones de línea de comandos (`--demo`, `--bench`). |
| `pathlib` | varios | Rutas de archivos **multiplataforma** (Windows/Linux/macOS). |
| `colorsys` | `app.py` | Paleta de gradiente verde→rojo del coloreo por distancia. |
| `contextlib.redirect_stdout` | `main.py` | Capturar la salida de la demo para mostrarla en la GUI. |
| `sys` | `algorithms.py`, `app.py` | Límite de recursión del backtracking; detección de plataforma. |
| `ctypes` | `app.py` | Memoria del proceso en **Windows** (API `psapi`) cuando no existe `resource`. |
| `resource` | `app.py` | Memoria del proceso en **Linux/macOS** (no existe en Windows; hay respaldo). |

---

## Dependencia externa para ejecutar (opcional)

| Paquete | Instalación | Para qué |
|---------|-------------|----------|
| **matplotlib** | `pip install -r requirements.txt` | Gráficas incrustadas de las pestañas *Benchmark* y *Recursos*. Sin él, la app funciona igual (mapa, pruebas y demo); esas dos pestañas muestran un aviso. |

---

## Herramientas usadas solo para preparar datos y el informe

No hacen falta para ejecutar el programa; se listan por transparencia
(reproducibilidad y uso responsable).

| Herramienta | Uso |
|-------------|-----|
| **scipy** (`scipy.sparse.csgraph.dijkstra`) | Rutear cada arista de interferencia sobre la red vial (Dijkstra multi-fuente en C) al construir `data/peru_map.json`. |
| **OpenStreetMap** vía **Overpass API** | Descargar la red vial nacional del Perú (vías *motorway/trunk/primary* con geometría). |
| **GeoJSON oficial de departamentos** (repositorio `juaneladio/peru-geojson`) | Polígonos de los 25 departamentos; de ahí se derivan centroides y las **53 fronteras** (adyacencia = polígonos que comparten frontera). |
| **pyflakes** | Verificar que no quedan imports ni variables sin uso (código limpio). |
| **TinyTeX / pdflatex** (o MiKTeX / Overleaf) | Compilar el informe `docs/analisis_complejidad.tex` a PDF. |
| **Douglas–Peucker** (implementado a mano) | Simplificar los polígonos y las rutas de carretera para un archivo de datos compacto. |

---

## Por qué (casi) todo es biblioteca estándar

El núcleo algorítmico, el mapa y las pruebas se apoyan **solo en lo que trae
Python**, para que el programa se ejecute en una máquina limpia (por ejemplo, una
laptop con Windows recién instalado) sin depender de paquetes pesados. Las
estructuras de grafo usan `dict`/`set` (tablas hash, operaciones O(1) promedio),
el heap de DSATUR usa `heapq`, y el mapa se dibuja con el `Canvas` de Tkinter.
Solo las **gráficas** del benchmark usan `matplotlib`, por eso es opcional.
