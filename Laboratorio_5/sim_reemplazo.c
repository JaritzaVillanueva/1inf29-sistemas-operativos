/* sim_reemplazo.c - Simulador de reemplazo de paginas con diagrama ASCII
 * FIFO esta implementado. LRU y OPT son tu tarea.
 * Compilar: gcc sim_reemplazo.c -o sim_reemplazo
 */
#include <stdio.h>
#define MAX_REF    64
#define MAX_MARCOS  8
#define ETIQUETA   18   /* ancho fijo de la columna de etiquetas */
/* historia[i][m] = pagina en el marco m despues de la referencia i (-1 = vacio) */
int historia[MAX_REF][MAX_MARCOS];
int hubo_fallo[MAX_REF];
/* --------------------------------------------------------------------------
 * FIFO (implementado): los marcos forman una cola.
 * 'siguiente' apunta al marco que sera reemplazado en el proximo desalojo.
 * --------------------------------------------------------------------------- */
int fifo(int ref[], int n_ref, int n_marcos) {
    int marco[MAX_MARCOS];
    int siguiente = 0, cargados = 0, fallos = 0;
    for (int m = 0; m < n_marcos; m++) marco[m] = -1;
    for (int i = 0; i < n_ref; i++) {
        int pagina = ref[i], encontrada = 0;
        /* Buscar la pagina en los marcos */
        for (int m = 0; m < n_marcos; m++)
            if (marco[m] == pagina) { encontrada = 1; break; }
        if (encontrada) {
            hubo_fallo[i] = 0;                 /* acierto */
        } else {
            hubo_fallo[i] = 1; fallos++;       /* fallo de pagina */
            if (cargados < n_marcos) {
                marco[cargados++] = pagina;    /* aun hay marcos vacios */
            } else {
                marco[siguiente] = pagina;     /* desalojar al mas antiguo */
                siguiente = (siguiente + 1) % n_marcos;
            }
        }
        /* Registrar el estado de los marcos para el diagrama */
        for (int m = 0; m < n_marcos; m++) historia[i][m] = marco[m];
    }
    return fallos;
}

int lru(int ref[], int n_ref, int n_marcos) {
    int marco[MAX_MARCOS];
    int ultimo_uso[MAX_MARCOS]; /* instante del ultimo acceso a cada marco */
    int cargados = 0, fallos = 0;
 
    for (int m = 0; m < n_marcos; m++) {
        marco[m] = -1;
        ultimo_uso[m] = -1;
    }
 
    for (int i = 0; i < n_ref; i++) {
        int pagina = ref[i];
        int marco_encontrado = -1;
 
        /* Buscar si la pagina ya esta en algun marco */
        for (int m = 0; m < n_marcos; m++)
            if (marco[m] == pagina) { marco_encontrado = m; break; }
 
        if (marco_encontrado != -1) {
            /* Acierto: actualizar ultimo uso */
            hubo_fallo[i] = 0;
            ultimo_uso[marco_encontrado] = i;
        } else {
            /* Fallo de pagina */
            hubo_fallo[i] = 1; fallos++;
            if (cargados < n_marcos) {
                /* Aun hay marcos libres */
                marco[cargados] = pagina;
                ultimo_uso[cargados] = i;
                cargados++;
            } else {
                /* Buscar la victima: marco con el menor ultimo_uso */
                int victima = 0;
                for (int m = 1; m < n_marcos; m++)
                    if (ultimo_uso[m] < ultimo_uso[victima])
                        victima = m;
                marco[victima] = pagina;
                ultimo_uso[victima] = i;
            }
        }
        for (int m = 0; m < n_marcos; m++) historia[i][m] = marco[m];
    }
    return fallos;
}
 
/* Funcion auxiliar: devuelve el indice de la proxima aparicion de 'pagina'
   en ref[desde..n_ref-1]. Si no aparece, devuelve n_ref (infinito). */
static int proxima_aparicion(int ref[], int n_ref, int desde, int pagina) {
    for (int j = desde; j < n_ref; j++)
        if (ref[j] == pagina) return j;
    return n_ref; /* nunca vuelve a usarse */
}
 
