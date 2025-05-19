#include "rp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

// Variables globales para la Base de Datos
Libro libros[MAX_LIBROS];
int num_libros = 0;

// Función para buscar un libro por ISBN
int buscar_libro_por_isbn(int isbn) {
    for (int i = 0; i < num_libros; i++) {
        if (libros[i].ISBN == isbn) {
            return i; // Retorna el índice del libro
        }
    }
    return -1; // No encontrado
}

// Función para buscar un ejemplar disponible de un libro
int buscar_ejemplar_disponible(Libro *libro) {
    for (int i = 0; i < libro->num_ejemplares; i++) {
        if (libro->ejemplares_list[i].status == 'D') {
            return i; // Retorna el índice del ejemplar
        }
    }
    return -1; // No hay ejemplares disponibles
}

// Función para buscar un ejemplar prestado específico
int buscar_ejemplar_prestado(Libro *libro, int id_ejemplar) {
    for (int i = 0; i < libro->num_ejemplares; i++) {
        if (libro->ejemplares_list[i].id_ejemplar == id_ejemplar && 
            libro->ejemplares_list[i].status == 'P') {
            return i; // Retorna el índice del ejemplar
        }
    }
    return -1; // No encontrado o no está prestado
}

// Función para obtener la fecha actual + días adicionales
struct tm obtener_fecha_futura(int dias_adicionales) {
    time_t tiempo_actual;
    struct tm *tm_info;
    struct tm resultado;

    time(&tiempo_actual);
    tiempo_actual += dias_adicionales * 24 * 60 * 60; // Convertir días a segundos
    tm_info = localtime(&tiempo_actual);
    
    // Copiar los valores a una nueva estructura para evitar problemas
    memcpy(&resultado, tm_info, sizeof(struct tm));
    
    return resultado;
}

// Implementación de la función enviar_respuesta_a_cliente
void enviar_respuesta_a_cliente(int pipe_fd, const Mensaje *solicitud, Respuesta *respuesta) {
    if (!enviar_respuesta(pipe_fd, respuesta)) {
        fprintf(stderr, "[ERROR] Error al enviar respuesta al cliente (PID: %d)\n", solicitud->pid);
    } else if (respuesta->exito) {
        printf("[INFO] Respuesta exitosa enviada al cliente (PID: %d): %s\n", 
               solicitud->pid, respuesta->mensaje);
    } else {
        printf("[INFO] Respuesta de error enviada al cliente (PID: %d): %s\n", 
               solicitud->pid, respuesta->mensaje);
    }
}

// Implementación de la función manejar_devolucion
void manejar_devolucion(const Mensaje *msg, int verbose) {
    if (verbose) {
        printf("[INFO] Procesando devolución: '%s' (ISBN: %d) - PID: %d\n", 
               msg->titulo, msg->ISBN, msg->pid);
    }
    
    // Simular procesamiento
    int indice_libro = buscar_libro_por_isbn(msg->ISBN);
    
    // Preparar respuesta inmediata
    Respuesta respuesta;
    respuesta.exito = 1; // Siempre aceptamos devoluciones
    strcpy(respuesta.mensaje, "Devolución aceptada. Gracias por devolver el libro.");
    
    // Enviar respuesta
    int pipe_fd = conectar_a_pipe_receptor("pipeReceptor");  // Usar el mismo pipe receptor
    enviar_respuesta_a_cliente(pipe_fd, msg, &respuesta);
    cerrar_pipe(pipe_fd);
}

// Implementación de la función manejar_renovacion
void manejar_renovacion(const Mensaje *msg, int verbose) {
    if (verbose) {
        printf("[INFO] Procesando renovación: '%s' (ISBN: %d) - PID: %d\n", 
               msg->titulo, msg->ISBN, msg->pid);
    }
    
    // Simular procesamiento
    int indice_libro = buscar_libro_por_isbn(msg->ISBN);
    
    // Obtener fecha una semana después
    struct tm nueva_fecha = obtener_fecha_futura(7); // 7 días = 1 semana
    
    // Preparar respuesta inmediata con la nueva fecha
    Respuesta respuesta;
    respuesta.exito = 1; // Siempre aceptamos renovaciones
    sprintf(respuesta.mensaje, "Renovación aceptada. Nueva fecha de entrega: %02d-%02d-%04d",
            nueva_fecha.tm_mday, nueva_fecha.tm_mon + 1, nueva_fecha.tm_year + 1900);
    
    // Enviar respuesta
    int pipe_fd = conectar_a_pipe_receptor("pipeReceptor");  // Este pipe debería existir ya
    enviar_respuesta_a_cliente(pipe_fd, msg, &respuesta);
    cerrar_pipe(pipe_fd);
}

