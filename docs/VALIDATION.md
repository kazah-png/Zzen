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

La comprobación de la sesión real se documentará al terminar la prueba de esta compilación.
