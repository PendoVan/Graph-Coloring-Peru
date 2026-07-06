# Especificación de Requisitos de Software

## 1. Requisitos Funcionales (RF)

### 1.1. CLI (`main.py`)
- **RF-01: Ejecución en modo Consola (Demo)**
  - **Descripción**: Muestra un reporte por consola sobre el coloreo del grafo de Perú con múltiples algoritmos.
  - **Inputs**: Parámetro `--demo`.
  - **Outputs**: Impresión en `stdout` con el reporte (tiempo de ejecución en milisegundos, validez del coloreo, y cotas halladas por backtracking). Muestra también la tabla de asignación óptima de frecuencias.
- **RF-02: Ejecución en modo Benchmark**
  - **Descripción**: Dispara la suite de benchmark por consola para múltiples grafos y algoritmos, reportando resultados y guardando en CSV.
  - **Inputs**: Parámetro `--bench` y opciones (`--geo-sizes`, `--gnp-sizes`, `--reps`, `--bt-timeout`, `--seed`, `--out`).
  - **Outputs**: Impresión de progreso y tabla por `stdout`. Archivo exportado `resultados.csv` (por defecto en `data/`).
- **RF-03: Lanzador de Interfaz Gráfica**
  - **Descripción**: Inicializa la aplicación Tkinter por defecto o explícitamente.
  - **Inputs**: Ninguno o parámetro `--gui`.

### 1.2. Pestaña Mapa Interactivo (`src/app.py` - `MapTab`)
- **RF-04: Coloreo y visualización de grafo de Perú**
  - **Descripción**: Permite renderizar los polígonos de los departamentos, visualizar el coloreo según diferentes algoritmos o la distancia desde un nodo base.
  - **Inputs**:
    - **Modo**: "Frecuencias (coloreo FAP)" o "Distancia (desde depto elegido)".
    - **Algoritmo**: "Greedy (natural)", "Greedy (mayor grado)", "Greedy (smallest-last)", "Greedy (por distancia)", "DSATUR", "Backtracking (exacto)".
    - **Departamento base**: Elegido por combobox o clic sobre polígono en el Canvas.
    - **Timeout BT**: Segundos de tiempo límite para backtracking (1-120).
    - **Mostrar carreteras / nombres**: Checkboxes para renderizar o no aristas y rótulos.
    - **Botones**: "Colorear", "Animar por distancia", "Reiniciar".
  - **Outputs**: 
    - Renderización en Canvas con proyección calculada al vuelo.
    - Tabla en el panel lateral (Treeview) detallando los departamentos asignados a cada frecuencia $f_c$.
    - Métricas en panel de Resultados (algoritmo usado, cantidad de colores, tiempo de ejecución, si el coloreo es propio).
  - **Reglas de negocio y validaciones**:
    - En modo Distancia (BFS), el color se grada en HSV de verde (distancia=0) a rojo (distancia=1 en normalizado al máximo nivel BFS).
    - El tiempo de la animación BFS avanza con `after(110)` (milisegundos) por oleada de vértices.
    - Los departamentos no coloreados o con valor $0$ usan color `#dddddd`. Las frecuencias $\ge 1$ reciclan colores de una paleta estática `PALETTE` usando la fórmula `(c - 1) % len(PALETTE)`.
  - **Manejo de errores**:
    - Si el Backtracking excede el timeout configurado, no arroja un crash; detiene la exploración y reporta "timeout" en la interfaz mostrando las cotas inferior/superior descubiertas.

