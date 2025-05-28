#ifndef RECEPTOR_H
#define RECEPTOR_H
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/select.h>

#include <pthread.h>

#define N 10

typedef struct {
    char accion[2];
    char nombre[51];
    char isbn[5];
    char pipe_respuesta[100];
} Solicitud;

// Variables globales
extern int terminar;
extern char pipe_receptor[256];
extern char archivo_datos[256];
extern char archivo_salida[256];
extern int verbose;
extern char buffer_bytes[N * sizeof(Solicitud)];
extern int contador;
extern int pos_entrada;
extern int pos_salida;
extern pthread_mutex_t mutex;
extern pthread_cond_t no_lleno;
extern pthread_cond_t no_vacio;

// Declaración de funciones
void mostrar_uso(const char *nombre_programa);
int detectar_modo(int argc, char *argv[]);
void configurar_modo_interactivo();
int procesar_argumentos(int argc, char *argv[]);
void escribir_estado_final();
void generar_reporte();
void poner_en_buffer(Solicitud *solicitud);
void obtener_del_buffer(Solicitud *solicitud);
char* procesar_solicitud(Solicitud* solicitud);
void enviar_respuesta(const char* pipe_respuesta, const char* resultado);
void* hilo_consumidor(void *arg);
void* escuchar_comandos(void *arg);

#endif // RECEPTOR_H