int opt(int ref[], int n_ref, int n_marcos) {
    int marco[MAX_MARCOS];
    int cargados = 0, fallos = 0;
 
    for (int m = 0; m < n_marcos; m++) marco[m] = -1;
 
    for (int i = 0; i < n_ref; i++) {
        int pagina = ref[i];
        int encontrada = 0;
 
        /* Buscar si la pagina ya esta cargada */
        for (int m = 0; m < n_marcos; m++)
            if (marco[m] == pagina) { encontrada = 1; break; }
 
        if (encontrada) {
            hubo_fallo[i] = 0;
        } else {
            hubo_fallo[i] = 1; fallos++;
            if (cargados < n_marcos) {
                marco[cargados++] = pagina;
            } else {
                /* Buscar la victima: pagina cuyo proximo uso es el mas lejano */
                int victima = 0;
                int max_dist = -1;
                for (int m = 0; m < n_marcos; m++) {
                    int dist = proxima_aparicion(ref, n_ref, i + 1, marco[m]);
                    if (dist > max_dist) {
                        max_dist = dist;
                        victima = m;
                    }
                }
                marco[victima] = pagina;
            }
        }
        for (int m = 0; m < n_marcos; m++) historia[i][m] = marco[m];
    }
    return fallos;
}

/* --------------------------------------------------------------------------
 * DIBUJAR (infraestructura, no la toques): imprime el diagrama ASCII
 * usando los arreglos 'historia' y 'hubo_fallo' que dejo el algoritmo.
 * --------------------------------------------------------------------------- */
static void linea_borde(int n_ref) {
    for (int k = 0; k < ETIQUETA; k++) putchar(' ');
    putchar('+');
    for (int i = 0; i < n_ref; i++) printf("---+");
    putchar('\n');
}

void dibujar(int ref[], int n_ref, int n_marcos) {
    printf("%-*s ", ETIQUETA, "Referencia:");
    for (int i = 0; i < n_ref; i++) printf("%2d  ", ref[i]);
    putchar('\n');
    linea_borde(n_ref);
    for (int m = 0; m < n_marcos; m++) {
        char etq[32];
        snprintf(etq, sizeof(etq), "Marco %d", m);
        printf("%-*s|", ETIQUETA, etq);
        for (int i = 0; i < n_ref; i++) {
            if (historia[i][m] == -1) printf(" . |");
            else printf("%2d |", historia[i][m]);
        }
        putchar('\n');
    }
    linea_borde(n_ref);
    printf("%-*s ", ETIQUETA, "Estado:");
    for (int i = 0; i < n_ref; i++) printf(" %c  ", hubo_fallo[i] ? 'F' : '.');
    putchar('\n');
}

void reporte(const char *nombre, int fallos, int n_ref,
             int ref[], int n_marcos) {
    printf("=== %s con %d marcos ===\n", nombre, n_marcos);
    printf("Fallos de pagina: %d / %d   (aciertos: %.1f%%)\n",
           fallos, n_ref, 100.0 * (n_ref - fallos) / n_ref);
    dibujar(ref, n_ref, n_marcos);
    printf("\n");
}

int main(void) {
    int ref[] = {7, 0, 1, 2, 0, 3, 0, 4, 2, 3, 0, 3, 2};
    int n_ref = sizeof(ref) / sizeof(ref[0]);
    int n_marcos = 3;
    //reporte("FIFO", fifo(ref, n_ref, n_marcos), n_ref, ref, n_marcos);
    //reporte("LRU", lru(ref, n_ref, n_marcos), n_ref, ref, n_marcos);
    //reporte("OPT", opt(ref, n_ref, n_marcos), n_ref, ref, n_marcos);

    /* Verificacion con cadena de Belady */
    printf("--- Verificacion anomalia de Belady ---\n");
    int ref2[] = {1,2,3,4,1,2,5,1,2,3,4,5};
    int n2 = sizeof(ref2)/sizeof(ref2[0]);
    //reporte("FIFO (3 marcos)", fifo(ref2, n2, 3), n2, ref2, 3);
    reporte("FIFO (4 marcos)", fifo(ref2, n2, 4), n2, ref2, 4);

    return 0;
}