### 1.3. Pestaña Benchmark (`src/app.py` - `BenchTab`)
- **RF-05: Benchmark Interactivo de Algoritmos**
  - **Descripción**: Ejecuta baterías de pruebas (grafos Perú, G(n,p), y geométricos) en segundo plano y muestra los resultados en tabla y gráfico.
  - **Inputs**: 
    - Selector "Tamaño": "Rápido (hasta 20 000)", "Medio (hasta 100 000)", "Completo (hasta 1 000 000)".
    - Botones "Ejecutar benchmark" y "Guardar CSV".
  - **Outputs**:
    - `Treeview` de resultados (instancia, n, m, algoritmo, colores, tiempo, memoria).
    - Gráfica Log-Log "Escalabilidad (grafos geométricos)" en Canvas Matplotlib.
  - **Flujo**:
    1. Bloqueo de botón "Ejecutar benchmark".
    2. Hilo secundario calcula métricas, enviando mensajes de progreso por una `Queue`.
    3. GUI recibe progreso en `after(100)` (polling). Actualiza barra de progreso y etiqueta de estado.
    4. Al finalizar, renderiza tabla, gráfica y desbloquea el botón.
  - **Manejo de errores**:
    - Excepciones en el subproceso se capturan y se muestran en la etiqueta de status.
    - "Guardar CSV" avisa si no se ha ejecutado el benchmark previamente.

### 1.4. Pestaña Recursos (`src/app.py` - `ResourceTab`)
- **RF-06: Monitoreo de Memoria y Tiempos**
  - **Descripción**: Mide el consumo pico de memoria y los tiempos de ejecución de los algoritmos usando subprocesos y medición del S.O.
  - **Inputs**: Botón "Medir recursos (Perú + geométrico 5 000)".
  - **Outputs**:
    - Etiqueta autoupdatable de la memoria física pico de la App completa (tick cada 1500 ms).
    - 2 subplots de barras comparativos (Tiempo vs Memoria pico) de todos los algoritmos implementados, aplicados en Perú y Geométrico n=5000.

### 1.5. Pestaña Pruebas (`src/app.py` - `TestTab`)
- **RF-07: Ejecución de Tests en UI**
  - **Descripción**: Ejecuta el módulo interno `unittest` sobre el paquete `tests/`.
  - **Inputs**: Botón "Ejecutar pruebas".
  - **Outputs**: Tabla con el resultado de cada test ejecutado (ícono, nombre, ms, detalle del error), y resumen estadístico.
  - **Flujo**: Se delega el proceso a un hilo que usa una clase hija de `unittest.TestResult` para comunicar los fallos y aciertos vía `Queue` a la UI.

### 1.6. Pestaña Demo (`src/app.py` - `DemoTab`)
- **RF-08: Log demostrativo en texto**
  - **Descripción**: Redirige y muestra la salida de la función demo del CLI a un control de texto.
  - **Inputs**: Botón "Ejecutar demo (Perú)".

### 1.7. Algoritmos (`src/algorithms.py`)
- **RF-09: Backtracking Exacto (`chromatic_number`)**
  - **Descripción**: Halla el número cromático exacto $\chi(G)$.
  - **Reglas / Validaciones**:
    - Pre-evalúa un clique máximo goloso (cota inferior) y el coloreo DSATUR (cota superior).
    - Falla con `ValueError` si $n > max\_n$ (default 200).
    - Aplica pre-coloreo del clique hallado, forward checking a nivel de bit (`dom = {v: full}`), y límite por tiempo de ejecución (`timeout_s`).
    - Usa `sys.setrecursionlimit` al vuelo dinámicamente $\max(sys.getrecursionlimit(), n+200)$.
- **RF-10: Algoritmos Voraces y Órdenes**
  - **Greedy**: Usa el primer color disponible buscando iterativamente enteros de 1 hacia arriba probando colisiones vecinales.
  - **DSATUR**: Prioriza el vértice de mayor saturación. Implementado usando un max-heap (tuplas negativas de tamaño de set de saturación y grado de nodo) con borrado perezoso para lograr $\mathcal{O}((n+m)\log n)$.
  - **Smallest-last**: Obtiene la subsecuencia por degenerescencia Matula-Beck, removiendo vértices de menor grado.

### 1.8. Modelado Principal y Grafo (`src/core.py`)
- **RF-11: Estructura de Grafo (`Graph`)**
  - **Descripción**: Implementación mediante diccionario de sets (`adj: dict[int, set[int]]`).
  - **Validaciones**: Se ignora la adición de auto-lazos (si `u == v` no se inserta).
