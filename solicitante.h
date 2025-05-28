#ifndef SOLICITANTE_H
#define SOLICITANTE_H
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

void enviar_mensaje(const char *pipe_sol, const char *contenido);
void procesar_linea_archivo(const char *linea, const char *pipe_sol);

#endif // SOLICITANTE_H
