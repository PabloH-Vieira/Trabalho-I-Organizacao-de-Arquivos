# Variables for compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -g
LIBS = 

# Source and object files
SOURCES = main.c file.c
OBJECTS = main.o file.o
TARGET = main

# Directory for output files
OUTPUT_DIR = output

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJECTS)
	@echo "Compilando e linkando..."
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	@echo "✓ Compilação concluída: $(TARGET)"

# Compile main.c
main.o: main.c file.h
	$(CC) $(CFLAGS) -c main.c

# Compile file.c
file.o: file.c file.h
	$(CC) $(CFLAGS) -c file.c

# Create output directory
$(OUTPUT_DIR):
	@mkdir -p $(OUTPUT_DIR)
	@echo "✓ Pasta $(OUTPUT_DIR) criada"

# Run the program with test input
run: all $(OUTPUT_DIR)
	@echo "Executando programa..."
	@echo "1\nestacoes.csv\n$(OUTPUT_DIR)/estacoes.bin\n2\n$(OUTPUT_DIR)/estacoes.bin" | ./$(TARGET)

# Compile only
build: all

# Clean build artifacts
clean:
	@echo "Limpando arquivos de compilação..."
	rm -f $(OBJECTS) $(TARGET)
	@echo "✓ Limpeza concluída"

# Clean everything including output directory
distclean: clean
	@echo "Removendo pasta $(OUTPUT_DIR)..."
	rm -rf $(OUTPUT_DIR)
	@echo "✓ Limpeza completa concluída"

# Rebuild everything
rebuild: clean all

# Help target
help:
	@echo "Targets disponíveis:"
	@echo "  make all      - Compila o projeto (padrão)"
	@echo "  make build    - Sinônimo de 'make all'"
	@echo "  make run      - Compila e executa o programa com teste"
	@echo "  make clean    - Remove arquivos de compilação"
	@echo "  make distclean- Remove arquivos e pasta output/"
	@echo "  make rebuild  - Limpa e recompila tudo"
	@echo "  make help     - Mostra esta mensagem"

# Phony targets (não são arquivos)
.PHONY: all build run clean distclean rebuild help