- **RF-12: Verificación estricta de color (`is_proper`)**
  - **Descripción**: Valida si ningún par de adyacentes comparte el mismo valor numérico y el color no es 0 ni None.

### 1.9. Generadores Aleatorios (`src/generators.py`)
- **RF-13: Algoritmo Batagelj-Brandes (Gnp)**
  - **Fórmula de generación**: Usa saltos exponenciales para las aristas `w += 1 + int(math.log1p(-rng.random()) / log_q)`.
- **RF-14: Grafo Geométrico**
  - **Validación / Regla**: Genera puntos uniformes en $[0,1]^2$. Restringe las pruebas de distancia (para hallar conexiones) usando grillas cuadradas (spatial hash) con tamaño de celda igual al radio de conexión (radio = $\sqrt{avg\_degree / (\pi \cdot n)}$) para eficiencia.

### 1.10. Carga del Mapa del Perú (`src/peru.py`)
- **RF-15: Serialización Espacial**
  - **Descripción**: Parsea `data/peru_map.json` para extraer centroides, aristas de frontera y formas topológicas (anillos geográficos). Carga cacheada por `functools.lru_cache`.

---

## 2. Requisitos No Funcionales (RNF)
- **Rendimiento**:
  - `algorithms.py` evita escaneos de $\mathcal{O}(n^2)$ en implementaciones Greedy mediante bucket sorts para "Largest-First" y "Smallest-Last", logrando tiempo esperado $\Theta(n+m)$.
  - Hilos paralelos en la UI (`threading.Thread`) previenen que el Tkinter MainLoop se congele, usando polling manual con `queue.Queue` vía `after`.
  - La medición de memoria deshabilita dinámicamente el Recolector de Basura (`gc.disable()`) durante el muestreo temporal en benchmark, y delega a SO nativo en monitoreo de App.
- **Usabilidad**:
  - Respeto del layout mediante contenedores `pack` y `grid` que se adaptan, excepto el panel lateral derecho (resultados) que está congelado en 250 píxeles (`pack_propagate(False)`) para estabilidad visual.
  - La UI de la tabla "TestTab" soporta filas coloreadas con colores pastel semánticos (verde/rojo).
- **Compatibilidad**:
  - Sistema independiente respecto al núcleo. Para memoria pico (monitoreo en App), el programa implementa un dual stack: `resource.getrusage` para Linux/macOS y `psapi` vía llamadas por `ctypes` (API de Windows C) en Windows.
  - La visualización de gráficas de Benchmark y Recursos requiere explícitamente `matplotlib>=3.8` pero **no es requisito bloqueante** de la aplicación (captura de ImportError limpia que informa al usuario).
- **Mantenibilidad**:
  - Uso extendido de tipado estricto (Type Hints nativos de Python 3.9+) y `dataclasses` (`Row`, `ExactResult`).
  - Separación de frontend (MVC-like) en `app.py` e iteraciones pesadas contenidas netamente en `algorithms.py` y `generators.py`.
- **Confiabilidad**:
  - `chromatic_number` usa un Try/Except/Finally que garantiza que `sys.setrecursionlimit` volverá a su estado original independiente de errores.
  - "AMBIGÜEDAD": En la función temporal de GUI (Timeout), la ejecución finalizada repentinamente detiene la medición con un coloreo potencialmente inválido. Retorna el coloreo hallado más restrictivo pero válido solo según los topes pre-calculados (DSATUR superior).

---

## 3. Modelo de Datos / Estructuras Internas

### 3.1 Clases de Dominio
- **`Graph`** (`src/core.py`):
  - Atributos: `adj: dict[int, set[int]]`, `labels: dict[int, str]`.
  - Tipo Interno: Usa representación Lista de Adyacencia hash-table (Óptima en memoria para los grafos geométricos dispersos de m = O(n)).
- **`ExactResult`** (`src/algorithms.py`):
  - Atributos: `k: int`, `coloring: dict[int, int]`, `exact: bool`, `nodes: int`, `elapsed: float`, `lower: int`, `upper: int`.
