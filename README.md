# Asignación de Frecuencias en Redes de Telecomunicaciones mediante Coloreo de Grafos

**Proyecto 9 — Grupo 10 · UNMSM ADA (Semanas 12–15)**

Implementación y comparativa de algoritmos de coloreo de grafos aplicados al problema de asignación de frecuencias (FAP). El caso de estudio central es el **mapa del Perú (25 departamentos)**: cada departamento modela una antena y cada frontera compartida representa interferencia potencial entre frecuencias.

---

## Estado de avance

| Fase | Descripción | Estado |
|------|-------------|--------|
| **A** | Núcleo algorítmico (modelos, algoritmos, generadores) | ✅ Completa |
| **B** | Datos experimentales y escalabilidad | 🔲 Pendiente |
| **C** | Interfaz gráfica (GUI) | 🔲 Pendiente |
| **D** | Informe final | 🔲 Pendiente |
| **E** | Video y sustentación | 🔲 Pendiente |

---

## Algoritmos implementados

| Algoritmo | Complejidad | Garantía de optimalidad |
|-----------|-------------|-------------------------|
| **Greedy** (natural / Largest-First / Welsh-Powell) | O(n + m) | ❌ Heurística |
| **DSATUR** (Degree of Saturation) | O((n + m) · log n) | ❌ Heurística |
| **Backtracking** (exacto + forward checking + poda simetría) | O(k^n) peor caso | ✅ Exacto |

---

## Estructura del proyecto

```
Graph-Coloring-Peru/
├── main.py                         # Punto de entrada: ejecuta los 3 algoritmos sobre el mapa del Perú
├── src/
│   ├── core/
│   │   ├── graph_model.py          # Representación por lista de adyacencia/CSR + wrapper NetworkX
│   │   └── validator.py            # Verifica coloración propia φ(u) ≠ φ(v) en O(n+m)
│   ├── algorithms/
│   │   ├── greedy.py               # 3 estrategias de ordenamiento
│   │   ├── dsatur.py               # Heap binario con clave (-sat, -grado)
│   │   └── backtracking.py         # Búsqueda exacta con timeout/límite de n
│   ├── generators/
│   │   ├── peru_map.py             # Grafo real de 25 departamentos (caso verificable)
│   │   └── random_graphs.py        # Grafos G(n,p) y geométricos → instancias 10³, 10⁶
│   ├── benchmarking/
│   │   └── runner.py               # Mide tiempo, nº de colores y memoria (tracemalloc)
│   └── gui/
│       └── app.py                  # Interfaz gráfica (Fase C — pendiente)
├── tests/                          # Pruebas unitarias e integración
├── data/                           # Resultados CSV y archivos de adyacencia del Perú
├── log_IA.md                       # Registro de uso responsable de IA
└── PlanDeTrabajo_Proyecto9_Grupo10.md
```

---

## Instalación

### Requisitos

- Python 3.8+
- pip

### Dependencias

```bash
pip install networkx matplotlib
```

> Las dependencias de GUI (CustomTkinter / PyQt6) se añadirán al completar la Fase C.

---

## Ejecución

### Demostración principal (mapa del Perú)

```bash
python main.py
```

Ejecuta los tres algoritmos sobre el grafo de los 25 departamentos del Perú, valida la coloración y muestra el número cromático obtenido por cada uno.

---

## Dominio del problema

El problema de asignación de frecuencias (FAP) se modela como coloreo de grafos:

- **Nodo** → departamento (antena)
- **Arista** → frontera compartida (interferencia)
- **Color** → frecuencia asignada

El objetivo es minimizar el número de colores (frecuencias) utilizados, es decir, calcular el **número cromático χ(G)**.

---

## Integrantes — Grupo 10

| Integrante | Área asignada (Fase E) |
|------------|------------------------|
| Badillo | Modelado FAP → grafos + caso Perú |
| Ccoicca | Greedy y DSATUR (pseudocódigo + complejidad) |
| Maque | Backtracking + NP-completitud + límite 10¹⁰ |
| Zárate | Experimentos, GUI y comparación crítica |

---

## Registro de IA (`log_IA.md`)

Se documenta el uso responsable de IA durante el desarrollo. Correcciones ya registradas: falacia 10¹⁰, URL de NetworkX rota, ambigüedad en complejidad de DSATUR, `color[v] ← 0` faltante, distinción "no visitado" vs. "no coloreado", derivación de Greedy.
