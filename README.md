# Zzen · Gratis

Aplicación de escritorio en C++ para repetir partidos del **modo Crónica** de Inazuma Eleven: Victory Road mediante Comandante y Revancha.

**[Descargar la versión 0.1 para Windows](https://github.com/kazah-png/Zzen/releases/tag/0.1)**

## Qué incluye

- Selección de categoría, rival, aparición y dificultad.
- Cantidad de partidos o repetición sin límite.
- Catálogo inicial de 131 entradas en ocho categorías, actualizable desde el juego.
- Inicio, pausa, reanudación y parada con **F8**.
- Resultados y contadores de la sesión actual.
- Diagnóstico con captura, recuadros de texto, última acción y registro.

Esta es la edición Gratis. No identifica objetos ni conserva un historial permanente de partidas o recompensas.

## Empezar

1. Descarga `Zzen-0.1-Windows-x64.zip` desde Releases y **extrae toda la carpeta**.
2. Abre `Zzen.exe`. No requiere instalación, cuenta ni clave de activación.
3. Abre una ruta de **Crónica** en el juego, con la interfaz en español y los controles de teclado/ratón.
4. Elige categoría, rival, dificultad y cantidad. **Aparición** distingue rivales repetidos dentro de una categoría.
5. Pulsa **Iniciar sesión**. Zzen busca al rival, activa Comandante y repite con Revancha.

El juego debe permanecer visible y con el foco. Cambiar de ventana pausa Zzen; **Reanudar** continúa. **F8** detiene la sesión. Solo una edición de Zzen puede estar abierta a la vez.

## Requisitos y alcance de 0.1

- Windows x64 con reconocimiento de texto de Windows disponible.
- Juego en español; referencia visual de 1920 × 1080 y relación 16:9.
- Las dificultades deben estar desbloqueadas en tu partida.
- El catálogo enumera entradas observadas en Crónica; no garantiza que todas estén desbloqueadas en otra partida.
- El reconocimiento puede necesitar ajustes si cambia la interfaz, el idioma o la disposición del juego. Consulta las [pruebas y límites](docs/VALIDATION.md).

Zzen utiliza capturas y entradas normales de Windows. No modifica archivos del juego, no inyecta código, no lee memoria del proceso ni altera el antitrampas. No es una herramienta oficial de LEVEL5.

## Si algo se detiene

Abre **Diagnóstico**. **Observar juego** analiza sin enviar entradas. **Guardar diagnóstico** conserva la captura y el registro localmente.

Un marcador ilegible queda **Pendiente** y detiene el avance. Revisa la evidencia antes de utilizar **Corregir resultado**. No se cuenta automáticamente como victoria. Los resultados de Gratis se reinician al iniciar otra sesión o cerrar la aplicación.

## Datos locales

Todo se guarda junto al ejecutable, sin cuentas ni telemetría:

| Carpeta | Contenido |
|---|---|
| `data` | Configuración, catálogo y registro de actividad |
| `debug` | Diagnósticos guardados expresamente |
| `captures` | Evidencia de resultados ilegibles |
| `evidence` | Capturas del barrido del catálogo |

Los JSON se reemplazan de forma atómica y mantienen copia `.bak`. Los archivos generados y los diagnósticos están excluidos del repositorio. Revisa las capturas antes de compartirlas en una incidencia.

## Compilar y probar

Necesitas Visual Studio Build Tools con **Desarrollo para el escritorio con C++**, Windows SDK con C++/WinRT y PowerShell.

```powershell
./build.ps1 -Test
```

Genera `dist/Zzen/Zzen.exe` y ejecuta pruebas de clasificación, almacenamiento y transiciones del motor. El motor de pruebas sustituye las entradas por eventos en memoria: **no controla el juego**.

También se incluye `CMakeLists.txt` para MSVC. Las pruebas visuales usan capturas locales que no se publican:

```powershell
./tests/visual.ps1 -Fixtures C:/ruta/a/capturas-de-prueba
```

## Estructura

`src/core.hpp`: clasificación y reglas; `vision.hpp`: captura y OCR; `zzen.cpp`: motor; `catalog_engine.inc`: navegación de Crónica; `storage.hpp`: configuración y catálogo; `ui.hpp`: interfaz nativa Win32.
