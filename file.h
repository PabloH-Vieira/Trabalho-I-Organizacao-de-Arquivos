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
}File;

typedef struct{
    //Flags para indicar quais campos são critérios de busca
    int flag_codEstacao;
    int flag_nomeEstacao;
    int flag_codLinha;
    int flag_nomeLinha;
    int flag_codProxEstacao;
    int flag_distProxEstacao;
    int flag_codLinhaIntegra;
    int flag_codEstIntegra;

    //Valores dos campos a serem buscados, caso o campo seja um critério de busca
    Registro regBusca;
}CriteriosBusca;

void newHeader(Header *header);
void writeHeader(Header *header, FILE* file);
void readHeader(Header *header, FILE* file);
void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual);
void writeRegistros(Registro *registro, FILE* saida, Header *cabecalho);
int readRegistros(Registro *registro, FILE* file);
void printRegistros(Registro *registro);
FILE* CreateTable(char* InputFile, char* OutPutFile);
void Select(char *FileName);
void binarioNaTela(const char* filename);

#endif