#ifndef PIPE_H
#define PIPE_H

#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
//Uso en Windows
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
//Uso en Linux
#else
#include <unistd.h>
#include <errno.h>
#endif

// Estructura para mensajes
typedef struct {
    char operacion;        // 'D' (Devolver), 'R' (Renovar), 'P' (Préstamo), 'Q' (Quit/Salir)
    char titulo[100];      // Título del libro
    int ISBN;              // ISBN del libro
    int pid;
} Mensaje;

// Estructura para respuestas
typedef struct {
    int exito;             // 1 = éxito, 0 = fracaso
    char mensaje[100];     // Mensaje descriptivo o fecha de devolución
} Respuesta;

// Funciones para el PS (Proceso Solicitante)
int conectar_a_pipe_receptor(const char *nombre_pipe);
int enviar_solicitud(int pipe_fd, Mensaje *solicitud);
int recibir_respuesta(int pipe_fd, Respuesta *respuesta);
void cerrar_pipe(int pipe_fd);

// Funciones para el RP (Receptor de Peticiones)
int crear_pipe_receptor(const char *nombre_pipe);
int esperar_conexion(int pipe_fd);
int recibir_solicitud(int pipe_fd, Mensaje *solicitud);
int enviar_respuesta(int pipe_fd, Respuesta *respuesta);

#endif // PIPE_H