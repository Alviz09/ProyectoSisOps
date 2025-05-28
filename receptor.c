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
#include "receptor.h"



#define N 10 // Tamaño del buffer (puede modificarse según necesidad)

int terminar = 0;

// Variables globales para los argumentos
char pipe_receptor[256] = "";
char archivo_datos[256] = "";
char archivo_salida[256] = "";
int verbose = 0;


// Buffer compartido como array de bytes para evitar problemas de tipo
char buffer_bytes[N * sizeof(Solicitud)];
int contador = 0;      // Contador de elementos en el buffer
int pos_entrada = 0;   // Posición de entrada al buffer
int pos_salida = 0;    // Posición de salida del buffer

// Mutex y variables de condición para sincronización
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t no_lleno = PTHREAD_COND_INITIALIZER;
pthread_cond_t no_vacio = PTHREAD_COND_INITIALIZER;

void mostrar_uso(const char *nombre_programa) {
    printf("Uso:\n");
    printf("  Modo interactivo: %s\n", nombre_programa);
    printf("  Modo con flags:   %s –p pipeReceptor –f filedatos [-v] [–s filesalida]\n", nombre_programa);
    printf("\nOpciones:\n");
    printf("  –p pipeReceptor  : Nombre del pipe nominal para comunicación entre procesos\n");
    printf("  –f filedatos     : Nombre del archivo donde se encuentra la BD inicial\n");
    printf("  –v               : Modo verbose (muestra todas las solicitudes en consola)\n");
    printf("  –s filesalida    : Archivo donde se escribirá el estado final de la BD\n");
    printf("\nEjemplos:\n");
    printf("  %s\n", nombre_programa);
    printf("  %s –p mipipe_sol –f bd.txt\n", nombre_programa);
    printf("  %s –p mipipe_sol –f bd.txt –v –s estado_final.txt\n", nombre_programa);
}

// Función para detectar el modo de operación
int detectar_modo(int argc, char *argv[]) {
    // Si solo se ejecuta ./receptor sin argumentos -> modo interactivo
    if (argc == 1) {
        return 0; // Modo interactivo
    }
    
    // Si hay argumentos -> modo archivo
    return 1; // Modo archivo
}

// Función para configurar modo interactivo (valores por defecto)
void configurar_modo_interactivo() {
    strcpy(pipe_receptor, "/home/user/proy/mipipe_sol");
    strcpy(archivo_datos, "/home/user/proy/bd.txt");
    strcpy(archivo_salida, ""); // Sin archivo de salida por defecto
    verbose = 0; // Sin verbose por defecto
    
    printf("=== MODO INTERACTIVO ACTIVADO ===\n");
    printf("Configuración por defecto:\n");
    printf("  Pipe: %s\n", pipe_receptor);
    printf("  Archivo BD: %s\n", archivo_datos);
    printf("  Verbose: DESACTIVADO\n");
    printf("  Archivo salida: NO ESPECIFICADO\n");
    printf("================================\n\n");
}

