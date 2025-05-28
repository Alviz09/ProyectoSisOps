#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <errno.h>
#include <signal.h>
#include <sys/select.h>
#include "solicitante.h"

// Timeout en segundos para esperar respuesta
#define TIMEOUT_SECONDS 10

void enviar_mensaje(const char *pipe_sol, const char *contenido) {
    // Crear un nombre único para el pipe de respuesta usando el PID
    char pipe_res[100];
    snprintf(pipe_res, sizeof(pipe_res), "/tmp/pipe_res_%d", getpid());

    // Eliminar pipe anterior si existe
    unlink(pipe_res);
    
    // Crear el pipe de respuesta
    if (mkfifo(pipe_res, 0666) == -1 && errno != EEXIST) {
        perror("Error al crear FIFO de respuesta");
        return;
    }

    // Armar el mensaje que contiene el nombre del pipe de respuesta y el contenido
    char buffer[200];
    snprintf(buffer, sizeof(buffer), "%s|%s", pipe_res, contenido);

    // Verificar si el pipe de solicitud existe
    struct stat st;
    if (stat(pipe_sol, &st) == -1) {
        printf("Error: El pipe de solicitud %s no existe. ¿Está ejecutándose el receptor?\n", pipe_sol);
        unlink(pipe_res);
        return;
    }

    // Enviar el mensaje al pipe de solicitud con timeout
    printf("Enviando mensaje: %s\n", contenido);
    int fd = open(pipe_sol, O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        if (errno == ENXIO) {
            printf("Error: No hay receptor escuchando en el pipe %s\n", pipe_sol);
        } else {
            perror("Error al abrir FIFO de solicitud");
        }
        unlink(pipe_res);
        return;
    }

    // Escribir el mensaje
    ssize_t bytes_written = write(fd, buffer, strlen(buffer) + 1);
    if (bytes_written == -1) {
        perror("Error al escribir en el pipe de solicitud");
        close(fd);
        unlink(pipe_res);
        return;
    }
    close(fd);
    printf("Mensaje enviado correctamente\n");

    // Si el mensaje es "salir", no esperar respuesta
    if (strcmp(contenido, "salir") == 0) {
        unlink(pipe_res);
        return;
    }

    // Esperar la respuesta con timeout
    printf("Esperando respuesta...\n");
    fd = open(pipe_res, O_RDONLY | O_NONBLOCK);
    if (fd == -1) {
        perror("Error al abrir FIFO de respuesta");
        unlink(pipe_res);
        return;
    }

    // Usar select para implementar timeout
    fd_set readfds;
    struct timeval timeout;
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = TIMEOUT_SECONDS;
    timeout.tv_usec = 0;

    int result = select(fd + 1, &readfds, NULL, NULL, &timeout);
    if (result == -1) {
        perror("Error en select");
    } else if (result == 0) {
        printf("Timeout: No se recibió respuesta en %d segundos\n", TIMEOUT_SECONDS);
    } else {
        // Hay datos disponibles para leer
        char respuesta[100];
        ssize_t n = read(fd, respuesta, sizeof(respuesta) - 1);
        if (n > 0) {
            respuesta[n] = '\0';
            printf("Respuesta del receptor: %s\n", respuesta);
        } else if (n == 0) {
            printf("El receptor cerró la conexión\n");
        } else {
            perror("Error al leer respuesta");
        }
    }

    close(fd);
    unlink(pipe_res);
    
    // Pequeña pausa entre mensajes en modo archivo
    usleep(100000); // 100ms
}

void procesar_linea_archivo(const char *linea, const char *pipe_sol) {
    char linea_copia[256];
    strcpy(linea_copia, linea);
    
    // Remover espacios al inicio y final
    char *inicio = linea_copia;
    while (*inicio && isspace(*inicio)) inicio++;
    
    char *final = inicio + strlen(inicio) - 1;
    while (final > inicio && isspace(*final)) final--;
    *(final + 1) = '\0';
    
    // Ignorar líneas vacías
    if (strlen(inicio) == 0) {
        return;
    }
    
    printf("\nProcesando línea: %s\n", inicio);

    char tipo, nombre[52], isbn[6];
    if (sscanf(inicio, "%c, %50[^,], %5s", &tipo, nombre, isbn) != 3) {
        printf("Error: Línea inválida - %s\n", inicio);
        printf("Formato esperado: TIPO, NOMBRE, ISBN\n");
        return;
    }

    // Limpiar espacios de nombre e isbn
    char *p = nombre;
    while (*p && isspace(*p)) p++;
    memmove(nombre, p, strlen(p) + 1);
    p = nombre + strlen(nombre) - 1;
    while (p > nombre && isspace(*p)) p--;
    *(p + 1) = '\0';

    p = isbn;
    while (*p && isspace(*p)) p++;
    memmove(isbn, p, strlen(p) + 1);
    p = isbn + strlen(isbn) - 1;
    while (p > isbn && isspace(*p)) p--;
    *(p + 1) = '\0';

    if (tipo == 'Q') {
        enviar_mensaje(pipe_sol, "salir");
        printf("Fin de operaciones desde archivo.\n");
        return;
    }

    char codigo;
    const char *operacion;
    switch (tipo) {
        case 'D': 
            codigo = 'D'; 
            operacion = "Devolver";
            break;
        case 'R': 
            codigo = 'R'; 
            operacion = "Renovar";
            break;
        case 'P': 
            codigo = 'S'; 
            operacion = "Solicitar";
            break;
        default:
            printf("Error: Operación desconocida: %c\n", tipo);
            printf("Operaciones válidas: D (Devolver), R (Renovar), P (Solicitar), Q (Salir)\n");
            return;
    }

    printf("Operación: %s libro '%s' (ISBN: %s)\n", operacion, nombre, isbn);

    char buffer[100];
    snprintf(buffer, sizeof(buffer), "%c/%s/%s", codigo, nombre, isbn);
    enviar_mensaje(pipe_sol, buffer);
}

