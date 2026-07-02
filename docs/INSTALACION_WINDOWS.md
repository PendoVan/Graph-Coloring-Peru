# Instalación y ejecución en Windows

Guía paso a paso para ejecutar el proyecto en **Windows 10 u 11**. La aplicación
(mapa interactivo, pruebas y demo) funciona **solo con Python**; `matplotlib` es
opcional y únicamente habilita las gráficas de las pestañas *Benchmark* y
*Recursos*.

---

## 1. Instalar Python

1. Descarga Python **3.10 o superior** desde el sitio oficial:
   <https://www.python.org/downloads/windows/>.
   > Usa el instalador de **python.org**, no la versión de Microsoft Store: el
   > instalador oficial incluye **Tcl/Tk (Tkinter)**, que es lo que dibuja la
   > interfaz y el mapa.
2. En la primera pantalla del instalador marca la casilla
   **“Add python.exe to PATH”** y pulsa *Install Now*.
3. Deja activada la opción **“tcl/tk and IDLE”** (viene marcada por defecto).

### Verificar la instalación

Abre **PowerShell** o **CMD** y ejecuta:

```powershell
python --version           # debe mostrar Python 3.10+ (o usa: py --version)
python -m tkinter          # debe abrir una pequeña ventana de prueba de Tk
```

Si `python` no se reconoce, usa el lanzador `py` (p. ej. `py main.py`) o
reinstala marcando “Add to PATH”.

---

## 2. Obtener el proyecto

- Con Git: `git clone <url-del-repositorio>` y entra a la carpeta, **o**
- Descarga el ZIP desde el repositorio y descomprímelo.

Abre una terminal **dentro de la carpeta del proyecto** (la que contiene
`main.py`). En el Explorador: clic en la barra de ruta, escribe `powershell` y
Enter.

---

## 3. (Recomendado) Entorno virtual e instalar matplotlib

```powershell
py -m venv .venv
.\.venv\Scripts\Activate.ps1     # en CMD:  .\.venv\Scripts\activate.bat
pip install -r requirements.txt  # instala matplotlib (opcional)
```

> Si PowerShell bloquea el script de activación, ejecútalo una vez:
> `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned` y responde *Sí*.

Puedes **saltarte** este paso: sin `matplotlib` la app abre igual y todo
funciona salvo las gráficas de *Benchmark* y *Recursos* (que muestran un aviso).

---

## 4. Ejecutar

```powershell
python main.py            # abre la aplicación visual (5 pestañas)
python main.py --demo     # ejecución por consola sobre el mapa del Perú
python main.py --bench    # benchmark por consola
python -m unittest discover -s tests -v   # suite de pruebas
```

En la pestaña **Mapa del Perú** puedes hacer clic en un departamento para
elegirlo como base, alternar entre *Frecuencias* y *Distancia*, mostrar el grafo
sobre las carreteras y usar **Animar por distancia**.

---

## 5. (Opcional) Compilar el informe LaTeX en Windows

- **En línea (más simple):** sube la carpeta `docs/` a <https://overleaf.com> y
  compila `analisis_complejidad.tex`.
- **Local:** instala **MiKTeX** (<https://miktex.org>) y ejecuta dos veces:
  ```powershell
  cd docs
  pdflatex analisis_complejidad.tex
  pdflatex analisis_complejidad.tex
  ```
- Para regenerar las figuras y la tabla (requiere matplotlib):
  ```powershell
  python docs\figuras.py
  python docs\make_tabla.py
  ```

---

## Solución de problemas

| Síntoma | Causa / solución |
|---|---|
| `No module named 'tkinter'` | Instalaste desde Microsoft Store o desmarcaste Tcl/Tk. Reinstala desde **python.org** con “tcl/tk and IDLE”. |
| `'python' no se reconoce…` | Falta el PATH. Usa `py` en vez de `python`, o reinstala marcando “Add to PATH”. |
| Las pestañas *Benchmark*/*Recursos* dicen “Instale matplotlib” | Ejecuta `pip install -r requirements.txt` (mejor dentro del entorno virtual). |
| La activación del `.venv` falla en PowerShell | `Set-ExecutionPolicy -Scope CurrentUser RemoteSigned`. |
| El benchmark completo (n=10⁶) tarda | Es normal (~1 min). Usa el preset **Rápido** en la pestaña, o `--geo-sizes` menores por consola. |