// Función para procesar argumentos de línea de comandos (modo archivo)
int procesar_argumentos(int argc, char *argv[]) {
    // Inicializar variables
    strcpy(pipe_receptor, "");
    strcpy(archivo_datos, "");
    strcpy(archivo_salida, "");
    verbose = 0;
    
    // Verificar que hay argumentos suficientes (mínimo -p pipe -f archivo)
    if (argc < 5) {
        printf("Error: Se requieren al menos los parámetros –p y –f\n");
        mostrar_uso(argv[0]);
        return -1;
    }
    
    // Procesar argumentos manualmente para manejar el formato específico
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "–p") == 0 || strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) {
                strcpy(pipe_receptor, argv[i + 1]);
                i++; // Saltar el siguiente argumento ya que es el valor de -p
            } else {
                printf("Error: La opción –p requiere un argumento\n");
                mostrar_uso(argv[0]);
                return -1;
            }
        }
        else if (strcmp(argv[i], "–f") == 0 || strcmp(argv[i], "-f") == 0) {
            if (i + 1 < argc) {
                strcpy(archivo_datos, argv[i + 1]);
                i++; // Saltar el siguiente argumento ya que es el valor de -f
            } else {
                printf("Error: La opción –f requiere un argumento\n");
                mostrar_uso(argv[0]);
                return -1;
            }
        }
        else if (strcmp(argv[i], "–v") == 0 || strcmp(argv[i], "-v") == 0) {
            verbose = 1;
        }
        else if (strcmp(argv[i], "–s") == 0 || strcmp(argv[i], "-s") == 0) {
            if (i + 1 < argc) {
                strcpy(archivo_salida, argv[i + 1]);
                i++; // Saltar el siguiente argumento ya que es el valor de -s
            } else {
                printf("Error: La opción –s requiere un argumento\n");
                mostrar_uso(argv[0]);
                return -1;
            }
        }
        else {
            // Argumento no reconocido, podría ser un valor de una opción anterior
            // No hacer nada aquí ya que se maneja en los casos anteriores
        }
    }
    
    // Verificar que tenemos los argumentos obligatorios
    if (strlen(pipe_receptor) == 0) {
        printf("Error: Se requiere especificar el pipe con –p pipeReceptor\n");
        mostrar_uso(argv[0]);
        return -1;
    }
    
    if (strlen(archivo_datos) == 0) {
        printf("Error: Se requiere especificar el archivo de datos con –f filedatos\n");
        mostrar_uso(argv[0]);
        return -1;
    }
    
    // Verificar que el archivo de datos existe
    if (access(archivo_datos, F_OK) != 0) {
        printf("Error: El archivo de datos '%s' no existe\n", archivo_datos);
        return -1;
    }
    
    printf("=== MODO ARCHIVO ACTIVADO ===\n");
    printf("Configuración:\n");
    printf("  Pipe: %s\n", pipe_receptor);
    printf("  Archivo BD: %s\n", archivo_datos);
    printf("  Verbose: %s\n", verbose ? "ACTIVADO" : "DESACTIVADO");
    printf("  Archivo salida: %s\n", strlen(archivo_salida) > 0 ? archivo_salida : "NO ESPECIFICADO");
    printf("============================\n\n");
    
    return 0;
}

// Función corregida para escribir el estado final de la BD
void escribir_estado_final() {
    if (strlen(archivo_salida) == 0) {
        printf("No se especificó archivo de salida, no se generará copia final.\n");
        return;
    }
    
    // Primero verificar que el archivo de datos existe
    if (access(archivo_datos, F_OK) != 0) {
        printf("Error: El archivo de datos '%s' no existe para generar la copia final\n", archivo_datos);
        return;
    }
    
    FILE *fp_entrada = fopen(archivo_datos, "r");
    if (!fp_entrada) {
        printf("Error: No se pudo abrir el archivo de datos '%s' para generar la copia final\n", archivo_datos);
        perror("Detalles del error");
        return;
    }
    
    FILE *fp_salida = fopen(archivo_salida, "w");
    if (!fp_salida) {
        printf("Error: No se pudo crear el archivo de salida '%s'\n", archivo_salida);
        perror("Detalles del error");
        fclose(fp_entrada);
        return;
    }
    
    printf("Copiando estado final de '%s' a '%s'...\n", archivo_datos, archivo_salida);
    
    // Copiar el contenido byte por byte para mayor seguridad
    int ch;
    int lineas_copiadas = 0;
    int bytes_copiados = 0;
    
    while ((ch = fgetc(fp_entrada)) != EOF) {
        if (fputc(ch, fp_salida) == EOF) {
            printf("Error escribiendo en el archivo de salida\n");
            fclose(fp_entrada);
            fclose(fp_salida);
            return;
        }
        bytes_copiados++;
        if (ch == '\n') {
            lineas_copiadas++;
        }
    }
    
    // Asegurar que se escriban los datos al disco
    fflush(fp_salida);
    
    fclose(fp_entrada);
    fclose(fp_salida);
    
    printf("Estado final de la BD copiado exitosamente a: %s\n", archivo_salida);
    printf("Estadísticas de copia: %d bytes, %d líneas\n", bytes_copiados, lineas_copiadas);
    
    // Verificar que el archivo se creó correctamente
    struct stat st;
    if (stat(archivo_salida, &st) == 0) {
        printf("Archivo de salida creado: tamaño %ld bytes\n", st.st_size);
    } else {
        printf("Advertencia: No se pudo verificar el archivo de salida\n");
    }
}

