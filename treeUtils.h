<<<<<<< Updated upstream
#include <stdio.h>
#include <stdlib.h>
#include "binaryTree.h"
#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4        // m = 4, máximo de 4 filhos por nó
#define MAX_CHAVES 3   // máximo de 3 chaves por nó

/*
 * Retorna o RRN onde um novo nó vai ser escrito.
 * Se houver nó removido na pilha, reaproveit ele.
 * Caso contrário, usa o proxRRN e incrementa.
 */
int alocarRRN(FILE *file, binaryHeader *header);

/*
 * Insere a chave e o ponteiro para o registro na posição correta
 * dentro do nó, mantendo a ordem crescente. Também empurra o
 * filho à direita para a posição certa caso venha de um split.
 */
void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita);

/*
 * Faz o split de um nó cheio. Com m=4, cada nó tem no máximo 3 chaves.
 * Quando um nó estoura (teria 4 chaves), dividimos assim:
 *   - nó esquerdo fica com 2 chaves (o nó original, modificado)
 *   - chave do meio é promovida para o pai
 *   - nó direito (novo) fica com 1 chave
 *
 * A chave promovida é a primeira do nó direito, conforme especificação.
 */
void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq, int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita, binaryHeader *header);

/*
 * Inserção recursiva na árvore. Desce até a folha certa e sobe
 * promovendo chaves quando há overflow (split).
 *
 * Retorna 1 se houve promoção (split), 0 caso contrário.
 * A chave/ptr/filho promovidos são retornados pelos ponteiros.
 */
int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, int *chavePromovida, int *ptrPromovido, 
                            int *rrnDireita, binaryHeader *header);

/*
 * Ponto de entrada da inserção. Cuida do caso especial de split
 * na raiz (quando a raiz precisa ser substituída por uma nova raiz).
 */
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header);

/*
 * Busca uma chave na árvore a partir da raiz.
 * Retorna o RRN do registro no arquivo de dados, ou -1 se não encontrar.
 */
int searchKey(FILE *file, int chave, binaryHeader *header);
=======
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
>>>>>>> Stashed changes
