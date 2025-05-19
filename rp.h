#ifndef RP_H
#define RP_H

#include "Pipe.h"
#include "BD.h"
#include <pthread.h>  // Añadido para los hilos
#include "rp.c"
// Estructura para las solicitudes en el buffer compartido
typedef struct {
    Mensaje solicitud;
    int ocupado;
} Buffer_Elemento;

// Función principal que inicia el receptor de peticiones
void iniciar_receptor(const char *pipe, const char *archivo_bd, int verbose, const char *archivo_salida);

// Manejadores de operaciones
void manejar_devolucion(const Mensaje *msg, int verbose);
void manejar_renovacion(const Mensaje *msg, int verbose);
void manejar_prestamo(const Mensaje *msg, int verbose);


// Funciones para respuestas
void enviar_respuesta_a_cliente(int pipe_fd, const Mensaje *solicitud, Respuesta *respuesta);

#endif // RP_H