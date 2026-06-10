#ifndef TREEUTILS_H
#define TREEUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include "binaryTree.h"

#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4
#define MAX_CHAVES 3

// Insere uma chave e o RRN do registro correspondente na árvore. Trata o caso de split na raiz.
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header);

// Busca uma chave na árvore, retorna o RRN do registro ou -1 se não encontrar
int searchKey(FILE *file, int chave, binaryHeader *header);

// Remove uma chave da árvore tratando underflow, redistribuição e concatenação
void removeKey(FILE *file, int chave, binaryHeader *header);

// Retorna o RRN onde um novo nó vai ser escrito, reaproveitando nós removidos da pilha ou usando valor do campo proxRRN
int alocarRRN(FILE *file, binaryHeader *header);

/*
 * Insere a chave e o ponteiro para o registro na posição correta dentro do nó, mantendo a ordem. 
 * Empurra o filho da direia para a posição certa caso ele venha de um split
 */
void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita);

/*
 * Faz o split de um nó cheio.
 * Quando um nó estoura (teria 4 chaves):
 *   - nó esquerdo fica com 2 chaves
 *   - primeira chave do nó à direita é promovida
 *   - nó direito (novo) fica com 1 chave
*/
void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq, int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita, binaryHeader *header);

                      
/*
 * Inserção recursiva na árvore. Desce até a folha certa e sobe promovendo chaves quando há overflow.
 * Retorna 1 se houve promoção (split), 0 caso contrário. A chave/ptr/filho promovidos são retornados pelos ponteiros.
 */
int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, int *chavePromovida, int *ptrPromovido, 
                            int *rrnDireita, binaryHeader *header);


#endif