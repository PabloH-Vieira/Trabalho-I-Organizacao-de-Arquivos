#ifndef TREEFUNCTIONALITIES_H
#define TREEFUNCTIONALITIES_H
#include "binaryTree.h"
#include "treeUtils.h"

// Cria o arquivo de índice a partir do arquivo binário, usando o campo de codEstacao
void createIndex(char *binFileName, char *indexFileName);

// Busca um registro no arquivo binário usando a árvore de índice e os critérios de busca, e imprime o registro encontrado
void searchWithIndex(char *binFileName, char *indexFileName, int nroBuscas);

// Função que remove um registro do arquivo binário usando a árvore de índice
void deleteWithIndex(char *binFileName, char *indexFileName, int nroRemocoes);

// Função que insere um novo registro no arquivo binário e atualiza a árvore de índice
void insertWithIndex(char *binFileName, char *indexFileName, int nroInsercoes);

#endif