- **`Row`** (`src/benchmark.py`):
  - Atributos: `instance: str`, `n: int`, `m: int`, `algorithm: str`, `time_s: float`, `time_sd: float`, `colors: int`, `valid: bool`, `mem_mb: float`, `note: str`.

### 3.2 Fórmulas Exactas y Algoritmos Implementados
- **Distancia Haversine** (`geo_distance`, `src/peru.py`):
  ```python
  p1, p2 = math.radians(lat1), math.radians(lat2)
  dp, dl = math.radians(lat2 - lat1), math.radians(lon2 - lon1)
  h = math.sin(dp / 2) ** 2 + math.cos(p1) * math.cos(p2) * math.sin(dl / 2) ** 2
  return 2 * 6371 * math.asin(math.sqrt(h))
  ```
- **Radio de Conexión Geométrico** (`src/generators.py`):
  ```python
  radius = math.sqrt(avg_degree / (math.pi * n))
  ```
- **Fórmula de Saltos de Batagelj-Brandes** (`src/generators.py`):
  ```python
  log_q = math.log1p(-p)
  # ... loop interior:
  w += 1 + int(math.log1p(-rng.random()) / log_q)
  ```
- **Proyección de Mapa en GUI** (`src/app.py` - `_transform`):
  - Escala `kx = math.cos(math.radians((lat0 + lat1) / 2))` para corrección esférica.
  - Escala base local (bounding box transform): `s = min((w - 2*margin)/((lon1-lon0)*kx), (h - 2*margin)/(lat1-lat0))`.

### 3.3 Formato de Archivos (E/S)
- **Entrada (`data/peru_map.json`)**: Archivo de texto JSON conteniendo las claves:
  - `"deptos"`: Mapa ID (string que parsea a entero) -> `"nombre"` (String), `"centroide"` ([lon, lat]), `"anillos"` (arreglo tridimensional de arreglos `[lon, lat]`).
  - `"aristas"`: Lista de sub-listas de 2 números indicando la frontera.
  - `"capitales"`: (Opcional) ID -> `"nombre"`, `"coord"`.
  - `"rutas"`: (Opcional) `"id1-id2"` -> Objeto con `"via"` (polilínea) y/o `"recta"` (boolean).
- **Salida (`data/resultados.csv`)**: CSV estructurado con cabecera estándar de `csv.writer`, usando el formato estricto: `instancia, n, m, algoritmo, t_medio_s, t_sd_s, colores, valida, mem_mb, nota`.

---

## 4. Dependencias Externas

### 4.1. Módulos Nativos (Librería Estándar de Python)
- `tkinter`, `ttk`: Para la creación íntegra de toda la interfaz gráfica de usuario y widgets.
- `threading`, `queue`: Para implementar comunicación IPC entre subprocesos paralelos y la GUI sin causar "No responde" del sistema operativo.
- `unittest`: Para la ejecución de las suites de validación (`tests/test_all.py`), interceptada al vuelo (`_GuiResult`).
- `json`, `math`, `random`: Parsers de mapas y modelado estocástico.
- `argparse`: Consumo de banderas CLI.
- `gc`, `tracemalloc`: Desactivación del Garbage Collector y profiling explícito de consumo de RAM.
- `ctypes`, `resource`: Wrappers OS-level calls para recuperar la "Working Set Size" de Memoria Física en Windows o la struct de consumo C de POSIX en Linux/Mac.

### 4.2. Módulos de Terceros (`requirements.txt`)
- **`matplotlib`**:
  - Función: Renderizado explícito de los gráficos (Log-Log de escalamiento O() de algoritmos de coloreo en `BenchTab` y los diagramas de barras de memoria/tiempo en `ResourceTab`). 
  - Versión Mínima: `>=3.8`.

---

## 5. Inventario de Componentes de UI

La ventana raíz `App` (del tipo `tk.Tk`) posee como widget primario un gestor de pestañas `ttk.Notebook` que distribuye 5 Frames.

