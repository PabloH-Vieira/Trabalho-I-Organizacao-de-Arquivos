#include "header.h"

void newHeader(Header *header){
    header->status = '0';
    header->topo = -1;
    header->proxRRN = 0;
    header->nroEstacoes = 0;
    header->nroParesEstacao = 0;
}

void writeHeader(Header *header, FILE *file){
    fseek(file, 0, SEEK_SET);
    // Escreve o cabeçalho no arquivo campo a campo para evitar o padding
    fwrite(&header->status, sizeof(char), 1, file);
    fwrite(&header->topo, sizeof(int), 1, file);
    fwrite(&header->proxRRN, sizeof(int), 1, file);
    fwrite(&header->nroEstacoes, sizeof(int), 1, file);
    fwrite(&header->nroParesEstacao, sizeof(int), 1, file);
}

void readHeader(Header *header, FILE *file){
    fseek(file, 0, SEEK_SET);
    fread(&header->status, sizeof(char), 1, file);
    fread(&header->topo, sizeof(int), 1, file);
    fread(&header->proxRRN, sizeof(int), 1, file);
    fread(&header->nroEstacoes, sizeof(int), 1, file);
    fread(&header->nroParesEstacao, sizeof(int), 1, file);
}