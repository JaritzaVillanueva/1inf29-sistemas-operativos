#include <stdio.h>
int main(void) {
    printf("Antes del acceso ilegal...\n");
    //fflush(stdout);   /* Forzar la impresion ANTES del crash */
    /* Direccion arbitraria que casi seguro NO esta mapeada
     * en el espacio virtual de este proceso. */
    char *puntero = (char *)0x12345678;
    char valor = *puntero;   /* <-- Aqui ocurre el SIGSEGV */
    printf("Despues del acceso (esto NO debe imprimirse). Valor=%c\n", valor);
    return 0;
}