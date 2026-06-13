#ifndef TREEUTILS_H
#define TREEUTILS_H
#include <stdio.h>
#include <stdlib.h>
#include "binaryTree.h"

#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4
#define MAX_CHAVES 3

/**
 * @brief Função auxiliar de gerência de espaço: Aloca um RRN para uma nova página da Árvore-B.
 *
 * Implementa a abordagem dinâmica de reaproveitamento de espaços para o arquivo de índice 
 * utilizando o conceito de pilha de registros logicamente removidos. 
 * Quando uma nova página precisa ser criada (devido a um Split), a função verifica o campo 'topo' 
 * do cabeçalho. Se houver uma página empilhada (topo != -1), realiza o 'Pop' na pilha, 
 * reaproveitando o RRN e atualizando o cabeçalho com o link para o próximo nó livre. 
 * Caso contrário, aloca um RRN virgem no final do arquivo utilizando e incrementando o 'proxRRN'. 
 * Incrementa também o contador total de nós ativos na árvore.
 *
 * @param file Ponteiro do tipo FILE correspondente ao arquivo de índice binário.
 * @param header Ponteiro para a struct de cabeçalho da Árvore-B em memória RAM.
 * @return int O número do RRN alocado (seja um espaço reaproveitado ou uma nova extensão de fim de arquivo).
**/
int alocarRRN(FILE *file, binaryHeader *header);


/**
 * @brief Função auxiliar de ordenação em memória: Insere uma chave e seus ponteiros em um nó não-cheio.
 *
 * Realiza uma inserção direta dentro dos vetores de uma página em memória RAM. Desloca em cascata todas
 * as chaves maiores para as posições subsequentes, arrastando seus respectivos ponteiros de dados (byte offsets) 
 * e ponteiros de subárvores. Essa operação mantém estrita a propriedade estrutural da 
 * Árvore-B de ordenação interna crescente das chaves de busca (C1 < C2 < ... < Cq-1) dentro de cada nó.
 *
 * @param node Ponteiro para a struct do nó da Árvore-B em RAM que receberá o novo elemento.
 * @param chave O valor do 'codEstacao' a ser inserido ordenadamente.
 * @param ptr O ponteiro para o arquivo de dados (byte offset do registro original) associado à chave.
 * @param filhoDireita O ponteiro de subárvore (RRN da página da Árvore-B) gerado à direita da chave (ex: após um split).
 */
void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita);


/**
 * @brief Orquestrador de Inserção na Árvore-B: Insere uma nova chave de busca e seu ponteiro de dados.
 *
 * Coordena o fluxo de inserção de chaves baseadas no campo 'codEstacao' no índice binário. 
 * Lida de forma autônoma com dois cenários estruturais distintos:
 * 1. Árvore Vazia: Instancia e grava a primeira página folha, definindo-a como a raiz inicial da estrutura.
 * 2. Árvore Existente: Dispara a descida recursiva pelos nós em disco. Caso o particionamento (Split) 
 * propague-se de baixo para cima até alcançar o topo, a função intercepta a promoção, cria uma nova 
 * página raiz e realiza o acoplamento das subárvores geradas, expandindo a altura do índice.
 *
 * @param file Ponteiro do tipo FILE correspondente ao arquivo de índice binário Árvore-B.
 * @param byteOffsetRegistro Posição física (byte offset) do registro correspondente no arquivo de dados.
 * @param chave O valor numérico 'codEstacao' que será indexado na estrutura.
 * @param header Ponteiro para a estrutura de cabeçalho da Árvore-B em memória RAM.
**/
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header);


/**
 * @brief Algoritmo de busca na Árvore-B: Localiza o ponteiro de dados associado a uma chave.
 *
 * Realiza uma pesquisa em profundidade por caminhos de subárvores no arquivo de índice para 
 * encontrar a chave de busca ('codEstacao'). Navega pelas páginas lidas do disco 
 * utilizando busca linear dentro do nó. Se a correspondência exata for localizada, retorna 
 * imediatamente o byte offset (ponteiro de dados) correspondente ao registro no arquivo de dados. 
 * Caso atinja um nó folha e o ponteiro de descida decline para -1, conclui que a chave não 
 * está cadastrada no sistema.
 *
 * @param file Ponteiro do tipo FILE correspondente ao arquivo de índice binário Árvore-B.
 * @param chave O valor numérico 'codEstacao' a ser pesquisado.
 * @param header Ponteiro para a estrutura de cabeçalho da Árvore-B em memória RAM.
 * @return int O byte offset do registro no arquivo de dados se encontrado; ou -1 caso a chave não exista.
 */
int searchKey(FILE *file, int chave, binaryHeader *header);

               
/**
 * @brief Gerenciador de reaproveitamento de páginas: Empilha um nó liberado na lista LIFO.
 * @param file Ponteiro do arquivo de índice (.bin).
 * @param rrn Número do RRN do nó que está sendo destruído/liberado.
 * @param header Ponteiro para o cabeçalho da Árvore-B em RAM.
 */
void empilharNoRemovido(FILE *file, int rrn, binaryHeader *header);

/**
 * @brief Busca em profundidade pelo menor elemento da subárvore direita.
 * @param file Ponteiro do arquivo de índice (.bin).
 * @param rrnFilhoDireita RRN do filho à direita da chave a ser substituída.
 * @param chaveSucc Ponteiro para capturar o valor da chave sucessora encontrada na folha.
 * @param ptrSucc Ponteiro para capturar o byte offset associado à chave sucessora.
 * @return int O RRN da página folha onde o sucessor foi localizado.
 */
int encontrarSucessor(FILE *file, int rrnFilhoDireita, int *chaveSucc, int *ptrSucc);

/**
 * @brief Balanceamento por empréstimo: Move chaves uniformemente entre nós irmãos adjacentes.
 * @param file Ponteiro do arquivo de índice (.bin).
 * @param rrnPai RRN do nó pai que abriga a chave separadora.
 * @param indiceFilho Posição do filho em subunderflow no vetor de ponteiros do pai.
 * @param lado Direção da redistribuição: 0 para irmão direito, 1 para irmão esquerdo.
 */
void redistribuir(FILE *file, int rrnPai, int indiceFilho, int lado);

/**
 * @brief Fusão estrutural de nós: Une duas páginas irmãs em caso de falha de redistribuição.
 * @param file Ponteiro do arquivo de índice (.bin).
 * @param rrnPai RRN do nó pai que sofrerá a contração/descida da chave separadora.
 * @param indiceFilho Posição da subárvore direita envolvida na fusão.
 * @param header Ponteiro para o cabeçalho da Árvore-B em RAM (para gerenciar o empilhamento do nó destruído).
 * @return int Retorna 1 se o nó pai também entrou em sob-ocupação (Underflow); 0 caso contrário.
 */
int concatenar(FILE *file, int rrnPai, int indiceFilho, binaryHeader *header);

/**
 * @brief Orquestrador de Exclusão na Árvore-B: Ponto de entrada para a remoção física de chaves.
 * @param file Ponteiro do arquivo de índice (.bin).
 * @param chave O código da estação ('codEstacao') a ser removido do índice.
 * @param header Ponteiro para o cabeçalho da Árvore-B em RAM.
 */
void removeKey(FILE *file, int chave, binaryHeader *header);

#endif