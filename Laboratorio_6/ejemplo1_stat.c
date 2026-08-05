/* ejemplo1_stat.c
 * Lee los metadatos de un archivo usando stat().
 * Compilar: gcc -Wall -Wextra ejemplo1_stat.c -o ejemplo1
 * Ejecutar: ./ejemplo1 <archivo>
 */

#include <stdio.h>      // Permite usar printf(), fprintf() y perror().
#include <stdlib.h>     // Permite usar funciones generales de la biblioteca estándar.
#include <sys/stat.h>   // Permite usar struct stat, stat(), S_ISDIR() y S_ISREG().
#include <time.h>       // Permite usar localtime(), struct tm y strftime().

int main(int argc, char *argv[]) {  // Función principal; recibe argumentos desde la terminal.

    if (argc < 2) {  // Verifica si el usuario no pasó el nombre del archivo.
        fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);  // Muestra cómo usar el programa.
        return 1;  // Termina con error porque falta el argumento.
    }

    struct stat st;  // Declara una estructura donde se guardarán los metadatos del archivo.

    if (stat(argv[1], &st) != 0) {  // Llama a stat() para llenar st con los metadatos del archivo.
        perror("stat");  // Muestra el error si stat() falla, por ejemplo si el archivo no existe.
        return 1;  // Termina con error.
    }

    printf("Archivo: %s\n", argv[1]);  // Imprime el nombre recibido como argumento.

    printf(" Numero de inodo : %lu\n", (unsigned long)st.st_ino);  // Imprime el número de inodo del archivo.

    printf(" Tamano (bytes) : %lld\n", (long long)st.st_size);  // Imprime el tamaño del archivo en bytes.

    printf(" Enlaces duros : %lu\n", (unsigned long)st.st_nlink);  // Imprime cuántos nombres apuntan a este inodo.

    printf(" Es directorio : %s\n", S_ISDIR(st.st_mode) ? "si" : "no");  // Verifica si el archivo es un directorio.

    printf(" Es archivo reg. : %s\n", S_ISREG(st.st_mode) ? "si" : "no");  // Verifica si es un archivo regular.

    printf(" Permisos (octal): %o\n", st.st_mode & 0777);  // Imprime solo los permisos en formato octal, por ejemplo 644.

    char buf[64];  // Crea un arreglo de caracteres para guardar la fecha formateada.

    struct tm *t = localtime(&st.st_mtime);  // Convierte la fecha de modificación a hora local.

    strftime(buf, sizeof buf, "%Y-%m-%d %H:%M:%S", t);  // Formatea la fecha como texto legible.

    printf(" Ultima modif. : %s\n", buf);  // Imprime la fecha de última modificación.

    return 0;  // Termina el programa correctamente.
}