# Registro de uso responsable de IA — Proyecto 9 (Coloreo de Grafos)

Cada entrada documenta una salida de IA que fue **verificada y corregida**
por el grupo antes de incorporarse al proyecto, siguiendo la política del
curso. Formato: qué produjo la IA, qué estaba mal, cómo se detectó y qué
se hizo.

## Fase de informe (ya documentadas en el Informe 1)

1. **Falacia 10^10**: la IA proponía "ejecutar" el caso n=10^10; es
   físicamente inviable (~20 GB solo el arreglo de colores uint16, ~480 GB
   la adyacencia CSR). Se trata como cota asintótica + argumentación
   distribuida (BSP/Pregel). Detección: cálculo de memoria a mano.
2. **URL de NetworkX rota** en las referencias generadas.
3. **Ambigüedad en la complejidad de DSATUR**: la IA mezclaba O(n^2) y
   O((n+m) log n) sin decir cuál implementación; se fijó heap binario +
   borrado perezoso = O((n+m) log n).
4. **`color[v] <- 0` faltante** en el pseudocódigo de backtracking (sin
   deshacer, el backtracking no explora; detectado en revisión de escritorio).
5. **"No visitado" vs "no coloreado"**: la IA los usaba como sinónimos;
   en coloreo el estado relevante es "sin color" (0), no "visitado".
6. **Derivación de Greedy**: la sumatoria saltaba de sum(deg(v)) a n+m sin
   justificar el lema del apretón de manos; se corrigió la derivación.

## Fase A/B — codificación (Semanas 13–14)

7. **Fronteras del mapa del Perú con 5 errores (datos generados por IA)**.
   La lista inicial de 52 fronteras incluía dos adyacencias inexistentes
   — (Áncash, Pasco): entre ambos median Huánuco y Lima; y
   (San Martín, Ucayali): media Huánuco/Loreto — y omitía tres reales:
   (La Libertad, Huánuco), (Ucayali, Madre de Dios) y (Puno, Tacna).
   Detección: verificación departamento por departamento contra los
   límites oficiales (IGN). El grafo correcto tiene **25 nodos y 53
   aristas**; grados verificables: Lima=7, Cusco=7, Huánuco=7, Áncash=3.
   χ(G)=4 se mantiene: contiene el K4 {Cusco, Apurímac, Ayacucho,
   Arequipa}. Se agregaron tests (`tests/test_generators.py`) que fijan
   estas correcciones.
8. **El README afirmaba podas que el código no tenía**: "backtracking
   exacto + forward checking + poda simetría", pero la primera versión era
   backtracking puro (solo probaba colores 1..k). Se implementaron de
   verdad: pre-coloreo de clique (P3), cota canónica max_usado+1 (P4),
   forward checking con dominios de bits (P5), más cotas inferior (clique
   golosa) y superior (DSATUR). Con ellas, χ(Perú) se decide con 0 nodos
   de búsqueda y la 4-coloración se encuentra explorando 22 nodos (vs
   ~4^21 del backtracking ciego). Lección: verificar que la documentación
   describa el código real.
9. **`edges()` gastaba Θ(m) de memoria extra** en un conjunto `seen` para
   no duplicar aristas; basta emitir cuando u < v (mismo tiempo Θ(n+m),
   espacio O(1)).
10. **DSATUR recorría la vecindad dos veces**: reconstruía `usados` desde
    los vecinos cuando `sat_colors[v]` ya ES el conjunto de colores
    prohibidos. Además, re-insertar entradas obsoletas al hacer pop era
    superfluo: la entrada "fresca" de cada vértice no coloreado siempre
    está en el heap (lema demostrado en docs/analisis_complejidad.pdf).
11. **`add_edge(v, v)` perdía el vértice**: un lazo retornaba sin
    registrar v. Detectado por el test `test_arista_duplicada_y_lazo`;
    ahora el lazo registra el vértice y solo omite la arista.
12. **El timeout del backtracking se propagaba como excepción no
    capturada** hasta main.py. Ahora `chromatic_number` lo captura y
    devuelve `BTResult(exact=False)` con el intervalo demostrado
    [lb, ub] y la mejor coloración heurística conocida.

## Fase C/D — mapa real e interfaz visual

