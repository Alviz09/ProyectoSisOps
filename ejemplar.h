#ifndef EJEMPLAR_H
#define EJEMPLAR_H

#include <time.h> // Para struct tm

typedef struct {
    int id_ejemplar;
    char status;
    struct tm fecha; // Fecha local (registro, préstamo, etc.)
} ejemplar;

#endif // EJEMPLAR_H