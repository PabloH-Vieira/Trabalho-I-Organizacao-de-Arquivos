#ifndef BINARYTREE_H
#define BINARYTREE_H
#include <stdio.h>
#include <stdlib.h>

typedef struct binaryHeader{
    char status; // '0' para inconsistente, '1' para consistente
    int noRaiz; // RRN do nó raiz da árvore
    int topo; // RRN do topo da pilha de nós removidos
    int proxRRN; // Próximo RRN disponível para escrita de um novo nó
    int nroNos; // Número de nós atualmente na árvore
}binaryHeader;

typedef struct binaryNode{
    char removido; // '0' para nó removido, '1' para nó válido
    int proximo; // RRN do próximo nó na pilha de nós removidos (apenas para nós removidos)
    int tipoNo;
    int nroChaves; // Número de chaves atualmente no nó
    int chaves[3]; // Vetor de chaves (RRNs dos registros correspond
    int ponteiros[3]; // Vetor de ponteiros para os registros correspondentes às chaves
    int filhos[4]; // Vetor de RRN dos filhos (apenas para nós internos)
}binaryNode;

// Funções de manipulação da árvore binária
// função para ler o cabeçalho do arquivo binário
void readBinaryHeader(binaryHeader *header, FILE *file, int rrn);

// Função para criar um novo cabeçalho para o arquivo de índice
void createBinaryHeader(binaryHeader *header);

// função para escrever o cabeçalho no arquivo binário
void writeBinaryHeader(binaryHeader *header, FILE *file);

// função para ler um nó da árvore binária
int readBinaryNode(binaryNode *node, FILE *file, int rrn);

// função para escrever um nó da árvore binária
void writeBinaryNode(binaryNode *node, FILE *file, int rrn);

// função que cria um nó vazio
binaryNode* createEmptyBinaryNode();

#endif