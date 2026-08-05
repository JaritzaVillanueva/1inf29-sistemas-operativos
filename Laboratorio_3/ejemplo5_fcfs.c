/* ejemplo5_fcfs.c
 * Simulador FCFS básico.
 * FCFS ejecuta los procesos en orden de llegada.
 *
 * Compilar:
 * gcc ejemplo5_fcfs.c -o ejemplo5
 */

#include <stdio.h>

// Estructura que representa un proceso
typedef struct {
    int pid;          // Identificador del proceso
    int llegada;      // Tiempo en que llega el proceso
    int rafaga;       // Tiempo que necesita usar la CPU
    int inicio;       // Tiempo en que empieza a ejecutarse
    int fin;          // Tiempo en que termina
    int turnaround;   // Tiempo total en el sistema: fin - llegada
    int espera;       // Tiempo esperando CPU: turnaround - rafaga
} Proceso;

// Función que simula el algoritmo FCFS
void fcfs(Proceso p[], int n) {
    int tiempo = 0;   // Tiempo actual de la CPU

    // FCFS recorre los procesos en orden de llegada
    for (int i = 0; i < n; i++) {

        // Si el proceso aún no ha llegado, la CPU queda ociosa
        if (tiempo < p[i].llegada) {
            tiempo = p[i].llegada;
        }

        // El proceso empieza cuando la CPU está disponible
        p[i].inicio = tiempo;

        // El proceso termina después de ejecutar su ráfaga
        p[i].fin = tiempo + p[i].rafaga;

        // Turnaround = tiempo de fin - tiempo de llegada
        p[i].turnaround = p[i].fin - p[i].llegada;

        // Espera = turnaround - tiempo real usando CPU
        p[i].espera = p[i].turnaround - p[i].rafaga;

        // Actualizamos el tiempo actual
        tiempo = p[i].fin;
    }
}

// Función para imprimir los resultados
void imprimir(const char *nombre, Proceso p[], int n) {
    printf("=== %s ===\n", nombre);
    printf("PID  Llegada  Rafaga  Inicio  Fin  Turnaround  Espera\n");

    int sum_t = 0; // Suma de turnaround
    int sum_e = 0; // Suma de espera

    for (int i = 0; i < n; i++) {
        printf(" %d     %2d      %2d      %2d    %2d      %2d        %2d\n",
               p[i].pid,
               p[i].llegada,
               p[i].rafaga,
               p[i].inicio,
               p[i].fin,
               p[i].turnaround,
               p[i].espera);

        sum_t += p[i].turnaround;
        sum_e += p[i].espera;
    }

    printf("Promedio turnaround: %.2f\n", (double)sum_t / n);
    printf("Promedio espera:     %.2f\n", (double)sum_e / n);
}

int main() {
    // Lista de procesos: PID, llegada, ráfaga, inicio, fin, turnaround, espera
    Proceso p[] = {
        {1, 0, 7, 0, 0, 0, 0},
        {2, 2, 4, 0, 0, 0, 0},
        {3, 4, 1, 0, 0, 0, 0},
        {4, 5, 4, 0, 0, 0, 0}
    };
    /*
    Proceso p[MAX_PROCESOS];
    int n;

    printf("Ingrese cantidad de procesos: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        p[i].pid = i + 1;

        printf("\nProceso P%d\n", p[i].pid);

        printf("Llegada: ");
        scanf("%d", &p[i].llegada);

        printf("Rafaga: ");
        scanf("%d", &p[i].rafaga);

        p[i].inicio = 0;
        p[i].fin = 0;
        p[i].turnaround = 0;
        p[i].espera = 0;
    }
    */

    // Cantidad de procesos
    int n = sizeof(p) / sizeof(p[0]);

    // Ejecutamos FCFS
    fcfs(p, n);

    // Imprimimos resultados
    imprimir("FCFS", p, n);

    return 0;
}