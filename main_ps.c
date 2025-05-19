#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ps.h"

int main(int argc, char *argv[]) {
    char pipe_name[100] = "";
    char archivo_entrada[100] = "";
    int entrada_por_archivo = 0;

    // Procesamiento de argumentos
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-p") == 0 && i + 1 < argc)
            strcpy(pipe_name, argv[++i]);
        else if (strcmp(argv[i], "-i") == 0 && i + 1 < argc) {
            strcpy(archivo_entrada, argv[++i]);
            entrada_por_archivo = 1;
        }
    }

    if (strlen(pipe_name) == 0) {
        fprintf(stderr, "Uso: %s -p pipeReceptor [-i archivoSolicitudes]\n", argv[0]);
        fprintf(stderr, "En su lugar Use : %s [-i archivoSolicitudes] -p pipeReceptor\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if (entrada_por_archivo) {
        iniciar_ps_archivo(archivo_entrada, pipe_name);
    } else {
        iniciar_solicitante_menu(pipe_name);
    }

    return 0;
}