13. **Adyacencia derivada del mapa, no escrita a mano**. Se descargó un
    GeoJSON oficial de los 25 departamentos y se comprobó que preserva
    topología (los polígonos vecinos comparten vértices exactos). La
    adyacencia calculada geométricamente (vecinos = polígonos que comparten
    ≥2 vértices de frontera) dio **exactamente las 53 aristas** de la lista
    verificada a mano en la fase anterior: se eliminó la lista codificada y
    ahora grafo, centroides y polígonos salen de una sola fuente
    (`data/peru_map.json`). Los polígonos se simplificaron con
    Douglas-Peucker (3374 → 2668 puntos) cuidando de no colapsar los anillos
    cerrados (bug detectado: el DP directo sobre un anillo cerrado devolvía 2
    puntos; se corrigió partiendo el anillo en el vértice más lejano).
14. **Semántica del "coloreo por distancia"**. El pedido ("mientras más
    lejos un color, más cerca otro") se implementó de dos formas coherentes
    con el problema FAP: (a) un *sombreado* por distancia BFS desde el
    departamento elegido (verde=cerca, rojo=lejos), y (b) un *orden* de
    coloreo goloso sembrado por esa distancia, que anima el frente de color
    expandiéndose desde la base sin dejar de ser coloreo propio. Se documentó
    la distinción para no confundir "vista de distancia" con "coloreo FAP".
15. **Consolidación (clean code)**. Se aplanó `src/` de 6 subcarpetas a 6
    módulos, se quitaron funciones muertas (`to_networkx`, `from_networkx`,
    `count_conflicts`, alias redundantes) verificado con `pyflakes`, y se
    redujeron los comentarios a lo esencial. Los tres tabs con hilos usan el
    patrón cola+`after` en el hilo principal (un `after()` llamado desde un
    hilo trabajador lanzaba `RuntimeError: main thread is not in main loop`;
    se corrigió en la pestaña Demo).

## Fase D/E — carreteras reales, portabilidad y correcciones de UI

16. **Grafo ruteado por carreteras reales**. Se descargó la red vial nacional de
    OpenStreetMap (Overpass API; ~1.3 M de puntos) y, con Dijkstra multi-fuente
    (scipy) desde las 25 capitales, se calculó el camino por carretera de cada
    arista de interferencia. 49/53 aristas siguen carreteras; las 4 de Loreto se
    dibujan rectas porque Iquitos **realmente no tiene conexión vial** con el
    resto del país (dato correcto, no un bug). Los nodos del grafo se movieron de
    los centroides a las **capitales departamentales**. El modelo de coloreo NO
    cambió (sigue siendo adyacencia por fronteras, χ=4); las carreteras son solo
    la capa visual.
17. **Bug de "Animar por distancia"**: la animación forzaba
    `mode.set("Frecuencias")`, así que al animar desde el modo Distancia saltaba
    a Frecuencias. Corregido: la animación respeta el modo actual (revela
    frecuencias u oleadas de distancia según corresponda) y ya no cambia el
    selector.
18. **Panel derecho volátil**: las métricas estaban en un `Label` con
    `wraplength` que cambiaba de alto y reacomodaba toda la columna; además el
    hover actualizaba texto en cada movimiento del mouse. Se rehízo con un panel
    derecho de **ancho fijo** (`pack_propagate(False)`), etiquetas de resultado
    con `StringVar` y una lista de departamentos por frecuencia de alto fijo; se
    quitó el tooltip por hover (la info de la base aparece al hacer clic).
19. **Portabilidad Windows**: la memoria del proceso usaba `resource` (inexistente
    en Windows); se agregó respaldo con `ctypes`/`psapi` (`GetProcessMemoryInfo`)
    y detección de plataforma. Se documentó la instalación en Windows (Tkinter
    viene con el instalador de python.org, no con Microsoft Store).

## Verificación transversal

- 19 pruebas unitarias (`python -m unittest discover -s tests`) fijan grafos
  de χ conocido (K4, C5, C6, K3,3, Petersen, Perú), los grados del mapa, el
  K4 {Cusco, Apurímac, Ayacucho, Arequipa} y la equivalencia rejilla vs
  fuerza bruta del generador geométrico. Corren en la propia app (pestaña
  Pruebas) y por consola.
- Todos los números del documento LaTeX (nodos explorados, tiempos, memoria)
  provienen de ejecuciones reales reproducibles con semilla 42
  (`data/resultados.csv`); las figuras se generan con `docs/figuras.py`.