// Implementación de la función manejar_prestamo
void manejar_prestamo(const Mensaje *msg, int verbose) {
    if (verbose) {
        printf("[INFO] Procesando préstamo: '%s' (ISBN: %d) - PID: %d\n", 
               msg->titulo, msg->ISBN, msg->pid);
    }
    
    // Buscar el libro por ISBN
    int indice_libro = buscar_libro_por_isbn(msg->ISBN);
    
    // Preparar respuesta
    Respuesta respuesta;
    
    if (indice_libro == -1) {
        // El libro no existe
        respuesta.exito = 0;
        sprintf(respuesta.mensaje, "El libro con ISBN %d no existe en la biblioteca.", msg->ISBN);
    } else {
        // Verificar si hay ejemplares disponibles
        int indice_ejemplar = buscar_ejemplar_disponible(&libros[indice_libro]);
        
        if (indice_ejemplar == -1) {
            // No hay ejemplares disponibles
            respuesta.exito = 0;
            strcpy(respuesta.mensaje, "Lo sentimos, no hay ejemplares disponibles de este libro.");
        } else {
            // Hay ejemplar disponible, realizar el préstamo
            ejemplar *e = &libros[indice_libro].ejemplares_list[indice_ejemplar];
            
            // Cambiar estado a prestado
            e->status = 'P';
            
            // Establecer fecha de devolución (2 semanas por defecto)
            e->fecha = obtener_fecha_futura(14); // 14 días = 2 semanas
            
            respuesta.exito = 1;
            sprintf(respuesta.mensaje, "Préstamo realizado con éxito. Ejemplar %d. Fecha devolución: %02d-%02d-%04d", 
                    e->id_ejemplar, e->fecha.tm_mday, e->fecha.tm_mon + 1, e->fecha.tm_year + 1900);
        }
    }
    
    // Enviar respuesta al cliente
    int pipe_fd = conectar_a_pipe_receptor("pipeReceptor");  // Este pipe debería existir ya
    enviar_respuesta_a_cliente(pipe_fd, msg, &respuesta);
    cerrar_pipe(pipe_fd);
}

// Función para generar reporte de libros
void generar_reporte(int verbose) {
    printf("\n=== REPORTE DE BIBLIOTECA ===\n");
    printf("Status, Nombre del Libro, ISBN, ejemplar, fecha\n");
    
    for (int i = 0; i < num_libros; i++) {
        for (int j = 0; j < libros[i].num_ejemplares; j++) {
            ejemplar *e = &libros[i].ejemplares_list[j];
            printf("%c, %s, %d, %d, %02d-%02d-%04d\n",
                   e->status, libros[i].titulo, libros[i].ISBN, e->id_ejemplar,
                   e->fecha.tm_mday, e->fecha.tm_mon + 1, e->fecha.tm_year + 1900);
        }
    }
    
    printf("=========================\n\n");
}

// Función para guardar el estado de la BD al terminar
void guardar_estado_bd(const char *archivo_salida) {
    if (!archivo_salida || strlen(archivo_salida) == 0) {
        return;
    }
    
    FILE *archivo = fopen(archivo_salida, "w");
    if (!archivo) {
        fprintf(stderr, "[ERROR] No se pudo abrir el archivo %s para escritura\n", archivo_salida);
        return;
    }
    
    for (int i = 0; i < num_libros; i++) {
        // Escribir línea de libro: nombre, ISBN, número de ejemplares
        fprintf(archivo, "%s, %d, %d\n", libros[i].titulo, libros[i].ISBN, libros[i].num_ejemplares);
        
        // Escribir líneas de ejemplares: id, status, fecha
        for (int j = 0; j < libros[i].num_ejemplares; j++) {
            ejemplar *e = &libros[i].ejemplares_list[j];
            fprintf(archivo, "%d, %c, %02d-%02d-%04d\n",
                    e->id_ejemplar, e->status,
                    e->fecha.tm_mday, e->fecha.tm_mon + 1, e->fecha.tm_year + 1900);
        }
    }
    
    fclose(archivo);
    printf("[INFO] Estado final de la BD guardado en %s\n", archivo_salida);
}