void generar_reporte() {
    FILE *fp = fopen(archivo_datos, "r");
    if (!fp) {
        printf("Error abriendo archivo de base de datos: %s\n", archivo_datos);
        return;
    }

    printf("\n----- REPORTE ACTUAL DE LA BASE DE DATOS -----\n");

    char linea[256];
    while (fgets(linea, sizeof(linea), fp)) {
        printf("%s", linea);
    }

    printf("----- FIN DEL REPORTE -----\n\n");
    fclose(fp);
}

// Función para colocar una solicitud en el buffer
void poner_en_buffer(Solicitud *solicitud) {
    // Calcular la posición de memoria exacta
    char *destino = buffer_bytes + (pos_entrada * sizeof(Solicitud));
    
    // Copiar la estructura al buffer
    memcpy(destino, solicitud, sizeof(Solicitud));
}

// Función para obtener una solicitud del buffer
void obtener_del_buffer(Solicitud *solicitud) {
    // Calcular la posición de memoria exacta
    char *origen = buffer_bytes + (pos_salida * sizeof(Solicitud));
    
    // Copiar desde el buffer a la estructura
    memcpy(solicitud, origen, sizeof(Solicitud));
}

// Función para procesar las solicitudes
char* procesar_solicitud(Solicitud* solicitud) {
    static char resultado[100];
    
    // Obtener la fecha actual + 7 días
    time_t t = time(NULL);
    struct tm fecha_actual = *localtime(&t);
    fecha_actual.tm_mday += 7;
    mktime(&fecha_actual);

    char nueva_fecha[32];
    strftime(nueva_fecha, sizeof(nueva_fecha), "%d-%m-%Y", &fecha_actual);

    FILE *fp = fopen(archivo_datos, "r");
    if (!fp) {
        printf("Error abriendo archivo: %s\n", archivo_datos);
        strcpy(resultado, "Error abriendo archivo");
        return resultado;
    }

    char archivo_temp[300];
    snprintf(archivo_temp, sizeof(archivo_temp), "%s.tmp", archivo_datos);
    
    FILE *temp_fp = fopen(archivo_temp, "w");
    if (!temp_fp) {
        printf("Error abriendo archivo temporal: %s\n", archivo_temp);
        fclose(fp);
        strcpy(resultado, "Error abriendo archivo temporal");
        return resultado;
    }

    char line[256];
    int found = 0, dentro_libro = 0, accion_realizada = 0, dummy;
    char nombre_linea[51], isbn_linea[5];

    while (fgets(line, sizeof(line), fp)) {
        if (strchr(line, '-') == NULL &&
            sscanf(line, "%50[^,], %4[^,], %d", nombre_linea, isbn_linea, &dummy) == 3) {
            dentro_libro = (strcmp(nombre_linea, solicitud->nombre) == 0 &&
                            strcmp(isbn_linea, solicitud->isbn) == 0);
            fprintf(temp_fp, "%s", line);
            continue;
        }

        if (dentro_libro && !accion_realizada) {
            int id;
            char estado, fecha[32];

            if (sscanf(line, "%d, %c, %31[^\n]", &id, &estado, fecha) == 3) {
                if (strcmp(solicitud->accion, "S") == 0 && estado == 'D') {
                    fprintf(temp_fp, "%d, P, %s\n", id, nueva_fecha);
                    accion_realizada = 1; found = 1;
                    continue;
                } else if (strcmp(solicitud->accion, "D") == 0 && estado == 'P') {
                    fprintf(temp_fp, "%d, D, %s\n", id, nueva_fecha);
                    accion_realizada = 1; found = 1;
                    continue;
                } else if (strcmp(solicitud->accion, "R") == 0 && estado == 'P') {
                    fprintf(temp_fp, "%d, P, %s\n", id, nueva_fecha);
                    accion_realizada = 1; found = 1;
                    continue;
                }
            }
        }

        fprintf(temp_fp, "%s", line);
    }

    fclose(fp);
    fclose(temp_fp);

    if (found) {
        rename(archivo_temp, archivo_datos);
        snprintf(resultado, sizeof(resultado),
                 "Acción '%s' realizada con éxito para '%s'.", solicitud->accion, solicitud->nombre);
    } else {
        remove(archivo_temp);
        snprintf(resultado, sizeof(resultado),
                 "No se pudo realizar la acción '%s' para '%s'.", solicitud->accion, solicitud->nombre);
    }

    return resultado;
}

