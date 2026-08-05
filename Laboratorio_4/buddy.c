#include <stdio.h>
#include <stdlib.h>

// ─── Lista libre ───────────────────────────────────────────────
typedef struct Bloque {
    int inicio;
    int tamanio;
    struct Bloque *siguiente;
} Bloque;

static Bloque *lista = NULL;

// ─── Lista de bloques asignados ────────────────────────────────
typedef struct BloqueAsignado {
    int pid;
    int inicio;
    int tamanio;
    struct BloqueAsignado *siguiente;
} BloqueAsignado;

static BloqueAsignado *asignados = NULL;

// ─── Imprimir lista libre ──────────────────────────────────────
static void imprimir_lista(const char *etiqueta) {
    printf("%-20s -> ", etiqueta);
    for (Bloque *b = lista; b != NULL; b = b->siguiente)
        printf("[inicio=%d tam=%d] ", b->inicio, b->tamanio);
    printf("\n");
}

// ─── Eliminar nodo de la lista libre ──────────────────────────
static int eliminar_nodo(int inicio) {
    Bloque *prev = NULL;
    Bloque *cur  = lista;
    while (cur != NULL) {
        if (cur->inicio == inicio) {
            if (prev == NULL)
                lista = cur->siguiente;
            else
                prev->siguiente = cur->siguiente;
            free(cur);
            return 1;
        }
        prev = cur;
        cur  = cur->siguiente;
    }
    return 0;
}

// ─── Fusionar: reinserta el bloque y mergea buddies ───────────
static void fusionar(int inicio, int tamanio) {
    // Paso 1: reinsertar ordenado por inicio
    Bloque *nuevo    = malloc(sizeof(Bloque));
    nuevo->inicio    = inicio;
    nuevo->tamanio   = tamanio;
    nuevo->siguiente = NULL;

    if (lista == NULL || inicio < lista->inicio) {
        nuevo->siguiente = lista;
        lista = nuevo;
    } else {
        Bloque *cur = lista;
        while (cur->siguiente != NULL && cur->siguiente->inicio < inicio)
            cur = cur->siguiente;
        nuevo->siguiente = cur->siguiente;
        cur->siguiente   = nuevo;
    }

    // Paso 2: fusionar buddies repetidamente
    int fusionado;
    do {
        fusionado = 0;
        Bloque *a = lista;
        while (a != NULL && a->siguiente != NULL) {
            Bloque *b = a->siguiente;

            int mismo_tamanio  = (a->tamanio == b->tamanio);
            int contiguos      = (a->inicio + a->tamanio == b->inicio);
            int bien_alineados = (a->inicio % (2 * a->tamanio) == 0);

            if (mismo_tamanio && contiguos && bien_alineados) {
                a->tamanio  *= 2;
                a->siguiente = b->siguiente;
                free(b);
                fusionado = 1;
                break;
            }
            a = a->siguiente;
        }
    } while (fusionado);
}

// ─── Best Fit ──────────────────────────────────────────────────
static int asignar(int tamanio_pedido, int *tam_asignado) {
    int tam = 1;
    while (tam < tamanio_pedido) tam *= 2;

    Bloque *mejor = NULL;
    for (Bloque *b = lista; b != NULL; b = b->siguiente)
        if (b->tamanio >= tam)
            if (mejor == NULL || b->tamanio < mejor->tamanio)
                mejor = b;

    if (mejor == NULL) {
        printf("  [!] No hay espacio para %d\n", tamanio_pedido);
        *tam_asignado = -1;
        return -1;
    }

    while (mejor->tamanio > tam) {
        Bloque *buddy    = malloc(sizeof(Bloque));
        buddy->tamanio   = mejor->tamanio / 2;
        buddy->inicio    = mejor->inicio + buddy->tamanio;
        buddy->siguiente = mejor->siguiente;
        mejor->tamanio   = mejor->tamanio / 2;
        mejor->siguiente = buddy;
    }

    *tam_asignado       = mejor->tamanio;
    int inicio_asignado = mejor->inicio;
    eliminar_nodo(inicio_asignado);
    return inicio_asignado;
}

// ─── Registrar y liberar bloques asignados ────────────────────
static void registrar_bloque(int pid, int inicio, int tamanio) {
    BloqueAsignado *b = malloc(sizeof(BloqueAsignado));
    b->pid       = pid;
    b->inicio    = inicio;
    b->tamanio   = tamanio;
    b->siguiente = asignados;
    asignados    = b;
}

static void liberar_bloque(int pid) {
    BloqueAsignado *prev = NULL;
    BloqueAsignado *cur  = asignados;
    while (cur != NULL) {
        if (cur->pid == pid) {
            printf("  [liberando] pid=%d inicio=%d tam=%d\n",
                   cur->pid, cur->inicio, cur->tamanio);
            fusionar(cur->inicio, cur->tamanio);
            if (prev == NULL)
                asignados = cur->siguiente;
            else
                prev->siguiente = cur->siguiente;
            free(cur);
            return;
        }
        prev = cur;
        cur  = cur->siguiente;
    }
    printf("  [!] pid=%d no encontrado\n", pid);
}

// ─── Main ──────────────────────────────────────────────────────
int main(void) {
    lista = malloc(sizeof(Bloque));
    lista->inicio    = 0;
    lista->tamanio   = 1024;
    lista->siguiente = NULL;

    imprimir_lista("inicial");

    int tam;

    printf("\n-- Bloque 1 solicita 100KB --\n");
    int i1 = asignar(100, &tam);
    registrar_bloque(1, i1, tam);
    imprimir_lista("tras B1");

    printf("\n-- Bloque 2 solicita 200KB --\n");
    int i2 = asignar(200, &tam);
    registrar_bloque(2, i2, tam);
    imprimir_lista("tras B2");

    printf("\n-- Bloque 3 solicita 50KB --\n");
    int i3 = asignar(50, &tam);
    registrar_bloque(3, i3, tam);
    imprimir_lista("tras B3");

    printf("\n-- Bloque 4 solicita 50KB --\n");
    int i4 = asignar(50, &tam);
    registrar_bloque(4, i4, tam);
    imprimir_lista("tras B4");

    printf("\n-- Termina Bloque 2 --\n");
    liberar_bloque(2);
    imprimir_lista("tras liberar B2");

    printf("\n-- Termina Bloque 1 --\n");
    liberar_bloque(1);
    imprimir_lista("tras liberar B1");

    printf("\n-- Termina Bloque 3 --\n");
    liberar_bloque(3);
    imprimir_lista("tras liberar B3");

    printf("\n-- Termina Bloque 4 --\n");
    liberar_bloque(4);
    imprimir_lista("tras liberar B4");

    return 0;
}