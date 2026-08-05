#include <stdio.h>
/* Retorna la menor potencia de 2 >= n.
 * Ej: 100 -> 128, 64 -> 64, 65 -> 128 */
int potencia_de_2_mayor_o_igual(int n) {
    int p = 1;
    while (p < n) p *= 2;
    return p;
}
/* Dada la direccion de inicio de un bloque y su tamano,
 * retorna la direccion de inicio de su buddy. */
int direccion_buddy(int inicio, int tamanio) {
    /* Truco clasico: XOR entre la direccion y el tamano. */
    return inicio ^ tamanio;
}
int main(void) {
    /* Prueba de potencias */
    int valores[] = {1, 50, 64, 65, 100, 240, 256, 257, 500};
    int n = sizeof(valores) / sizeof(valores[0]);
    printf("=== Potencia de 2 >= n ===\n");
    for (int i = 0; i < n; i++) {
        printf("  %3d -> %d\n", valores[i],
               potencia_de_2_mayor_o_igual(valores[i]));
    }
    /* Prueba de buddies */
    printf("\n=== Buddy de cada bloque ===\n");
    printf("  Bloque en 0,   tam 128 -> buddy en %d\n", direccion_buddy(0, 128));
    printf("  Bloque en 128, tam 128 -> buddy en %d\n", direccion_buddy(128, 128));
    printf("  Bloque en 256, tam 64  -> buddy en %d\n", direccion_buddy(256, 64));
    printf("  Bloque en 320, tam 64  -> buddy en %d\n", direccion_buddy(320, 64));
    return 0;
}