// Función para enviar la respuesta al cliente
void enviar_respuesta(const char* pipe_respuesta, const char* resultado) {
    int fd = open(pipe_respuesta, O_WRONLY | O_NONBLOCK);
    if (fd == -1) {
        printf("Error al abrir pipe de respuesta: %s\n", pipe_respuesta);
        return;
    }

    write(fd, resultado, strlen(resultado));
    close(fd);
    
    if (verbose) {
        printf("Respuesta enviada a: %s -> %s\n", pipe_respuesta, resultado);
    }
}

// Función para el hilo auxiliar (consumidor)
void* hilo_consumidor(void *arg) {
    if (verbose) {
        printf("Hilo auxiliar (consumidor) iniciado\n");
    }
    
    while (!terminar) {
        pthread_mutex_lock(&mutex);
        
        // Esperar si el buffer está vacío
        while (contador == 0 && !terminar) {
            pthread_cond_wait(&no_vacio, &mutex);
        }
        
        if (terminar) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        
        // Tomar una solicitud del buffer
        Solicitud solicitud;
        obtener_del_buffer(&solicitud);
        pos_salida = (pos_salida + 1) % N;
        contador--;
        
        // Señalizar que el buffer no está lleno
        pthread_cond_signal(&no_lleno);
        pthread_mutex_unlock(&mutex);
        
        // Procesar solo operaciones de devolución o renovación
        if (strcmp(solicitud.accion, "D") == 0 || strcmp(solicitud.accion, "R") == 0) {
            if (verbose) {
                printf("Consumidor: Procesando %s para '%s'\n", 
                       (strcmp(solicitud.accion, "D") == 0) ? "devolución" : "renovación", 
                       solicitud.nombre);
            }
            
            // Procesar la solicitud
            char* resultado = procesar_solicitud(&solicitud);
            
            // Enviar la respuesta
            enviar_respuesta(solicitud.pipe_respuesta, resultado);
        }
    }
    
    if (verbose) {
        printf("Hilo auxiliar (consumidor) terminado\n");
    }
    pthread_exit(NULL);
}

void* escuchar_comandos(void *arg) {
    char comando[10];

    printf("Ingrese comando: 's' para salir o 'r' para generar reporte\n");
    
    while (!terminar) { 
        
        // Usar select para timeout en la entrada
        fd_set readfds;
        struct timeval timeout;
        
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        
        timeout.tv_sec = 1;  // 1 segundo de timeout
        timeout.tv_usec = 0;
        
        int result = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);
        
        if (result > 0 && FD_ISSET(STDIN_FILENO, &readfds)) {
            if (!fgets(comando, sizeof(comando), stdin)) continue;
            
            // Limpiar el buffer de entrada
            comando[strcspn(comando, "\n")] = '\0';
            
            if (comando[0] == 's' || strcmp(comando, "salir") == 0) {
                printf("Comando de salida recibido. Cerrando el sistema...\n");
                terminar = 1;
                
                // Despertar al consumidor si está esperando
                pthread_mutex_lock(&mutex);
                pthread_cond_broadcast(&no_vacio); // Usar broadcast para despertar todos
                pthread_cond_broadcast(&no_lleno);
                pthread_mutex_unlock(&mutex);
                exit(0);
            } else if (comando[0] == 'r' || strcmp(comando, "reporte") == 0) {
                generar_reporte();
            } else if (strlen(comando) > 0) {
                printf("Comando no reconocido. Use 's' para salir o 'r' para reporte.\n");
            }
        }
        
        // Verificar periódicamente si hay que terminar
        if (terminar) {
            break;
        }
    }
    
    printf("Hilo de comandos terminando...\n");
    return NULL;
  
}

