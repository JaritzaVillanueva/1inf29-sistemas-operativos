/* ejemplo3_enlaces.c
 * Muestra que dos nombres pueden apuntar al MISMO inodo usando enlaces duros.
 *
 * Compilar:
 * gcc -Wall -Wextra ejemplo3_enlaces.c -o ejemplo3
 *
 * Ejecutar:
 * ./ejemplo3
 */

#include <stdio.h>      // Permite usar printf(), fopen(), fprintf(), fclose() y perror().
#include <stdlib.h>     // Incluye funciones generales de la biblioteca estándar.
#include <unistd.h>     // Permite usar link() y unlink().
#include <sys/stat.h>   // Permite usar struct stat y stat().

/* 
 * Función auxiliar para mostrar el número de inodo
 * y el contador de enlaces de un archivo.
 */
static void mostrar(const char *ruta) {

    struct stat st;
    // Declara una estructura stat.
    // Aquí se guardarán los metadatos del archivo indicado por ruta.

    if (stat(ruta, &st) != 0) {
        // stat() intenta obtener los metadatos del archivo.
        // Si falla, devuelve un valor distinto de 0.

        perror("stat");
        // Imprime el error ocurrido.

        return;
        // Sale de la función mostrar().
    }

    printf(" %-12s inodo=%lu enlaces=%lu\n",
           ruta,
           (unsigned long)st.st_ino,
           (unsigned long)st.st_nlink);
    // Imprime:
    // ruta       : nombre del archivo consultado.
    // st.st_ino  : número de inodo.
    // st.st_nlink: contador de enlaces duros.
}

int main(void) {

    FILE *f = fopen("original.txt", "w");
    // Crea o abre el archivo original.txt en modo escritura.
    // Si ya existía, su contenido anterior se reemplaza.

    fprintf(f, "contenido compartido\n");
    // Escribe texto dentro de original.txt.

    fclose(f);
    // Cierra el archivo después de escribir.

    printf("Tras crear original.txt:\n");
    // Mensaje para indicar el primer estado del archivo.

    mostrar("original.txt");
    // Muestra el inodo y contador de enlaces de original.txt.
    // En este momento normalmente enlaces = 1.

    /*
     * link() crea un enlace duro.
     * Es decir, crea un segundo nombre para el mismo inodo.
     */
    if (link("original.txt", "copia_dura.txt") != 0) {
        // Crea el nombre copia_dura.txt apuntando al mismo inodo que original.txt.
        // Si falla, devuelve un valor distinto de 0.

        perror("link");
        // Imprime el error ocurrido.

        return 1;
        // Termina el programa con error.
    }

    printf("Tras crear el enlace duro copia_dura.txt:\n");
    // Mensaje para indicar que ya se creó el enlace duro.

    mostrar("original.txt");
    // Muestra la información de original.txt.
    // Ahora su contador de enlaces debería ser 2.

    mostrar("copia_dura.txt");
    // Muestra la información de copia_dura.txt.
    // Debe tener el mismo número de inodo que original.txt.

    /*
     * unlink() elimina un nombre del sistema de archivos.
     * No necesariamente borra los datos inmediatamente.
     */
    unlink("copia_dura.txt");
    // Elimina el nombre copia_dura.txt.
    // El inodo sigue existiendo porque aún queda original.txt.

    printf("Tras borrar copia_dura.txt:\n");
    // Mensaje para indicar que se eliminó uno de los nombres.

    mostrar("original.txt");
    // Muestra que original.txt sigue existiendo.
    // Su contador de enlaces debería volver a 1.

    unlink("original.txt");
    // Elimina el último nombre que apuntaba al inodo.
    // Ahora el sistema puede liberar el inodo y sus bloques.

    return 0;
    // Termina correctamente.
}