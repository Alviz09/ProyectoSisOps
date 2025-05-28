#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "receptor.h"
#include "solicitante.h"

void mostrar_menu() {
    printf("\nMenú de Biblioteca:\n");
    printf("1. Devolver un libro\n");
    printf("2. Renovar un libro\n");
    printf("3. Solicitar prestado un libro\n");
    printf("Seleccione una opción: ");
}

int main() {
    char *fifo_path = "/tmp/fifo_twoway";
    mkfifo(fifo_path, 0666); // Asegurar que exista
 
    int opcion;
    mostrar_menu();
    scanf("%d", &opcion);

    int fd = open(fifo_path, O_RDONLY); // Abrir en modo lectura
    if (fd == -1){
	perror("Error al abrir FIFO");
        return 1;
	}

    char buffer[100];
    read(fd, buffer, sizeof(buffer));
    printf("Proceso lector recibió: %s\n", buffer);
    close(fd);
    
    switch (opcion) {
        case 1:
            printf("Ha seleccionado devolver un libro.\n");
            break;
        case 2:
            printf("Ha seleccionado renovar un libro.\n");
            break;
        case 3:
            printf("Ha seleccionado solicitar prestado un libro.\n");
            break;
        default:
            printf("Opción no válida.\n");
    }
    return 0;
}

