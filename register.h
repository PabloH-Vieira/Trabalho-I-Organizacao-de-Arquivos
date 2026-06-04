#ifndef REGISTER_H
#define REGISTER_H

#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    char nomeEstacao[100];
    int tamNomeLinha;
    char nomeLinha[100];
}Registro;

// Função que preenche os campos da struct Registro com os valores lidos do arquivo CSV
void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual);

// Função que escreve a struct Registro no arquivo binário
void writeRegistros(Registro *registro, FILE* saida, Header *cabecalho);

// Função que lê os campos de um registro do arquivo binário e preenche a struct Registro
int readRegistros(Registro *registro, FILE* file);

// Função que imprime os campos de um registro na tela
void printRegistros(Registro *registro);

// Função que preenche os campos de um registro com os valores fornecidos pelo usuário para inserção
void preencherNovoRegistro(Registro *novoReg);

// Função que atualiza os campos de um registro com os valores fornecidos pelo usuário para atualização
void updateRegistro(Registro *registro, char camposUpdate[][50], char valoresUpdate[][100], int p);

#endif