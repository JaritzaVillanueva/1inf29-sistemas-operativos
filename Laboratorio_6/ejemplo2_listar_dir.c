/* ejemplo2_listar_dir.c
 * Recorre un directorio y muestra cada entrada:
 * nombre -> número de inodo.
 *
 * Compilar:
 * gcc -Wall -Wextra ejemplo2_listar_dir.c -o ejemplo2
 *
 * Ejecutar:
 * ./ejemplo2 <directorio>
 *
 * Si no se pasa un directorio como argumento,
 * el programa recorre el directorio actual ".".
 */

#include <stdio.h>      // Permite usar printf() y perror().
#include <stdlib.h>     // Incluye funciones generales de la biblioteca estándar.
#include <dirent.h>     // Permite usar DIR, struct dirent, opendir(), readdir() y closedir().
#include <sys/stat.h>
int main(int argc, char *argv[]) {   // Función principal; recibe argumentos desde la terminal.

    const char *ruta = (argc >= 2) ? argv[1] : ".";
    // Si el usuario pasó un argumento, se usa argv[1] como ruta del directorio.
    // Si no pasó argumento, se usa "." que representa el directorio actual.

    DIR *dir = opendir(ruta);
    // opendir() intenta abrir el directorio indicado por ruta.
    // Si tiene éxito, devuelve un puntero de tipo DIR *.
    // Ese puntero se usará luego para recorrer el directorio.

    if (!dir) {
        // Si dir es NULL, significa que opendir() falló.
        // Puede fallar si la ruta no existe o si no es un directorio.

        perror("opendir");
        // perror() imprime un mensaje de error relacionado con opendir().

        return 1;
        // Termina el programa indicando error.
    }

    printf("Entradas de %s:\n", ruta);
    // Imprime el nombre del directorio que se está recorriendo.

    printf(" %-20s %s\n", "NOMBRE", "INODO");
    // Imprime una cabecera para ordenar la salida.
    // %-20s reserva 20 espacios para mostrar el nombre alineado a la izquierda.

    struct dirent *e;
    // Declara un puntero a struct dirent.
    // Cada struct dirent representa una entrada del directorio.

    while ((e = readdir(dir)) != NULL) {
        // readdir() lee la siguiente entrada del directorio.
        // Cada llamada devuelve una entrada nueva.
        // Cuando ya no quedan entradas, devuelve NULL.
        char ruta_completa[1024];
        // Arreglo donde construiremos la ruta completa de la entrada.

        snprintf(ruta_completa, sizeof(ruta_completa), "%s/%s", ruta, e->d_name);
        // Construye la ruta completa.
        // Por ejemplo, si ruta="." y e->d_name="demo.txt",
        // entonces ruta_completa="./demo.txt".

        struct stat st;
        // Estructura donde se guardarán los metadatos de la entrada.

        if (stat(ruta_completa, &st) != 0) {
            // stat() obtiene los metadatos de la entrada.
            // Si falla, se imprime un error y se continúa con la siguiente entrada.

            perror("stat");
            // Muestra el error producido por stat().

            continue;
            // Salta esta entrada y sigue con la siguiente.
        }
        printf(" %-25s %-15lu %lld bytes\n",
               e->d_name,
               (unsigned long)e->d_ino,
               (long long)st.st_size);
        // Imprime:
        // e->d_name  : nombre de la entrada.
        // e->d_ino   : número de inodo.
        // st.st_size : tamaño en bytes obtenido con stat().
    }

    closedir(dir);
    // Cierra el directorio abierto con opendir().
    // Es importante liberar los recursos usados por el sistema.

    return 0;
    // Termina correctamente.
}