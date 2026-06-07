#ifndef TREEUTILS_H
#define TREEUTILS_H
#include "binaryTree.h"

// Função para promover um nó filho para a posição de um nó intermediário
void promoteNode(FILE *file, binaryNode *node, int posicaoNode, int posicaoFilhoPromovido, binaryHeader *header);

// Função para inserir uma nova chave no arquivo de índice
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header);

// Função que divide as chaves de um nó
void splitNode(FILE *file, binaryNode *node, int posicaoNode, int rrnRegistro, binaryHeader *header);

// Função que busca uma chave na árvore binária
int searchKey(FILE *file, int chave, binaryHeader *header);

// Função que remove uma chave da árvore binária
void removeKey(FILE *file, int chave, binaryHeader *header);

// Função que trata o underflow
void handleUnderflow(FILE *file, int posicaoNode, binaryHeader *header);

// Função que faz o tratamento de redistribuição após uma remoção
void redistributeNodes(FILE *file, int posicaoNode, int posicaoIrmao, binaryHeader *header);

// Função que faz a concatenação de dois nós após uma remoção
void concatenateNodes(FILE *file, int posicaoNode, int posicaoIrmao, binaryHeader *header);

#endif