// Versión simplificada de iniciar_receptor 
void iniciar_receptor(const char *pipe, const char *archivo_bd, int verbose, const char *archivo_salida) {
    // 1. Cargar la base de datos
    printf("[INFO] Cargando base de datos desde: %s\n", archivo_bd);
    num_libros = leerArchivo(libros, MAX_LIBROS, archivo_bd);
    
    if (num_libros <= 0) {
        fprintf(stderr, "[ERROR] No se pudieron cargar libros desde %s\n", archivo_bd);
        return;
    }
    
    printf("[INFO] Base de datos cargada con éxito: %d libros\n", num_libros);
    
    // 2. Crear el pipe para comunicación con PS
    int pipe_fd = crear_pipe_receptor(pipe);
    if (pipe_fd == -1) {
        fprintf(stderr, "[ERROR] No se pudo crear el pipe receptor %s\n", pipe);
        return;
    }
    
    printf("[INFO] Pipe receptor '%s' creado con éxito (fd=%d)\n", pipe, pipe_fd);
    
    // 3. Bucle principal para recibir solicitudes (
    printf("[INFO] Receptor listo para recibir solicitudes\n");
    int continuar = 1;
    
    while (continuar) {
        // Esperar conexión al pipe
        if (!esperar_conexion(pipe_fd)) {
            fprintf(stderr, "[ERROR] Error al esperar conexión en el pipe\n");
            continue;
        }
        
        // Recibir solicitud
        Mensaje solicitud;
        if (!recibir_solicitud(pipe_fd, &solicitud)) {
            fprintf(stderr, "[ERROR] Error al recibir solicitud\n");
            continue;
        }
        
        if (verbose) {
            printf("[INFO] Solicitud recibida: '%c', '%s', ISBN=%d, PID=%d\n", 
                   solicitud.operacion, solicitud.titulo, solicitud.ISBN, solicitud.pid);
        }
        
        // Procesar solicitud según tipo
        switch (solicitud.operacion) {
            case 'D': // Devolución
                manejar_devolucion(&solicitud, verbose);
                break;
                
            case 'R': // Renovación
                manejar_renovacion(&solicitud, verbose);
                break;
                
            case 'P': // Préstamo
                manejar_prestamo(&solicitud, verbose);
                break;
                
            case 'Q': // Salida de un PS
                if (verbose) {
                    printf("[INFO] Proceso solicitante (PID=%d) ha terminado\n", solicitud.pid);
                }
                
                // Enviar confirmación de recepción
                Respuesta resp;
                resp.exito = 1;
                strcpy(resp.mensaje, "Terminación recibida. Adiós.");
                
                int tmp_pipe_fd = conectar_a_pipe_receptor("/tmp/respuesta_tmp");
                enviar_respuesta_a_cliente(tmp_pipe_fd, &solicitud, &resp);
                cerrar_pipe(tmp_pipe_fd);
                break;
                
            default:
                if (verbose) {
                    printf("[WARN] Operación desconocida: '%c'\n", solicitud.operacion);
                }
                
                // Enviar error
                Respuesta resp_err;
                resp_err.exito = 0;
                strcpy(resp_err.mensaje, "Operación no reconocida");
                
                int tmp_pipe_fd = conectar_a_pipe_receptor("/tmp/respuesta_tmp");
                enviar_respuesta_a_cliente(tmp_pipe_fd, &solicitud, &resp_err);
                cerrar_pipe(tmp_pipe_fd);
        }
        
        // Aquí se implementaría la verificación para salir del bucle usando entrada manual
        // Por simplicidad, solo mostramos cómo se procesarían los comandos
        printf("\nPresione 's' para salir o 'r' para mostrar reporte (continuará automáticamente): ");
        fflush(stdout);
        
        fd_set fds;
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // Esperar 100ms
        
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        
        if (select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0) {
            char comando;
            if (read(STDIN_FILENO, &comando, 1) > 0) {
                if (comando == 's' || comando == 'S') {
                    continuar = 0;
                    printf("[INFO] Comando de salida recibido. Terminando...\n");
                } else if (comando == 'r' || comando == 'R') {
                    printf("[INFO] Generando reporte...\n");
                    generar_reporte(verbose);
                }
            }
        }
    }
    
    // 4. Guardar estado final de la BD si se especificó archivo de salida
    if (archivo_salida != NULL && strlen(archivo_salida) > 0) {
        guardar_estado_bd(archivo_salida);
    }
    
    // 5. Cerrar recursos
    close(pipe_fd);
    printf("[INFO] Receptor de peticiones finalizado\n");
}