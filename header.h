#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct fileHeader{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
}Header;

// Função que preenche os campos de um novo registro com valores nulos
void newHeader(Header *header);

// Função que escreve a struct header no arquivo binário
void writeHeader(Header *header, FILE* file);

// Função que lê o cabeçalho do arquivo binário e preenche a struct Header
void readHeader(Header *header, FILE* file);

#endif