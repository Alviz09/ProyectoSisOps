#include "ps.h"
#include "Pipe.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void iniciar_ps_menu(const char *pipe) {
    int opcion;
    do {
        printf("\n--- Menú de Biblioteca ---\n");
        printf("1. Devolver un libro\n");
        printf("2. Renovar un libro\n");
        printf("3. Solicitar prestado un libro\n");
        printf("0. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);

        switch (opcion) {
            case 1:
                printf("Opción: Devolver un libro\n");
                // Aquí iría la lógica para devolver un libro
                break;
            case 2:
                printf("Opción: Renovar un libro\n");
                // Aquí iría la lógica para renovar un libro
                break;
            case 3:
                printf("Opción: Solicitar prestado un libro\n");
                // Aquí iría la lógica para solicitar un préstamo
                break;
            case 0:
                printf("Saliendo del menú.\n");
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n");
        }
    } while (opcion != 0);
}