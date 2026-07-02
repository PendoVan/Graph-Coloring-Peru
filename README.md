# Asignación de Frecuencias por Coloreo de Grafos — Mapa del Perú

**Proyecto 9 · Grupo 10 · UNMSM — Análisis y Diseño de Algoritmos**

Aplicación visual que resuelve el problema de asignación de frecuencias (FAP) sobre el **mapa real de los 25 departamentos del Perú**, modelado como coloreo de grafos: cada departamento es una antena, cada frontera compartida una interferencia, cada color una frecuencia. El objetivo es minimizar frecuencias, es decir, el número cromático **χ(G) = 4**.

Todo se ve en una sola ventana (`python main.py`): el mapa interactivo, el benchmark, el uso de recursos, la suite de pruebas y la demo.

---

## Ejecución

Requiere **Python 3.10+**. El núcleo, el mapa, las pruebas y la demo usan **solo la biblioteca estándar** (Tkinter). `matplotlib` es opcional y solo habilita las gráficas de las pestañas *Benchmark* y *Recursos*.

> **Windows:** ver la guía paso a paso en [`docs/INSTALACION_WINDOWS.md`](docs/INSTALACION_WINDOWS.md).
> **Herramientas y librerías de Python usadas:** [`docs/HERRAMIENTAS_PYTHON.md`](docs/HERRAMIENTAS_PYTHON.md).

```bash
pip install -r requirements.txt   # opcional (matplotlib para las gráficas)
python main.py                    # abre la aplicación visual
python main.py --demo             # demo por consola sobre el mapa del Perú
python main.py --bench            # benchmark por consola (ver --help)
python -m unittest discover -s tests -v   # 19 pruebas
```

---

## La aplicación (5 pestañas)

| Pestaña | Qué muestra |
|---------|-------------|
| **Mapa del Perú** | Mapa real e interactivo (polígonos de cada departamento). Se colorea con Greedy, DSATUR o Backtracking; se puede **hacer clic en un departamento** para elegirlo como base. Dos modos: *Frecuencias* (coloreo propio FAP) y *Distancia* (sombreado verde→rojo según la lejanía al departamento elegido). El grafo se dibuja **siguiendo las carreteras reales del Perú** (nodos en las capitales). Botón **Animar por distancia** que expande el color por oleadas desde la base, respetando el modo activo. Panel derecho de tamaño fijo con las métricas y los departamentos por frecuencia. |
| **Benchmark** | Ejecuta la batería completa (Perú, G(n,p), geométrico hasta 10⁶), muestra tabla y curva de escalabilidad log-log en vivo. Exporta `data/resultados.csv`. |
| **Recursos** | Tiempo y memoria pico por algoritmo (medidos con `tracemalloc`), más la memoria del proceso en vivo. |
| **Pruebas** | Corre la suite `unittest` dentro de la app y marca cada prueba en verde/rojo. |
| **Demo** | Resuelve el mapa del Perú con los tres algoritmos, muestra χ(G)=4 con su certificado (clique K₄) y la asignación óptima de frecuencias por departamento. |

### El coloreo por distancia

Al elegir un departamento base, la distancia a cada otro departamento se calcula con un **recorrido en anchura (BFS)** sobre el grafo de fronteras (sobre un mapa plano, la distancia en fronteras sigue a la lejanía geográfica). En el modo *Distancia* eso se pinta como gradiente (verde = cerca, rojo = lejos); en el modo *Frecuencias* ese mismo orden alimenta un coloreo goloso propio que se anima expandiéndose desde la base.

---

## Algoritmos

| Algoritmo | Tiempo | Garantía |
|-----------|--------|----------|
| **Greedy** (natural / mayor grado / smallest-last / por distancia) | Θ(n + m) | ≤ Δ+1 colores; smallest-last ≤ degenerescencia+1 |
| **DSATUR** (heap binario + borrado perezoso) | O((n + m) log n) | exacto en bipartitos; heurística en general |
| **Backtracking exacto** (cotas clique/DSATUR + pre-coloreo de clique + cota canónica + forward checking con bits) | O((Δ+χ)·χⁿ⁻ᑫ) | **χ(G) exacto** (con `max_n` y `timeout`) |

El análisis de complejidad **función por función** —con división en bloques, tablas costo×veces y sumatorias resueltas paso a paso— está en **[`docs/analisis_complejidad.pdf`](docs/analisis_complejidad.pdf)** (fuente `.tex` compilable con pdflatex u Overleaf).

---

## Estructura

```
main.py                    # punto de entrada: GUI (por defecto), --demo, --bench
src/
  core.py                  # Graph (dict de conjuntos) + is_proper + num_colors
  algorithms.py            # ordenamientos, greedy, dsatur, backtracking, distancia
  peru.py                  # carga data/peru_map.json; grafo, centroides, distancias
  generators.py            # G(n,p) (Batagelj–Brandes) y geométrico (rejilla)
  benchmark.py             # medición de tiempo/colores/memoria -> CSV
  app.py                   # aplicación Tkinter (5 pestañas)
tests/test_all.py          # 19 pruebas unitarias (solo stdlib)
data/peru_map.json         # 25 deptos: polígonos, centroides, 53 fronteras,
                           # capitales y rutas por carretera de cada arista
docs/
  analisis_complejidad.tex/.pdf   # análisis por función (bloques + sumatorias)
  INSTALACION_WINDOWS.md          # guía de instalación/uso en Windows
  HERRAMIENTAS_PYTHON.md          # librerías y herramientas de Python usadas
  figuras.py, make_tabla.py       # generan figuras y tabla del informe
```

El mapa, la adyacencia (53 fronteras) y los centroides se derivan de un **GeoJSON oficial**: dos departamentos son vecinos si sus polígonos comparten frontera. La adyacencia obtenida coincide con la verificación manual (Lima, Cusco y Huánuco con grado 7; Callao y Tumbes con grado 1). Para la visualización, cada arista se **rutea sobre la red vial nacional real** (OpenStreetMap): 49 de 53 aristas siguen carreteras; las 4 fronteras de Loreto no tienen conexión vial (Iquitos) y se dibujan como recta. El coloreo sigue usando la adyacencia por fronteras, así que χ = 4 no cambia.

---

## Resultados (semilla 42, `data/resultados.csv`)

| Instancia | n | m | Greedy (mayor grado) | DSATUR | Backtracking |
|---|---|---|---|---|---|
| Perú | 25 | 53 | 4 colores | 4 colores | **χ=4 exacto, 0 nodos** |
| G(n,p) | 10³ | 4995 | 7 colores | **5 colores** | χ∈[3,5], timeout 30 s |
| Geométrico | 10⁶ | 3×10⁶ | 13 col., 3.1 s | 13 col., 8.5 s, 442 MB | — (inviable) |

El backtracking es exacto e instantáneo en n=25 (las cotas cierran sin buscar) y se agota en n=10³; solo las heurísticas casi lineales escalan a 10⁶. El caso 10¹⁰ se trata solo asintóticamente (≈20 GB solo el arreglo de colores).

---

## Integrantes — Grupo 10

| Integrante | Área (Fase E) |
|------------|---------------|
| Badillo | Modelado FAP → grafos + caso Perú |
| Ccoicca | Greedy y DSATUR (pseudocódigo + complejidad) |
| Maque | Backtracking + NP-completitud + límite 10¹⁰ |
| Zárate | Experimentos, GUI y comparación crítica |

Uso responsable de IA documentado en [`log_IA.md`](log_IA.md).
