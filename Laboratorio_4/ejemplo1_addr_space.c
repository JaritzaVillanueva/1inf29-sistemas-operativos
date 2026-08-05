#include <stdio.h>   
#include <stdlib.h>  
#include <unistd.h>  

/* 
Variable global inicializada:
- Se almacena en la zona de datos inicializados del proceso.
- Existe durante toda la ejecución del programa. 
*/
int variable_global = 42;  
void funcion_dummy(void) {
/* Función vacía:
    - Solo existe para tener una dirección de código que mostrar.
    - Su dirección estará en la zona de código (text segment) del programa. */
}

int main(void) {
    /* 
    Variable local:
    - Se almacena en la pila (stack).
    - Solo existe mientras la función main esté ejecutándose. 
    */
    int variable_local = 100;                 
    /* 
    Reserva memoria dinámica:
    - Se almacena en el heap.
    - Su duración depende de que se libere con free().
    - malloc devuelve un puntero a la memoria reservada. 
    */
    int *variable_heap = malloc(sizeof(int)); 
    /* Asigna el valor 200 al bloque de memoria heap reservado */
    *variable_heap = 200; 

    // Muestra el PID del proceso actual
    printf("PID del proceso = %d\n", getpid());

    // Muestra la dirección de la función (zona de código)
    printf("Direccion del codigo (funcion)  = %p\n", (void *)funcion_dummy);

    // Muestra la dirección de la variable global (zona de datos inicializados)
    printf("Direccion de variable global    = %p\n", (void *)&variable_global);

    // Muestra la dirección de la variable local (stack)
    printf("Direccion de variable local     = %p\n", (void *)&variable_local);

    // Muestra la dirección de la memoria heap
    printf("Direccion de memoria heap       = %p\n", (void *)variable_heap);

    printf("\nAhora ejecuta en otra terminal:\n");

    // Indica cómo ver el mapa de memoria del proceso desde otra terminal
    printf("  cat /proc/%d/maps\n", getpid());

    printf("Presiona Enter cuando termines...\n");
    getchar();  // Espera que el usuario presione Enter para continuar

    free(variable_heap); // Libera la memoria dinámica reservada previamente con malloc

    return 0;            // Termina el programa
}