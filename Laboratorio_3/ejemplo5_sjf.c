#include <stdio.h>

// Estructura que representa un proceso
typedef struct {
    int pid;          // Identificador del proceso
    int llegada;      // Tiempo en que llega el proceso
    int rafaga;       // Tiempo que necesita usar CPU
    int inicio;       // Tiempo en que empieza a ejecutarse
    int fin;          // Tiempo en que termina
    int turnaround;   // Tiempo total en el sistema: fin - llegada
    int espera;       // Tiempo esperando CPU: turnaround - rafaga
    int completado;   // 0 = no ejecutado, 1 = ya ejecutado
} Proceso;

// Simulación de SJF no apropiativo
void sjf(Proceso p[], int n) {
    int tiempo = 0;        // Tiempo actual de la CPU
    int completados = 0;   // Cantidad de procesos ya terminados

    // Repetimos hasta ejecutar todos los procesos
    while (completados < n) {
        int mejor = -1;    // Guardará el índice del proceso con menor ráfaga

        // Buscamos entre los procesos que ya llegaron
        for (int i = 0; i < n; i++) {

            // Solo consideramos procesos no completados y que ya llegaron
            if (p[i].completado == 0 && p[i].llegada <= tiempo) {

                // Elegimos el de menor ráfaga
                if (mejor == -1 || p[i].rafaga < p[mejor].rafaga) {
                    mejor = i;
                }

                // Si hay empate en ráfaga, elegimos el menor PID
                else if (p[i].rafaga == p[mejor].rafaga &&
                         p[i].pid < p[mejor].pid) {
                    mejor = i;
                }
            }
        }

        // Si ningún proceso ha llegado aún, la CPU está ociosa
        if (mejor == -1) {
            tiempo++;
            continue;
        }

        // Ejecutamos el proceso elegido hasta terminarlo
        p[mejor].inicio = tiempo;
        p[mejor].fin = tiempo + p[mejor].rafaga;

        // Calculamos métricas
        p[mejor].turnaround = p[mejor].fin - p[mejor].llegada;
        p[mejor].espera = p[mejor].turnaround - p[mejor].rafaga;

        // Marcamos el proceso como terminado
        p[mejor].completado = 1;

        // Avanzamos el tiempo hasta el fin del proceso
        tiempo = p[mejor].fin;

        // Aumentamos contador de procesos completados
        completados++;
    }
}

// Imprime tabla de resultados
void imprimir(const char *nombre, Proceso p[], int n) {
    printf("=== %s ===\n", nombre);
    printf("PID  Llegada  Rafaga  Inicio  Fin  Turnaround  Espera\n");

    int sum_t = 0;
    int sum_e = 0;

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
    // PID, llegada, ráfaga, inicio, fin, turnaround, espera, completado
    Proceso p[] = {
        {1, 0, 7, 0, 0, 0, 0, 0},
        {2, 2, 4, 0, 0, 0, 0, 0},
        {3, 4, 1, 0, 0, 0, 0, 0},
        {4, 5, 4, 0, 0, 0, 0, 0}
    };

    int n = sizeof(p) / sizeof(p[0]);

    sjf(p, n);

    imprimir("SJF no apropiativo", p, n);

    return 0;
}