int main(int argc, char *argv[]) {
    char *archivo = NULL;
    char *pipe_sol = NULL;
    char *pipe_sol_default = "/home/user/proy/mipipe_sol";

    // Procesar argumentos
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "-i") == 0) {
            if (i + 1 < argc) {
                archivo = argv[i + 1];
                i += 2;
            } else {
                fprintf(stderr, "Error: La opción -i requiere un nombre de archivo\n");
                fprintf(stderr, "Uso: %s [-i archivo] [-p pipeSolicitud]\n", argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                pipe_sol = argv[i + 1];
                i += 2;
            } else {
                fprintf(stderr, "Error: La opción -p requiere un nombre de pipe\n");
                fprintf(stderr, "Uso: %s [-i archivo] [-p pipeSolicitud]\n", argv[0]);
                return 1;
            }
        } else {
            fprintf(stderr, "Error: Argumento no reconocido: %s\n", argv[i]);
            fprintf(stderr, "Uso: %s [-i archivo] [-p pipeSolicitud]\n", argv[0]);
            return 1;
        }
    }

    // Validaciones
    if (archivo && pipe_sol == NULL) {
        fprintf(stderr, "Error: Debe especificar el pipe de solicitud con -p cuando se usa -i\n");
        fprintf(stderr, "Uso: %s [-i archivo] [-p pipeSolicitud]\n", argv[0]);
        return 1;
    }

    if (!archivo && pipe_sol == NULL) {
        pipe_sol = pipe_sol_default;
        printf("=== MODO INTERACTIVO ===\n");
        printf("Usando pipe por defecto: %s\n", pipe_sol);
        printf("========================\n\n");
    } else if (archivo) {
        printf("=== MODO ARCHIVO ===\n");
        printf("Archivo de entrada: %s\n", archivo);
        printf("Pipe de solicitud: %s\n", pipe_sol);
        printf("====================\n\n");
    }

    // Crear el pipe de solicitud si no existe
    if (mkfifo(pipe_sol, 0666) == -1 && errno != EEXIST) {
        perror("Error al crear pipe de solicitud");
    }

    if (archivo) {
        // Modo archivo
        FILE *fp = fopen(archivo, "r");
        if (!fp) {
            perror("No se pudo abrir el archivo");
            return 1;
        }

        char linea[256];
        int numero_linea = 0;
        while (fgets(linea, sizeof(linea), fp)) {
            numero_linea++;
            linea[strcspn(linea, "\n")] = '\0';
            
            printf("\n--- Línea %d ---\n", numero_linea);
            procesar_linea_archivo(linea, pipe_sol);
            
            // Si encontramos 'Q', salir del bucle
            if (strlen(linea) > 0 && linea[0] == 'Q') {
                break;
            }
        }
        fclose(fp);
        printf("\nProcesamiento de archivo completado.\n");
    } else {
        // Modo interactivo
        int opcion;
        while (1) {
            printf("\nMenú de Biblioteca:\n");
            printf("1. Devolver un libro\n");
            printf("2. Renovar un libro\n");
            printf("3. Solicitar prestado un libro\n");
            printf("4. Salir\n");
            printf("Seleccione una opción: ");

            if (scanf("%d", &opcion) != 1) {
                printf("Entrada inválida.\n");
                while (getchar() != '\n');
                continue;
            }
            getchar(); // limpiar '\n'

            if (opcion == 4) {
                enviar_mensaje(pipe_sol, "salir");
                break;
            }

            char nombre[52], isbn[6];
            printf("Ingrese el nombre del libro: ");
            if (!fgets(nombre, sizeof(nombre), stdin)) {
                printf("Error al leer entrada\n");
                continue;
            }
            nombre[strcspn(nombre, "\n")] = '\0';

            printf("Ingrese el ISBN: ");
            if (!fgets(isbn, sizeof(isbn), stdin)) {
                printf("Error al leer entrada\n");
                continue;
            }
            isbn[strcspn(isbn, "\n")] = '\0';

            char tipo;
            switch (opcion) {
                case 1: tipo = 'D'; break;
                case 2: tipo = 'R'; break;
                case 3: tipo = 'S'; break;
                default:
                    printf("Opción inválida.\n");
                    continue;
            }

            char buffer[100];
            snprintf(buffer, sizeof(buffer), "%c/%s/%s", tipo, nombre, isbn);
            enviar_mensaje(pipe_sol, buffer);
        }
    }

    return 0;
}



