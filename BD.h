#ifndef BD_H
#define BD_H

#include "ejemplar.h"


#define MAX_TITULO 100
#define MAX_EJEMPLARES 10
#define MAX_LIBROS 200

typedef struct {
    char titulo[MAX_TITULO];
    int ISBN;
    int num_ejemplares;
    ejemplar ejemplares_list[MAX_EJEMPLARES];
} Libro;

// Devuelve la cantidad de libros leídos
int leerArchivo(Libro libros[], int max_libros, const char *nombre_archivo);
int imprimirLibros();

#endif // BD_H;