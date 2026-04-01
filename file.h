#ifndef FILE_H
#define FILE_H
#include <stdio.h>

typedef struct fileHeader{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
}Header;

typedef struct{
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char nomeEstacao[21];
    int tamNomeLinha;
    char nomeLinha[21];
}Registro;

typedef struct file {
    Header header;
    FILE* file;
    Registro registros[];
}File;

void CreateTable(char* InputFile, char* OutPutFile);
void binarioNaTela(const char* filename);

#endif