### Pestaña 1: "Mapa del Perú" (`MapTab` / `ttk.Frame`)
- **Contenedor `left` (`ttk.Frame`)**: Layout dinámico `pack` apilado vertical.
  - `ttk.Label` de "Modo".
  - 2 x `ttk.Radiobutton` de modos (variable: `self.mode`).
  - `ttk.Separator`.
  - `ttk.Label` de "Algoritmo".
  - `ttk.Combobox` (ReadOnly) de Algoritmos (`self.algo`).
  - `ttk.Label` "Departamento base".
  - `ttk.Combobox` (ReadOnly) listado de capitales (`self.src_box`). Evento de selección (`<<ComboboxSelected>>`) asigna base de cálculo.
  - Fila inferior interna (`ttk.Frame`):
    - `ttk.Label` de timeout.
    - `ttk.Spinbox` (`self.timeout`) de tiempo límite Backtracking.
  - 2 x `ttk.Checkbutton`: variables de renderización `self.show_graph`, `self.show_labels` que re-ejecutan función de pintado.
  - 3 x `ttk.Button`: "▶ Colorear", "⏱ Animar por distancia", "↺ Reiniciar".
- **Contenedor `right` (`ttk.Frame`)**: Ancho fijo 250px.
  - `box` (`ttk.LabelFrame` "Resultado"): Matriz tabular `grid` de 5x2 `ttk.Label` con indicadores (Base, Algoritmo, Colores, Tiempo, Propio).
  - `listbox` (`ttk.LabelFrame` "Departamentos por frecuencia"): `ttk.Treeview` (`self.freq`, columnas "f", "deptos"), y su `ttk.Scrollbar` vertical atado al eje Y.
- **Centro/Restante (`tk.Canvas`)**:
  - `self.canvas`: Polígonos de mapa y aristas vectoriales. Reacciona al Resize general re-renderizando con evento `<Configure>`. Las formas generadas (tags `dep{id}`) tienen bindings a un evento `<Button-1>` para reasignar "Departamento Base".
- **Bottom**: `ttk.Label` en estado "Sunken" que oficia de barra de hints al pie.

### Pestaña 2: "Benchmark" (`BenchTab` / `ttk.Frame`)
- **Cabecera (`ttk.Frame` superior)**:
  - `ttk.Label` "Tamaño".
  - `ttk.Combobox` (`self.preset`) tamaños discretos.
  - Botón "Ejecutar benchmark" (Inhabilita la GUI y acciona un Thread).
  - Botón "Guardar CSV".
  - `ttk.Progressbar` (`self.prog`) en modo `determinate`.
  - Etiqueta texto `self.status`.
- **Cuerpo Central (`ttk.Frame`)**:
  - Lista de Resultados `self.table` (`ttk.Treeview` con 7 columnas: "instancia", "n", "m", "algoritmo", "colores", "tiempo", "mem").
  - Canvas inyectado (`FigureCanvasTkAgg` nativo de Matplotlib) que exhibe gráficos.

### Pestaña 3: "Recursos" (`ResourceTab` / `ttk.Frame`)
- **Contenedor superior (`ttk.Frame`)**:
  - Botón "Medir recursos".
  - `ttk.Label` autoupdatable que indica Memoria del Proceso (tick automático asíncrono).
- **Cuerpo inferior**:
  - Contiene netamente un `FigureCanvasTkAgg` de Matplotlib, renderizando dos axes (subplots 1x2).

### Pestaña 4: "Pruebas" (`TestTab` / `ttk.Frame`)
- **Cabecera (`ttk.Frame`)**:
  - Botón "Ejecutar pruebas".
  - `ttk.Label` "summary" que resume el conteo final.
- **Grilla de resultados (`self.table` / `ttk.Treeview`)**:
  - Columnas: "estado", "prueba", "ms", "detalle".
  - Tags de estilo condicional atados a la grilla: "ok" (Fondo `#e7f6e7`) y "bad" (Fondo `#fbe4e4`).

### Pestaña 5: "Demo" (`DemoTab` / `ttk.Frame`)
- **Cabecera**: Botón "Ejecutar demo (Perú)".
- **Contenedor Central**: Widget `tk.Text` con fuente Monospace que inserta los strings crudos procesados por la API `peru_demo_text()` vía Thread secundario.
