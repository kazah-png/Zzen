# Validación de Zzen 0.1

## Pruebas automáticas

- Clasificación de pantallas, victoria/derrota/empate y límites finitos e infinitos.
- Resultados solo en memoria; no se lee ni sobrescribe un historial ajeno.
- Persistencia de configuración, recuperación de copia de seguridad y límites de valores.
- Conservación de apariciones repetidas de un rival en el catálogo.
- Navegación hacia la lista y salida de la edición de alineación.
- Un único registro por partido aunque se lea varias veces el marcador.
- Transición a Revancha, espera del siguiente partido y parada al alcanzar el límite.
- Comandante ya activo: no se vuelve a alternar a manual.
- Observación sin entradas, rechazo de resultados anteriores y de entradas estando pausado.

## Reconocimiento visual local

Nueve capturas de regresión: formación, dos variantes del menú de dificultad, lista de equipos, marcadores 5–0 y 4–0, marcador con cursor, transición sin marcador legible y un duelo con «Derrota» que no debe contarse como final del partido.

Las capturas del juego no se incluyen en este repositorio. Las pruebas anteriores del motor no necesitan esas capturas ni que el juego esté abierto.

## Alcance

La referencia de reconocimiento es español, teclado/ratón y 1920 × 1080. No se ha verificado cada rival, dificultad, marcador posible, idioma o resolución. El modo infinito se verifica con pruebas del motor; no constituye una prueba de funcionamiento sin interrupciones durante días.

## Sesión real: 11 de octubre de 2026

Royal Academy, primera aparición de Inazuma Eleven, dificultad Heroica, límite de dos partidos. La sesión arrancó en Crónica, seleccionó el rival y la dificultad, salió de la edición de alineación, activó Comandante y completó dos victorias por 4–0. Entre ambas utilizó Revancha. Al terminar la segunda volvió al menú y se detuvo con 2/2 completados, 2 victorias, 0 derrotas y 0 pendientes.

La comprobación real duró aproximadamente once minutos. La compilación de distribución añade el icono y un ajuste del texto de selección de resultados; el comportamiento de observación sin entradas se verificó también en las pruebas automáticas. Las derrotas y empates se cubren en pruebas automáticas, no en esta sesión real.

La compilación y las pruebas automáticas también se ejecutaron correctamente en un equipo Windows de GitHub Actions.
