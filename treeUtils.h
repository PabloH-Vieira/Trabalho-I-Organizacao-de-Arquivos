#ifndef TREEUTILS_H
#define TREEUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include "binaryTree.h"

#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4
#define MAX_CHAVES 3

// Insere uma chave e o RRN do registro correspondente na árvore
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header);

// Busca uma chave na árvore, retorna o RRN do registro ou -1 se não encontrar
int searchKey(FILE *file, int chave, binaryHeader *header);

// Remove uma chave da árvore tratando underflow, redistribuição e concatenação
void removeKey(FILE *file, int chave, binaryHeader *header);

#endif