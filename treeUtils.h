#ifndef TREEUTILS_H
#define TREEUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include "binaryTree.h"

#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4
#define MAX_CHAVES 3

/* Função auxiliar de gerência de espaço: Aloca um RRN para uma nova página da Árvore-B.
 * Implementa a abordagem dinâmica de reaproveitamento de espaços para o arquivo de índice 
 * utilizando o conceito de pilha de registros logicamente removidos. 
 * Quando uma nova página precisa ser criada (devido a um Split), a função verifica o campo 'topo' 
 * do cabeçalho. Se houver uma página empilhada (topo != -1), realiza o 'Pop' na pilha, 
 * reaproveitando o RRN e atualizando o cabeçalho com o link para o próximo nó livre. 
 * Caso contrário, aloca um RRN virgem no final do arquivo utilizando e incrementando o 'proxRRN'. 
 * Incrementa também o contador total de nós ativos na árvore.
 * Recebe como parâmetros: file (ponteiro do tipo FILE correspondente ao arquivo de índice binário) e
 * header (ponteiro para a struct de cabeçalho da Árvore-B)
 * Retorna o número do RRN alocado (seja um espaço reaproveitado ou uma inserção no fim do arquivo).
*/
int alocarRRN(FILE *file, binaryHeader *header);


/* Função auxiliar de ordenação em memória: Insere uma chave e seus ponteiros em um nó não-cheio.
 *
 * Realiza uma inserção direta dentro dos vetores de uma página em memória RAM. Desloca em cascata todas
 * as chaves maiores para as posições subsequentes, arrastando seus respectivos ponteiros de dados (byte offsets) 
 * e ponteiros de subárvores. Essa operação mantém estrita a propriedade estrutural da 
 * Árvore-B de ordenação interna crescente das chaves de busca (C1 < C2 < ... < Cq-1) dentro de cada nó.
 * Recebe como parâmetros: node (ponteiro para a struct do nó da Árvore-B que receberá o novo elemento),
 * chave (valor do 'codEstacao' a ser inserido ordenadamente), ptr (byte offset do registro original) e
 * filhoDireita (ponteiro de subárvore gerado à direita da chave.
 */
void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita);


/* Função de Inserção na Árvore-B: Insere uma nova chave de busca e seu ponteiro de dados.
 *
 * Lida com dois cenários distintos:
 * 1. Árvore Vazia: Instancia e grava a primeira página folha, definindo-a como a raiz inicial da estrutura.
 * 2. Árvore Existente: Dispara a descida recursiva pelos nós em disco. Caso o Split
 * propague-se de baixo para cima até alcançar o topo, a função intercepta a promoção, cria uma nova 
 * página raiz e realiza o acoplamento das subárvores geradas, expandindo a altura do índice.
 * Recebe como parâmetros: file (ponteiro do tipo FILE do arquivo de índice binário Árvore-B), 
 * byteOffsetRegistro (byteoffset do registro correspondente no arquivo de dados),
 * chave (valor numérico 'codEstacao' que será adicionado na estrutura) e header (ponteiro para o de cabeçalho da Árvore-B)
**/
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header);


/* Algoritmo de busca na Árvore-B: Localiza o ponteiro de dados associado a uma chave.
 * Realiza uma pesquisa em profundidade por caminhos de subárvores no arquivo de índice para 
 * encontrar a chave de busca ('codEstacao'). Navega pelas páginas lidas do disco 
 * utilizando busca linear dentro do nó. Se a correspondência exata for localizada, retorna 
 * imediatamente o byte offset (ponteiro de dados) correspondente ao registro no arquivo de dados. 
 * Caso atinja um nó folha e o ponteiro de descida decline para -1, conclui que a chave não 
 * está cadastrada no sistema.
 * Recebe como parâmetros: file (ponteiro do tipo FILE do arquivo de índice binário Árvore-B),
 * chave (valor numérico 'codEstacao' a ser pesquisado), header (ponteiro para o cabeçalho da Árvore-B).
 * Retorna o byte offset do registro no arquivo de dados se encontrado; ou -1 caso a chave não exista.
 */
int searchKey(FILE *file, int chave, binaryHeader *header);

               
/* Função de reaproveitamento de páginas: Empilha um nó liberado na pilha.
 * Recebe como parâmetros: file (ponteiro do arquivo de índice),
 * rrn (número do RRN do nó que está sendo removido)
 * header (ponteiro para o cabeçalho da Árvore-B).
 */
void empilharNoRemovido(FILE *file, int rrn, binaryHeader *header);

/* Função de busca em profundidade pelo menor elemento da subárvore direita.
 * Recebe como parâmetros: file (ponteiro do arquivo de índice),
 * rrnFilhoDireita (RRN do filho à direita da chave a ser substituída),
 * chaveSucc (ponteiro para capturar o valor da chave sucessora encontrada na folha).
 * ptrSucc (ponteiro para capturar o byte offset associado à chave sucessora)
 * Retorna o RRN da página folha onde o sucessor foi localizado.
 */
int encontrarSucessor(FILE *file, int rrnFilhoDireita, int *chaveSucc, int *ptrSucc);

/* Função de balanceamento: move chaves uniformemente entre nós irmãos adjacentes.
 * Recebe como parâmetros: file (ponteiro do arquivo de índice).
 * rrnPai (RRN do nó pai que abriga a chave separadora), 
 * indiceFilho (posição do filho em overflow no vetor de ponteiros do pai),
 * lado (direção da redistribuição: 0 para irmão direito, 1 para irmão esquerdo).
 */
void redistribuir(FILE *file, int rrnPai, int indiceFilho, int lado);

/* FUnção de fusão de nós: une duas páginas irmãs em caso de falha de redistribuição.
 * Recebe como parâmetros: file (ponteiro do arquivo de índice),
 * rrnPai (RRN do nó pai onde ocorrerá a descida da chave separadora),
 * indiceFilho (posição da subárvore direita envolvida na fusão),
 * header (ponteiro para o cabeçalho da Árvore-B para gerenciar o empilhamento do nó destruído).
 * Retorna 1 se o nó pai também entrou Underflow; 0 caso contrário.
 */
int concatenar(FILE *file, int rrnPai, int indiceFilho, binaryHeader *header);

/** Função de exclusão de chaves na Árvore-B
 * Recebe como parâmetros: file (ponteiro do arquivo de índice),
 * chave (código da estação a ser removido do índice) e header (ponteiro para o cabeçalho da Árvore-B).
 */
void removeKey(FILE *file, int chave, binaryHeader *header);

#endif