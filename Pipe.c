#include "Pipe.h"
#include <string.h>


// Funciones del PS (cliente)
int conectar_a_pipe_receptor(const char *nombre_pipe) {
    // Abre el pipe en modo de escritura
    int fd = open(nombre_pipe, O_RDWR);
    if (fd == -1) {
        perror("Error al conectar al pipe receptor");
    }
    return fd;
}

int enviar_solicitud(int pipe_fd, Mensaje *solicitud) {
    ssize_t bytes = write(pipe_fd, solicitud, sizeof(Mensaje));
    return bytes == sizeof(Mensaje);
}

int recibir_respuesta(int pipe_fd, Respuesta *respuesta) {
    // Leemos directamente del mismo pipe
    ssize_t bytes = read(pipe_fd, respuesta, sizeof(Respuesta));
    return bytes == sizeof(Respuesta);
}

void cerrar_pipe(int pipe_fd) {
    close(pipe_fd);
}

// Funciones del RP (servidor)
int crear_pipe_receptor(const char *nombre_pipe) {
    // Creamos el pipe si no existe
    if (mkfifo(nombre_pipe, 0666) == -1 && errno != EEXIST) {
        perror("Error al crear el pipe");
        return -1;
    }
    
    // Abrimos el pipe en modo lectura 
    int fd = open(nombre_pipe, O_RDWR );
    if (fd == -1) {
        perror("Error al abrir el pipe receptor");
    }
    return fd;
}

int esperar_conexion(int pipe_fd) {
    // No necesitamos cambiar nada, el pipe ya está abierto en modo lectura/escritura
    return pipe_fd != -1;
}

int recibir_solicitud(int pipe_fd, Mensaje *solicitud) {
    ssize_t bytes = read(pipe_fd, solicitud, sizeof(Mensaje));
    return bytes == sizeof(Mensaje);
}

int enviar_respuesta(int pipe_fd, Respuesta *respuesta) {
    // Escribimos directamente en el mismo pipe
    ssize_t bytes = write(pipe_fd, respuesta, sizeof(Respuesta));
    return bytes == sizeof(Respuesta);
}

