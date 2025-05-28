#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include "receptor.h"
#include "solicitante.h"

int main() {
    char *fifo_path = "/tmp/fifo_twoway";
    mkfifo(fifo_path, 0666); // Crear FIFO con permisos

    int fd = open(fifo_path, O_WRONLY); // Abrir en modo escritura
    char mensaje[] = "Hola desde el proceso escritor";
    write(fd, mensaje, sizeof(mensaje));
    close(fd);

    return 0;
}
