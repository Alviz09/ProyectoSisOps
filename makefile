# Variables
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread

# Ejecutables
TARGETS = receptor solicitante

# Archivos fuente para cada programa
RECEPTOR_SOURCES = receptor.c
SOLICITANTE_SOURCES = solicitante.c

# Archivos objeto
RECEPTOR_OBJECTS = $(RECEPTOR_SOURCES:.c=.o)
SOLICITANTE_OBJECTS = $(SOLICITANTE_SOURCES:.c=.o)

# Regla principal - compila ambos programas
all: $(TARGETS)

# Compilar receptor
receptor: $(RECEPTOR_OBJECTS)
	$(CC) $(CFLAGS) $(RECEPTOR_OBJECTS) -o receptor

# Compilar solicitante
solicitante: $(SOLICITANTE_OBJECTS)
	$(CC) $(CFLAGS) $(SOLICITANTE_OBJECTS) -o solicitante

# Compilar archivos objeto
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpiar archivos generados
clean:
	rm -f *.o $(TARGETS)

# Reinstalar (limpiar y compilar)
rebuild: clean all


# Indicar que estas reglas no son archivos
.PHONY: all clean rebuild run-receptor run-solicitante
