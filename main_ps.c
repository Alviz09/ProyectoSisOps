#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ps.h"

/**
 * Procesos Solicitantes (PS)
 * 
 * Uso: solicitante [-i file] -p pipeReceptor
 * 
 * Donde:
 * -i file: Archivo de entrada con operaciones (opcional)
 * -p pipeReceptor: Nombre del pipe nominal para comunicación con el RP
 */

void mostrar_ayuda(const char *nombre_programa) {
    printf("Uso: %s [-i file] -p pipeReceptor\n\n", nombre_programa);
    printf("Donde:\n");
    printf("  -i file          Archivo con operaciones a realizar (opcional)\n");
    printf("  -p pipeReceptor  Nombre del pipe para comunicación con el Receptor\n\n");
    printf("Si no se especifica archivo de entrada, el programa mostrará un menú interactivo.\n");
    printf("Formato de archivo de entrada:\n");
    printf("  Operación, nombre del libro, ISBN\n");
    printf("  Donde Operación puede ser: D (devolución), R (renovación), P (préstamo), Q (salir)\n\n");
    printf("Ejemplo:\n");
    printf("  D, Alicia en el País de las Maravillas, 2133\n");
    printf("  R, Hamlet, 234\n");
    printf("  P, Cálculo Diferencial, 120\n");
    printf("  Q, Salir, 0\n");
}

int main(int argc, char *argv[]) {
    char pipe_name[100] = "";
    char archivo_entrada[100] = "";
    int entrada_por_archivo = 0;

    // Si no hay argumentos, mostrar ayuda
    if (argc < 2) {
        mostrar_ayuda(argv[0]);
        return 1;
    }

    // Procesamiento de argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc) {
            strcpy(pipe_name, argv[++i]);
        } else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strcpy(archivo_entrada, argv[++i]);
            entrada_por_archivo = 1;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            mostrar_ayuda(argv[0]);
            return 0;
        }
    }

    // Verificar que se ha especificado el nombre del pipe
    if (strlen(pipe_name) == 0) {
        fprintf(stderr, "ERROR: Debe especificar el nombre del pipe con -p\n\n");
        mostrar_ayuda(argv[0]);
        return EXIT_FAILURE;
    }

    // Construir la ruta completa del pipe
    char pipe_path[256];
    strcat(pipe_path, pipe_name);

    printf("=== PROCESO SOLICITANTE (PS) ===\n");
    printf("PID: %d\n", getpid());
    printf("Pipe: %s\n", pipe_path);
    
    if (entrada_por_archivo) {
        printf("Archivo de entrada: %s\n", archivo_entrada);
        printf("==============================\n\n");
        
        // Verificar que el archivo existe
        if (access(archivo_entrada, F_OK) != 0) {
            fprintf(stderr, "ERROR: El archivo de entrada '%s' no existe\n", archivo_entrada);
            return EXIT_FAILURE;
        }
        
        // Iniciar con entrada desde archivo
        iniciar_ps_archivo(archivo_entrada, pipe_path);
    } else {
        printf("Modo: Interactivo (menú)\n");
        printf("==============================\n\n");
        
        // Iniciar con menú interactivo
        iniciar_ps_menu(pipe_path);
    }

    printf("\nProceso Solicitante (PID: %d) finalizado.\n", getpid());
    return 0;
}