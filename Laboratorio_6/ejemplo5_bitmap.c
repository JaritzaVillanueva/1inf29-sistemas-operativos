/* ejemplo5_bitmap.c
 * Reconstruye un mapa de bits de bloques ocupados
 * a partir de los bloques que reclaman los inodos.
 *
 * También detecta un cross-link:
 * un mismo bloque reclamado por dos inodos distintos.
 *
 * Compilar:
 * gcc -Wall -Wextra ejemplo5_bitmap.c -o ejemplo5
 *
 * Ejecutar:
 * ./ejemplo5
 */

#include <stdio.h>      // Permite usar printf().
#include <stdlib.h>     // Incluye funciones generales de la biblioteca estándar.
#include <stdint.h>     // Permite usar tipos enteros de tamaño fijo como uint8_t, uint16_t y uint32_t.
#include <string.h>     // Permite usar memset() y memcpy().

#define BLOQUE 64       // Tamaño de cada bloque en bytes.
#define TOTAL_BLOQUES 8 // Cantidad total de bloques del disco simulado.
#define PUNTEROS 4      // Cantidad de punteros directos por inodo.

/*
 * Cabecera del "disco".
 * En este ejemplo, vive en el bloque 0.
 */
typedef struct {
    uint32_t magic;       // Número mágico para validar el formato.
    uint16_t bitmap_ini;  // Número de bloque donde empieza el bitmap.
} Cabecera;

/*
 * Inodo simplificado.
 * Representa un archivo dentro del sistema de archivos simulado.
 */
typedef struct {
    uint16_t en_uso;              // Vale 1 si el inodo está activo; 0 si está libre.
    uint16_t enlaces;             // Cantidad de nombres que apuntan a este inodo.
    uint32_t directos[PUNTEROS];  // Punteros directos a bloques de datos.
} Inodo;

