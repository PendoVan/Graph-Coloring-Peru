# Plan de Trabajo — Proyecto 9: Graph Coloring (FAP)
**Grupo 10 · Semanas 12 → 15 · UNMSM ADA**

Estado: 1er Informe (Sem. 12) ✅ completo. Falta: **código + GUI**, **casos de prueba**, **experimentos**, **informe final**, **video**, **sustentación**.

---

## 0. Mapeo Rúbrica → Estado

| # Entregable | Rúbrica | Estado | Fase |
|---|---|---|---|
| Modelado del problema | 3 | ✅ (Informe 1) | — |
| Código + GUI | 3 | ❌ | A, C |
| Correctitud algoritmos | 3 | ⚠️ pseudocódigo listo, falta impl. | A |
| Análisis de complejidad | 3 | ✅ (Informe 1) | — |
| Experimentos y escalabilidad | 2 | ❌ | B |
| Comparación crítica | 2 | ❌ | B, D |
| Sustentación individual | 3 | ❌ | E |
| Uso responsable de IA | 1 | ⚠️ redactado, mantener `log_IA.md` | transversal |

Decisión de dominio: **mapa del Perú (25 departamentos)** en lugar de Australia como caso pequeño verificable. Antenas→departamentos, interferencia→frontera compartida, frecuencia→color.

---

## FASE A — Núcleo algorítmico (Semana 12–13)

**Objetivo:** motor funcional y correcto, desacoplado de la GUI.

### A.1 Estructura de carpetas
```
proyecto9/
├── src/
│   ├── core/
│   │   ├── graph_model.py      # grafo CSR/listas adyacencia + wrapper NetworkX
│   │   └── validator.py        # verifica coloración propia φ(u)≠φ(v)
│   ├── algorithms/
│   │   ├── greedy.py           # órdenes: natural, Largest-First, Welsh-Powell
│   │   ├── dsatur.py           # heap binario (sat, grado)
│   │   └── backtracking.py     # exacto + forward checking + poda simetría
│   ├── generators/
│   │   ├── peru_map.py         # grafo real 25 deptos (caso 10 verificable)
│   │   └── random_graphs.py    # G(n,p) y geométricos → casos 10³, 10⁶
│   ├── benchmarking/
│   │   └── runner.py           # mide tiempo, colores, memoria (tracemalloc)
│   └── gui/
│       └── app.py              # Fase C
├── tests/
├── data/                       # resultados .csv, adyacencias Perú
└── main.py
```

### A.2 Tareas
- [ ] `graph_model.py`: representación **lista de adyacencia / CSR** (dispersa, Θ(n+m)). Métodos: `add_edge`, `neighbors(v)`, `degree(v)`, `n`, `m`. Constructor desde `networkx.Graph` y desde lista de aristas.
- [ ] `validator.py`: `is_proper(graph, color) -> bool` en O(n+m); reporta primera arista en conflicto.
- [ ] `greedy.py`: recibe `orden`; implementa las 3 estrategias de ordenamiento. O(n+m).
- [ ] `dsatur.py`: cola de prioridad (`heapq`) con clave `(-sat, -grado)`; actualización de saturación de vecinos. O((n+m)·log n).
- [ ] `backtracking.py`: `chromatic_number(G)` invocando k=1,2,3… con las 3 podas del informe. Poner **timeout/límite de n** para no colgar en instancias grandes.
- [ ] Correspondencia estricta con el pseudocódigo del Informe 1 (incluye el `color[v] ← 0` del backtrack ya corregido).

**Criterio de cierre A:** los 3 algoritmos sobre el grafo del Perú devuelven coloración propia y coinciden en χ(G) (probable **4**; verificar a mano). Registrar el valor real tras construir el grafo.

---

## FASE B — Datos experimentales y escalabilidad (Semana 13)

**Objetivo:** cubrir los 4 tamaños de la rúbrica y generar los CSV para el informe.

### B.1 Casos de prueba
| Tamaño | Fuente | Uso | Algoritmos |
|---|---|---|---|
| **10** (25 deptos) | `peru_map.py` (real) | verificable a mano | los 3 |
| **10³** | `random_graphs.py` G(n,p) | mediano | los 3 (BT con timeout) |
| **10⁶** | grafo disperso geométrico | grande | Greedy, DSATUR |
| **10¹⁰** | — | **solo teórico** | argumentación asintótica |

