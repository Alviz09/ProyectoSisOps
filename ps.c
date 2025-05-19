#include "ps.h"
#include "Pipe.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

void enviar_operacion(int pipe_fd, char operacion, const char* titulo, int ISBN) {
    Mensaje msg;
    Respuesta resp;

    printf("[DEBUG] Preparando mensaje para enviar...\n");
    
    msg.operacion = operacion;
    strncpy(msg.titulo, titulo, 100);
    msg.titulo[99] = '\0';  // Asegurar null-termination
    msg.ISBN = ISBN;
    msg.pid = getpid();     // Añadir PID para que el servidor sepa a quién responder

    printf("[DEBUG] PID del proceso: %d\n", msg.pid);
    printf("[DEBUG] Enviando solicitud: %c, '%s', ISBN=%d\n", operacion, titulo, ISBN);
    
    if (!enviar_solicitud(pipe_fd, &msg)) {
        fprintf(stderr, "[ERROR] Error al enviar solicitud por pipe %d\n", pipe_fd);
        return;
    }
    printf("[DEBUG] Solicitud enviada correctamente\n");

    printf("[DEBUG] Esperando respuesta del servidor...\n");
    if (!recibir_respuesta(pipe_fd, &resp)) {
        fprintf(stderr, "[ERROR] Error al recibir respuesta por pipe %d\n", pipe_fd);
        return;
    }
    printf("[DEBUG] Respuesta recibida correctamente\n");

    if (resp.exito) {
        printf("[INFO] Operación exitosa: %s\n", resp.mensaje);
    } else {
        printf("[INFO] Operación fallida: %s\n", resp.mensaje);
    }
}

void iniciar_ps_menu(const char *pipe) {
    // Conectar al pipe
    int pipe_fd = conectar_a_pipe_receptor(pipe);
    if (pipe_fd == -1) {
        fprintf(stderr, "No se pudo conectar al pipe receptor\n");
        return;
    }

    int opcion;
    do {
        printf("\n--- Menú de Biblioteca ---\n");
        printf("1. Devolver un libro\n");
        printf("2. Renovar un libro\n");
        printf("3. Solicitar prestado un libro\n");
        printf("0. Salir\n");
        printf("Seleccione una opción: ");
        scanf("%d", &opcion);
        
        char titulo[100];
        int isbn;
        
        switch (opcion) {
            case 1:
                printf("Ingrese título del libro a devolver: ");
                scanf(" %99[^\n]", titulo);
                printf("Ingrese ISBN: ");
                scanf("%d", &isbn);
                enviar_operacion(pipe_fd, 'D', titulo, isbn);
                break;
            case 2:
                printf("Ingrese título del libro a renovar: ");
                scanf(" %99[^\n]", titulo);
                printf("Ingrese ISBN: ");
                scanf("%d", &isbn);
                enviar_operacion(pipe_fd, 'R', titulo, isbn);
                break;
            case 3:
                printf("Ingrese título del libro a solicitar: ");
                scanf(" %99[^\n]", titulo);
                printf("Ingrese ISBN: ");
                scanf("%d", &isbn);
                enviar_operacion(pipe_fd, 'P', titulo, isbn);
                break;
            case 0:
                printf("Enviando mensaje de salida...\n");
                enviar_operacion(pipe_fd, 'Q', "Salir", 0);
                printf("Saliendo del menú.\n");
                break;
            default:
                printf("Opción no válida. Intente de nuevo.\n");
        }
    } while (opcion != 0);

    // Cerrar el pipe al terminar
    cerrar_pipe(pipe_fd);
}

void iniciar_ps_archivo(const char *archivo, const char *pipe) {
    FILE *file = fopen(archivo, "r");
    if (!file) {
        perror("Error al abrir archivo de solicitudes");
        return;
    }

    // Conectar al pipe
    int pipe_fd = conectar_a_pipe_receptor(pipe);
    if (pipe_fd == -1) {
        fprintf(stderr, "No se pudo conectar al pipe receptor\n");
        fclose(file);
        return;
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), file)) {
        char operacion;
        char titulo[100];
        int isbn;

        // Eliminar salto de línea
        size_t len = strlen(linea);
        if (len > 0 && linea[len-1] == '\n')
            linea[len-1] = '\0';

        // Parsear la línea: Operación, nombre del libro, ISBN
        if (sscanf(linea, "%c, %99[^,], %d", &operacion, titulo, &isbn) == 3) {
            // Procesar la solicitud
            enviar_operacion(pipe_fd, operacion, titulo, isbn);
            
            // Si es una operación de salida, terminar
            if (operacion == 'Q') {
                printf("Comando de salida detectado en archivo. Terminando.\n");
                break;
            }
        } else {
            fprintf(stderr, "Formato incorrecto en línea: %s\n", linea);
        }
    }

    fclose(file);
    cerrar_pipe(pipe_fd);
}