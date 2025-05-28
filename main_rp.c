#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "rp.h"

/**
 * Proceso Receptor (RP)
 * 
 * Uso: ./receptor -p pipeReceptor -f filedatos [-v] [-s filesalida]
 * 
 * Donde:
 * -p: Nombre del pipe nominal para comunicación entre procesos
 * -f: Archivo de datos con la BD inicial
 * -v: Modo verbose (opcional)
 * -s: Archivo de salida para guardar el estado final de la BD (opcional)
 */

// Función para mostrar ayuda
void mostrar_ayuda(const char *nombre_programa) {
    printf("Uso: %s -p pipeReceptor -f filedatos [-v] [-s filesalida]\n", nombre_programa);
    printf("Donde:\n");
    printf("  -p pipeReceptor   Nombre del pipe para recibir solicitudes\n");
    printf("  -f filedatos      Archivo con la base de datos inicial\n");
    printf("  -v               Modo verbose (muestra todas las operaciones)\n");
    printf("  -s filesalida    Archivo donde guardar el estado final de la BD\n");
}

int main(int argc, char *argv[]) {
    // Variables para los parámetros
    char pipe_name[100] = "";
    char archivo_entrada[100] = "";
    char archivo_salida[100] = "";
    int verbose = 0;
    
    // Verificar que hay suficientes argumentos
    if (argc < 5) {
        fprintf(stderr, "Error: Faltan argumentos requeridos\n");
        mostrar_ayuda(argv[0]);
        return 1;
    }
    
    // Procesar argumentos de línea de comandos
    int opt;
    while ((opt = getopt(argc, argv, "p:f:vs:")) != -1) {
        switch (opt) {
            case 'p':
                strncpy(pipe_name, optarg, sizeof(pipe_name) - 1);
                pipe_name[sizeof(pipe_name) - 1] = '\0';
                break;
            case 'f':
                strncpy(archivo_entrada, optarg, sizeof(archivo_entrada) - 1);
                archivo_entrada[sizeof(archivo_entrada) - 1] = '\0';
                break;
            case 'v':
                verbose = 1;
                break;
            case 's':
                strncpy(archivo_salida, optarg, sizeof(archivo_salida) - 1);
                archivo_salida[sizeof(archivo_salida) - 1] = '\0';
                break;
            default:
                fprintf(stderr, "Opción desconocida: '-%c'\n", optopt);
                mostrar_ayuda(argv[0]);
                return 1;
        }
    }
    
    // Verificar que se hayan proporcionado los parámetros obligatorios
    if (strlen(pipe_name) == 0) {
        fprintf(stderr, "Error: Debe especificar el nombre del pipe con -p\n");
        mostrar_ayuda(argv[0]);
        return 1;
    }
    
    if (strlen(archivo_entrada) == 0) {
        fprintf(stderr, "Error: Debe especificar el archivo de datos con -f\n");
        mostrar_ayuda(argv[0]);
        return 1;
    }
    
    // Mostrar configuración inicial

    if (access(archivo_entrada, F_OK) != 0) {
        fprintf(stderr, "Error: El archivo de datos '%s' no existe\n", archivo_entrada);
        return 1;
    }
    
    // Construir la ruta completa del pipe (prefijo /tmp/)
    char pipe_path[256];
    strcat(pipe_path, pipe_name);
    
    // Iniciar el receptor
    printf("[INFO] Iniciando receptor de peticiones...\n");
    iniciar_receptor(pipe_path, archivo_entrada, verbose, 
                     strlen(archivo_salida) > 0 ? archivo_salida : NULL);
    
    return 0;
}