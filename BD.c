#include <stdio.h>
#include <string.h>
#include "BD.h"
int imprimirLibros() {
    Libro libros[MAX_LIBROS];
    int cantidad = leerArchivo(libros, MAX_LIBROS, "BD.txt");

    printf("Libros leídos: %d\n", cantidad);
    for (int i = 0; i < cantidad; i++) {
        printf("Libro: %s, ISBN: %d, Ejemplares: %d\n", libros[i].titulo, libros[i].ISBN, libros[i].num_ejemplares);
        for (int j = 0; j < libros[i].num_ejemplares; j++) {
            ejemplar *e = &libros[i].ejemplares_list[j];
            printf("  Ejemplar %d: ID=%d, Status=%c, Fecha=%02d-%02d-%04d\n",
                   j + 1, e->id_ejemplar, e->status,
                   e->fecha.tm_mday, e->fecha.tm_mon + 1, e->fecha.tm_year + 1900);
        }
    }

    return 0;
}
int leerArchivo(Libro libros[], int max_libros, const char *nombre_archivo) {
    // Abrir el archivo en modo lectura
    // Se asume que el archivo tiene el formato correcto
    FILE *archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) {
        printf("Error al abrir el archivo.\n");
        return 0;
    }

    char linea[256];
    int i = 0;
    // Leer línea por línea
    while (i < max_libros && fgets(linea, sizeof(linea), archivo)) {
        // Leer línea de libro
        char titulo[MAX_TITULO];
        int isbn, num_ejemplares;

        // Quitar salto de línea
        linea[strcspn(linea, "\r\n")] = 0;

                // Leer el título, ISBN y número de ejemplares
        if (sscanf(linea, " %99[^,], %d, %d", titulo, &isbn, &num_ejemplares) == 3) {
            if (isbn > 9999) {
                printf("ISBN inválido (más de 4 dígitos) para el libro: %s\n", titulo);
                // Saltar la lectura de ejemplares para este libro
                for (int j = 0; j < num_ejemplares; j++) {
                    if (!fgets(linea, sizeof(linea), archivo)) break;
                }
                continue;
            }
            strncpy(libros[i].titulo, titulo, MAX_TITULO); 
            libros[i].ISBN = isbn;
            libros[i].num_ejemplares = num_ejemplares;
        
            // Leer los ejemplares
            for (int j = 0; j < num_ejemplares; j++) {
                if (!fgets(linea, sizeof(linea), archivo)) break; // Si falla al leer, salir del bucle
                linea[strcspn(linea, "\r\n")] = 0;
        
                int id, dia, mes, anio;
                char status;
                // leer el id, status y fecha
                if (sscanf(linea, " %d, %c, %d-%d-%d", &id, &status, &dia, &mes, &anio) == 5) {
                    libros[i].ejemplares_list[j].id_ejemplar = id;
                    libros[i].ejemplares_list[j].status = status;
                    libros[i].ejemplares_list[j].fecha.tm_mday = dia;
                    libros[i].ejemplares_list[j].fecha.tm_mon = mes - 1; 
                    libros[i].ejemplares_list[j].fecha.tm_year = anio - 1900; 
                }
            }
            i++;
        }
    }

    fclose(archivo);
    return i; // Retorna la cantidad de libros leídos
}