int main(int argc, char *argv[]) {
    int modo = detectar_modo(argc, argv);
    
    if (modo == 0) {
        // Modo interactivo - configuración por defecto
        configurar_modo_interactivo();
    } else {
        // Modo archivo - procesar argumentos
        if (procesar_argumentos(argc, argv) != 0) {
            return 1;
        }
    }
    
    // Eliminar pipe anterior si existe para evitar conflictos
    unlink(pipe_receptor);
    
    // Crear el pipe nominal
    if (mkfifo(pipe_receptor, 0666) == -1) {
        perror("Error creando pipe");
        return 1;
    }
    
    printf("Pipe creado correctamente: %s\n", pipe_receptor);

    // *** SOLUCIÓN MÁS SIMPLE - ESTAS 3 LÍNEAS SOLUCIONAN EL PROBLEMA ***
    // Abrir el pipe para escritura de forma no bloqueante para evitar bloqueo
    int keep_alive_fd = open(pipe_receptor, O_WRONLY | O_NONBLOCK);
    // No cerrar este descriptor, se mantiene abierto durante toda la ejecución
    if (keep_alive_fd != -1) {
        printf("Pipe preparado para recibir conexiones (keep-alive establecido)\n");
    }

    // Iniciar hilo para escuchar comandos
    pthread_t hilo_comandos;
    pthread_create(&hilo_comandos, NULL, escuchar_comandos, NULL);
    
    // Iniciar hilo auxiliar (consumidor)
    pthread_t hilo_auxiliar;
    pthread_create(&hilo_auxiliar, NULL, hilo_consumidor, NULL);

    printf("Receptor iniciado. Esperando solicitudes en pipe: %s\n", pipe_receptor);

    // Hilo principal actúa como productor
    while (!terminar) {
        if (verbose) {
            printf("DEBUG: Intentando abrir pipe para lectura: %s\n", pipe_receptor);
        }
        
        int fd = open(pipe_receptor, O_RDONLY);
        if (fd == -1) {
            if (!terminar) {  // Solo mostrar error si no estamos terminando
                printf("Error al abrir FIFO para lectura: %s\n", pipe_receptor);
                perror("Detalles del error");
            }
            continue;
        }

        if (verbose) {
            printf("DEBUG: Pipe abierto correctamente, esperando datos...\n");
        }

        char buffer[256];
        ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
        close(fd);

        if (bytes_read <= 0) {
            if (verbose && !terminar) {
                printf("DEBUG: No se recibieron datos o error en lectura (bytes: %ld)\n", bytes_read);
            }
            continue;
        }

        buffer[bytes_read] = '\0';
        
        // Limpiar caracteres de nueva línea al final
        char *newline = strchr(buffer, '\n');
        if (newline) *newline = '\0';
        
        if (verbose) {
            printf("DEBUG: Mensaje recibido (raw): '%s'\n", buffer);
        }
        
        // Manejar diferentes formatos de mensaje
        char pipe_respuesta[100] = "";
        char contenido[200] = "";
        
        // Buscar el separador '|' 
        char *separador = strchr(buffer, '|');
        
        if (separador != NULL) {
            // Formato: pipe_respuesta|accion/nombre/isbn
            *separador = '\0';
            strncpy(pipe_respuesta, buffer, sizeof(pipe_respuesta) - 1);
            pipe_respuesta[sizeof(pipe_respuesta) - 1] = '\0';
            strncpy(contenido, separador + 1, sizeof(contenido) - 1);
            contenido[sizeof(contenido) - 1] = '\0';
        } else {
            // Formato simple: accion/nombre/isbn (para compatibilidad)
            strncpy(contenido, buffer, sizeof(contenido) - 1);
            contenido[sizeof(contenido) - 1] = '\0';
            strcpy(pipe_respuesta, "/tmp/respuesta_default"); // Pipe por defecto
        }

        if (verbose) {
            printf("DEBUG: Pipe respuesta: '%s'\n", pipe_respuesta);
            printf("DEBUG: Contenido: '%s'\n", contenido);
        }

        // Salir si recibe "salir"
        if (strcmp(contenido, "salir") == 0) {
            printf("Un solicitante ha decidido cerrar el programa\n");
            continue;
        }
        
        // Parsear el mensaje de solicitud
        Solicitud solicitud;
        memset(&solicitud, 0, sizeof(solicitud));
        
        // Guardar el pipe de respuesta
        strncpy(solicitud.pipe_respuesta, pipe_respuesta, sizeof(solicitud.pipe_respuesta) - 1);
        solicitud.pipe_respuesta[sizeof(solicitud.pipe_respuesta) - 1] = '\0';

        // Extraer campos de la solicitud
        char contenido_copia[200];
        strncpy(contenido_copia, contenido, sizeof(contenido_copia) - 1);
        contenido_copia[sizeof(contenido_copia) - 1] = '\0';
        
        char *token = strtok(contenido_copia, "/");
        if (token) {
            strncpy(solicitud.accion, token, sizeof(solicitud.accion) - 1);
            solicitud.accion[sizeof(solicitud.accion) - 1] = '\0';
        }
        token = strtok(NULL, "/");
        if (token) {
            strncpy(solicitud.nombre, token, sizeof(solicitud.nombre) - 1);
            solicitud.nombre[sizeof(solicitud.nombre) - 1] = '\0';
        }
        token = strtok(NULL, "/");
        if (token) {
            strncpy(solicitud.isbn, token, sizeof(solicitud.isbn) - 1);
            solicitud.isbn[sizeof(solicitud.isbn) - 1] = '\0';
        }

        // Verificar si es un comando de control
        if (strcmp(solicitud.accion, "X") == 0 && terminar) {
            break;
        }

        if (verbose) {
            printf("Solicitud procesada:\n");
            printf("  Acción: %s\n", solicitud.accion);
            printf("  Nombre: %s\n", solicitud.nombre);
            printf("  ISBN: %s\n", solicitud.isbn);
            printf("  Pipe respuesta: %s\n", solicitud.pipe_respuesta);
        }
               
        // Implementación del patrón Productor/Consumidor
        if (strcmp(solicitud.accion, "D") == 0 || strcmp(solicitud.accion, "R") == 0) {
            // Devoluciones y renovaciones van al hilo auxiliar
            pthread_mutex_lock(&mutex);
            
            // Esperar si el buffer está lleno
            while (contador == N && !terminar) {
                pthread_cond_wait(&no_lleno, &mutex);
            }
            
            if (terminar) {
                pthread_mutex_unlock(&mutex);
                break;
            }
            
            // Colocar la solicitud en el buffer
            poner_en_buffer(&solicitud);
            pos_entrada = (pos_entrada + 1) % N;
            contador++;
            
            if (verbose) {
                printf("Productor: Añadida solicitud de %s para '%s' al buffer. Contador: %d\n", 
                       (strcmp(solicitud.accion, "D") == 0) ? "devolución" : "renovación", 
                       solicitud.nombre, contador);
            }
            
            // Señalizar que el buffer no está vacío
            pthread_cond_signal(&no_vacio);
            pthread_mutex_unlock(&mutex);
        } else {
            // Solicitudes de préstamo (S) las maneja directamente el hilo principal
            if (verbose) {
                printf("Procesando solicitud de préstamo para '%s' directamente\n", solicitud.nombre);
            }
            char* resultado = procesar_solicitud(&solicitud);
            enviar_respuesta(solicitud.pipe_respuesta, resultado);
        }
    }

    printf("Finalizando programa...\n");
    
    // Esperar a que terminen los hilos
    pthread_join(hilo_comandos, NULL);
    pthread_join(hilo_auxiliar, NULL);
    
    // *** CERRAR EL DESCRIPTOR KEEP-ALIVE ***
    if (keep_alive_fd != -1) {
        close(keep_alive_fd);
        printf("Descriptor keep-alive cerrado\n");
    }
    
    printf("Hilos terminados. Generando archivo de salida...\n");
    
    // Escribir estado final si se especificó archivo de salida
    escribir_estado_final();
    
    // Limpiar el pipe
    unlink(pipe_receptor);
    
    // Liberar recursos
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&no_lleno);
    pthread_cond_destroy(&no_vacio);
    
    printf("Receptor terminado correctamente.\n");
    
    return 0;
}