> Nota clave (ya documentada): 10¹⁰ es inviable físicamente (~20 GB solo el array de colores uint16). Se trata como **cota asintótica**, no experimento. Extrapolar curvas hasta 10⁶–10⁷ y cerrar con computación distribuida (Pregel/BSP).

### B.2 Tareas
- [ ] `runner.py`: por cada (algoritmo, instancia) medir **tiempo** (`perf_counter`), **nº de colores** (calidad), **memoria pico** (`tracemalloc`). Exportar `data/resultados.csv`.
- [ ] Repeticiones (≥5) y promedio para estabilidad temporal.
- [ ] Generar gráficas: tiempo vs n (log-log), colores vs algoritmo, memoria vs n. Guardar PNG para el informe.

**Criterio de cierre B:** `resultados.csv` + gráficas para 10, 10³, 10⁶.

---

## FASE C — Interfaz gráfica (Semana 13–14)

**Objetivo:** GUI que visualice el coloreo del mapa del Perú (referencia: imagen de Australia adjunta) y permita comparar algoritmos.

### C.1 Tareas (CustomTkinter + Matplotlib embebido, o PyQt6)
- [ ] Panel izquierdo: selección de instancia (Perú / aleatorio n), selección de algoritmo, botón *Colorear*.
- [ ] Panel central: dibujo del grafo del Perú con `networkx` + `matplotlib`, nodos coloreados según φ. Layout geográfico aproximado (coords de departamentos).
- [ ] Panel derecho: métricas en vivo (nº colores, tiempo, ¿coloración propia?).
- [ ] Modo comparación: ejecuta los 3 y muestra tabla lado a lado.
- [ ] (Opcional) vista dual mapa-real ↔ grafo, como en la imagen de referencia.

**Criterio de cierre C:** GUI ejecutable que colorea el Perú con los 3 algoritmos y muestra métricas.

---

## FASE D — Informe final (Semana 14–15)

**Objetivo:** extender el Informe 1 con las secciones faltantes de la rúbrica.

Secciones a **añadir** (el modelado, pseudocódigo y complejidad ya están):
- [ ] **7. Resultados experimentales**: tablas + gráficas de Fase B. Caso Perú resuelto (mostrar coloración y χ real).
- [ ] **8. Discusión de resultados**: Greedy vs DSATUR vs Backtracking en tiempo/calidad/memoria/escalabilidad. Costo de la optimalidad; error de las heurísticas frente a χ exacto; punto donde Backtracking deja de ser viable.
- [ ] **9. Conclusiones**: qué algoritmo para qué régimen; limitaciones; trabajo futuro (T-coloring, span, demanda múltiple, distribuido).
- [ ] Actualizar **Uso de IA** con errores nuevos detectados durante la codificación (desde `log_IA.md`).

**Criterio de cierre D:** informe final PDF completo con las 9 secciones.

---

## FASE E — Video y sustentación (Semana 15)

- [ ] Guion por integrante (4): reparto sugerido —
  - Badillo: modelado FAP → grafos + caso Perú.
  - Ccoicca: Greedy y DSATUR (pseudocódigo + complejidad).
  - Maque: Backtracking + NP-completitud + límite 10¹⁰.
  - Zárate: experimentos, GUI y comparación crítica.
- [ ] Video ≤20 min, rostros visibles, explicar con slides/pizarra (no leer).
- [ ] Preparar defensa individual: cada quien domina "su" algoritmo, su Big-O y los resultados.
- [ ] Q&A anticipado: ¿por qué 3 algoritmos? ¿por qué DSATUR intermedio? ¿por qué χ(Perú)=? ¿por qué 10¹⁰ solo teórico?

---

## Transversal — `log_IA.md`
Registrar desde ya cada corrección hecha a salidas de IA durante la codificación (para sustentación y sección 6). Ya documentados: falacia 10¹⁰, URL NetworkX rota, ambigüedad complejidad DSATUR, `color[v]←0` faltante, "no visitado" vs "no coloreado", derivación Greedy.

---

## Ruta crítica (orden de ejecución)
`graph_model.py` + `validator.py` → `peru_map.py` (define χ real) → 3 algoritmos → `runner.py` → generadores 10³/10⁶ → GUI → informe final → video.

**Siguiente acción concreta:** implementar `graph_model.py` y `validator.py`.
