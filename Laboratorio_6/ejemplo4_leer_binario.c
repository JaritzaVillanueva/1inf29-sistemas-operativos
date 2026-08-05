/* ejemplo4_leer_binario.c
 * Escribe y luego lee estructuras desde un archivo binario,
 * ubicándolas por bloques de tamaño fijo con fseek() + fread().
 *
 * Compilar:
 * gcc -Wall -Wextra ejemplo4_leer_binario.c -o ejemplo4
 *
 * Ejecutar:
 * ./ejemplo4
 */

#include <stdio.h>      // Permite usar FILE, fopen(), fwrite(), fread(), fseek(), printf(), perror() y fclose().
#include <stdlib.h>     // Incluye funciones generales de la biblioteca estándar.
#include <stdint.h>     // Permite usar tipos enteros de tamaño fijo como uint8_t, uint16_t y uint32_t.
#include <string.h>     // Permite usar memset() y memcpy().

#define BLOQUE 64       // Define el tamaño fijo de cada bloque en bytes.

/* 
 * Cabecera del archivo.
 * En este ejemplo, la cabecera vive en el bloque 0.
 */
typedef struct {
    uint32_t magic;       // Número mágico usado para validar que el archivo tiene el formato esperado.
    uint16_t n_registros; // Cantidad de registros que habrá después de la cabecera.
} Cabecera;

/*
 * Registro de ejemplo.
 * Cada registro vive en su propio bloque, empezando desde el bloque 1.
 */
typedef struct {
    uint32_t id;          // Identificador numérico del registro.
    char nombre[20];      // Nombre asociado al registro.
} Registro;

/*
 * Función que lee un bloque completo desde el archivo.
 * Recibe:
 *   f   : archivo abierto.
 *   n   : número de bloque que se desea leer.
 *   buf : buffer donde se guardarán los bytes leídos.
 */
int leer_bloque(FILE *f, int n, void *buf) {

    /*
     * fseek() mueve el cursor del archivo.
     * El bloque n empieza en el byte:
     *
     *     n * BLOQUE
     *
     * SEEK_SET indica que el desplazamiento se cuenta desde el inicio del archivo.
     */
    if (fseek(f, (long)n * BLOQUE, SEEK_SET) != 0) {
        return -1;        // Si fseek falla, se retorna -1 para indicar error.
    }

    /*
     * fread() lee un bloque completo.
     * Aquí se lee 1 elemento de tamaño BLOQUE bytes.
     */
    if (fread(buf, BLOQUE, 1, f) != 1) {
        return -1;        // Si no se pudo leer el bloque completo, se retorna -1.
    }

    return 0;             // Si todo salió bien, se retorna 0.
}

int main(void) {

    /*
     * ---------------------------------------------------------
     * Parte 1: construir un archivo binario de ejemplo.
     * ---------------------------------------------------------
     */

    uint8_t img[4 * BLOQUE];
    // Crea un arreglo de bytes que representará una imagen de disco pequeña.
    // Tiene 4 bloques, cada uno de tamaño BLOQUE.

    memset(img, 0, sizeof img);
    // Inicializa toda la imagen con ceros.
    // Así los bytes no usados quedan vacíos.

    Cabecera cab = { 0xCAFE, 2 };
    // Crea una cabecera.
    // magic = 0xCAFE sirve para validar el formato.
    // n_registros = 2 indica que habrá dos registros.

    memcpy(img + 0 * BLOQUE, &cab, sizeof cab);
    // Copia la cabecera dentro del bloque 0 de la imagen.
    // img + 0 * BLOQUE apunta al inicio del bloque 0.

    Registro r1 = { 101, "sensor-temperatura" };
    // Crea el primer registro con id 101 y nombre "sensor-temperatura".

    Registro r2 = { 102, "sensor-humedad" };
    // Crea el segundo registro con id 102 y nombre "sensor-humedad".

    memcpy(img + 1 * BLOQUE, &r1, sizeof r1);
    // Copia el primer registro en el bloque 1.
    // img + 1 * BLOQUE apunta al inicio del bloque 1.

    memcpy(img + 2 * BLOQUE, &r2, sizeof r2);
    // Copia el segundo registro en el bloque 2.
    // img + 2 * BLOQUE apunta al inicio del bloque 2.

    FILE *w = fopen("datos.bin", "wb");
    // Abre el archivo datos.bin en modo escritura binaria.
    // Si el archivo no existe, lo crea.
    // Si existe, reemplaza su contenido.

    fwrite(img, 1, sizeof img, w);
    // Escribe toda la imagen de bytes dentro del archivo datos.bin.

    fclose(w);
    // Cierra el archivo después de escribirlo.

    /*
     * ---------------------------------------------------------
     * Parte 2: leer el archivo binario como si fuera un disco.
     * ---------------------------------------------------------
     */

    FILE *f = fopen("datos.bin", "rb");
    // Abre datos.bin en modo lectura binaria.

    if (!f) {
        // Si fopen devuelve NULL, significa que no se pudo abrir el archivo.

        perror("fopen");
        // Imprime el error ocurrido.

        return 1;
        // Termina el programa con error.
    }

    uint8_t buf[BLOQUE];
    // Buffer para guardar temporalmente un bloque leído del archivo.

    leer_bloque(f, 0, buf);
    // Lee el bloque 0 del archivo, donde debería estar la cabecera.

    Cabecera c;
    // Declara una variable donde se guardará la cabecera interpretada.

    memcpy(&c, buf, sizeof c);
    // Copia los primeros bytes del buffer hacia la estructura Cabecera.
    // Esto interpreta los bytes del bloque como una cabecera.

    printf("Cabecera: magic=0x%04X n_registros=%u\n",
           c.magic,
           c.n_registros);
    // Imprime el número mágico y la cantidad de registros.

    if (c.magic != 0xCAFE) {
        // Verifica si el número mágico es el esperado.
        // Esto sirve para comprobar que el archivo tiene el formato correcto.

        printf(" Magic invalido: el archivo no tiene el formato esperado.\n");
        // Muestra mensaje de error si el número mágico no coincide.

        fclose(f);
        // Cierra el archivo antes de salir.

        return 1;
        // Termina el programa con error.
    }

    /*
     * Recorre los registros indicados por la cabecera.
     * Como los registros empiezan en el bloque 1,
     * el registro i se lee desde el bloque 1 + i.
     */
    for (int i = 0; i < c.n_registros; i++) {

        leer_bloque(f, 1 + i, buf);
        // Lee el bloque donde está el registro actual.

        Registro r;
        // Declara una estructura para guardar el registro interpretado.

        memcpy(&r, buf, sizeof r);
        // Copia los bytes del buffer hacia la estructura Registro.

        printf("Registro en bloque %d: id=%u nombre=%s\n",
               1 + i,
               r.id,
               r.nombre);
        // Imprime el número de bloque, el id y el nombre del registro.
    }

    fclose(f);
    // Cierra el archivo binario después de leerlo.

    printf("\nsizeof(Cabecera)=%zu sizeof(Registro)=%zu BLOQUE=%d\n",
           sizeof(Cabecera),
           sizeof(Registro),
           BLOQUE);
    // Imprime los tamaños reales de las estructuras en memoria.
    // Esto permite observar que una estructura puede tener relleno por alineación.

    return 0;
    // Termina correctamente.
}