int main(void) {

    /*
     * Creamos un arreglo de bytes que simula un disco.
     * El disco tiene TOTAL_BLOQUES bloques,
     * y cada bloque mide BLOQUE bytes.
     */
    uint8_t disco[TOTAL_BLOQUES * BLOQUE];

    /*
     * Inicializamos todo el disco con ceros.
     * Esto representa un disco vacío al inicio.
     */
    memset(disco, 0, sizeof disco);

    /*
     * Creamos la cabecera del disco.
     * magic = 0xCAFE sirve para validar el formato.
     * bitmap_ini = 1 significa que el bitmap está en el bloque 1.
     */
    Cabecera cab = { 0xCAFE, 1 };

    /*
     * Copiamos la cabecera al bloque 0 del disco.
     * disco + 0 * BLOQUE apunta al inicio del bloque 0.
     */
    memcpy(disco + 0 * BLOQUE, &cab, sizeof cab);

    /*
     * El bloque 1 será el bitmap almacenado.
     * Como el disco fue inicializado con ceros,
     * el bitmap dice falsamente que todos los bloques están libres.
     *
     * Este es el "sabotaje" del ejemplo.
     */

    /*
     * Creamos un inodo legítimo.
     * Está en uso, tiene 1 enlace y reclama los bloques 3 y 4.
     */
    Inodo legitimo = { 1, 1, { 3, 4, 0, 0 } };

    /*
     * Creamos un inodo huérfano.
     * Está en uso, pero tiene 0 enlaces.
     * Además, reclama el bloque 3, que ya era usado por el inodo legítimo.
     */
    Inodo huerfano = { 1, 0, { 3, 0, 0, 0 } };

    /*
     * Guardamos los inodos en un arreglo para poder recorrerlos.
     */
    Inodo inodos[] = { legitimo, huerfano };

    /*
     * Cantidad de inodos del arreglo.
     */
    int n_inodos = 2;

    /*
     * Obtenemos un puntero al bitmap almacenado en el disco.
     * Como cab.bitmap_ini vale 1, esto apunta al bloque 1.
     */
    uint8_t *bm_almacenado = disco + cab.bitmap_ini * BLOQUE;

    /*
     * Mostramos el bitmap almacenado.
     * Como está lleno de ceros, afirma que todos los bloques están libres.
     */
    printf("Bitmap almacenado en el bloque %d (afirma 'todo libre'):\n ",
           cab.bitmap_ini);

    /*
     * Imprimimos los primeros TOTAL_BLOQUES valores del bitmap.
     */
    for (int i = 0; i < TOTAL_BLOQUES; i++) {
        printf("%d", bm_almacenado[i]);
    }

    printf("\n");

    /*
     * ---------------------------------------------------------
     * Reconstrucción del bitmap desde cero.
     * ---------------------------------------------------------
     */

    /*
     * Este bitmap reconstruido empieza vacío.
     * 0 significa bloque libre.
     * 1 significa bloque ocupado.
     */
    uint8_t bitmap[TOTAL_BLOQUES] = { 0 };

    /*
     * El arreglo dueno indica qué inodo reclamó primero cada bloque.
     * Si dueno[b] = -1, significa que ningún inodo ha reclamado el bloque b.
     */
    int dueno[TOTAL_BLOQUES];

    /*
     * Inicializamos todos los dueños en -1.
     */
    for (int i = 0; i < TOTAL_BLOQUES; i++) {
        dueno[i] = -1;
    }

    /*
     * Los bloques de metadatos siempre están ocupados.
     * El bloque 0 contiene la cabecera.
     */
    bitmap[0] = 1;

    /*
     * El bloque cab.bitmap_ini contiene el bitmap almacenado.
     * En este ejemplo, es el bloque 1.
     */
    bitmap[cab.bitmap_ini] = 1;

    /*
     * Recorremos todos los inodos.
     */
    for (int k = 0; k < n_inodos; k++) {

        /*
         * Si el inodo no está en uso, lo ignoramos.
         */
        if (!inodos[k].en_uso) {
            continue;
        }

        /*
         * Recorremos los punteros directos del inodo actual.
         */
        for (int d = 0; d < PUNTEROS; d++) {

            /*
             * Obtenemos el número de bloque apuntado por el puntero directo d.
             */
            int b = (int)inodos[k].directos[d];

            /*
             * En este ejemplo, el valor 0 significa "sin bloque asignado".
             * Por eso, si encontramos 0, dejamos de revisar más punteros.
             */
            if (b == 0) {
                break;
            }

            /*
             * Si el bloque ya estaba marcado como ocupado
             * y además ya tenía un dueño registrado,
             * significa que otro inodo ya lo había reclamado.
             *
             * Eso es un cross-link.
             */
            if (bitmap[b] && dueno[b] != -1) {

                /*
                 * a es el inodo que reclamó primero el bloque.
                 */
                int a = dueno[b];

                /*
                 * Decidimos cuál de los dos es huérfano.
                 * El criterio usado es el contador de enlaces.
                 * Si el inodo a tiene enlaces 0, a es huérfano.
                 * En caso contrario, k es el huérfano.
                 */
                int h = (inodos[a].enlaces == 0) ? a : k;

                /*
                 * El legítimo es el otro inodo.
                 */
                int l = (h == a) ? k : a;

                /*
                 * Reportamos el cross-link.
                 */
                printf("CROSS-LINK: el bloque %d lo reclaman el inodo %d y el inodo %d.\n",
                       b, a, k);

                /*
                 * Reportamos cuál se considera legítimo y cuál huérfano.
                 */
                printf(" legitimo = inodo %d (enlaces=%d); huerfano = inodo %d (enlaces=%d)\n",
                       l, inodos[l].enlaces, h, inodos[h].enlaces);

            } else {

                /*
                 * Si el bloque todavía no había sido reclamado,
                 * lo marcamos como ocupado.
                 */
                bitmap[b] = 1;

                /*
                 * Registramos que el inodo k es el dueño inicial del bloque b.
                 */
                dueno[b] = k;
            }
        }
    }

    /*
     * Imprimimos el bitmap reconstruido.
     */
    printf("Bitmap reconstruido (0=libre, 1=ocupado):\n ");

    for (int i = 0; i < TOTAL_BLOQUES; i++) {
        printf("%d", bitmap[i]);
    }

    /*
     * Imprimimos la lista de bloques ocupados.
     */
    printf("\n Bloques ocupados:");

    for (int i = 0; i < TOTAL_BLOQUES; i++) {
        if (bitmap[i]) {
            printf(" %d", i);
        }
    }

    printf("\n");

    return 0;
}