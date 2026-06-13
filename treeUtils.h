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
 * @brief Mecanismo recursivo de descida e partição: Insere uma chave navegando pelas páginas em disco.
 *
 * Realiza a busca em profundidade pelo nó folha apropriado para a inserção. Na subida da 
 * recursão, atua como o controlador de transição de estados da árvore, tratando de forma 
 * autônoma dois cenários lógicos pós-retorno:
 * 1. Estabilização (NO_PROMOTION): O nó possui espaço livre (nroChaves < 3) e insere o elemento ordenando as chaves.
 * 2. Propagação (PROMOTION): O nó sofre estouro de capacidade (Overflow), disparando 
 * um Split e promovendo o elemento médio para a página pai.
 *
 * @param file Ponteiro do tipo FILE correspondente ao arquivo de índice binário Árvore-B.
 * @param rrnAtual RRN do nó atual avaliado na recursão.
 * @param chave O valor numérico 'codEstacao' que está sendo inserido.
 * @param ptr O ponteiro de dados (byte offset) correspondente à chave no arquivo de dados.
 * @param promotionKey Ponteiro para capturar a chave promovida para o andar superior em caso de Split.
 * @param promotionPtr Ponteiro para capturar o byte offset associado à chave promovida.
 * @param promotionRightChild Ponteiro para capturar o RRN da nova subárvore à direita gerada pelo Split.
 * @param header Ponteiro para a estrutura de cabeçalho da Árvore-B em memória RAM.
 * @return int Códigos de estado contratuais: PROMOTION (houve split), NO_PROMOTION (estabilizou), ou ERROR (chave duplicada).
 */
int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, int *chavePromovida, int *ptrPromovido, 
                            int *rrnDireita, binaryHeader *header);


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

// Remove uma chave da árvore tratando underflow, redistribuição e concatenação
void removeKey(FILE *file, int chave, binaryHeader *header);


/**
 * @brief Função auxiliar de Split: Trata o overflow de chaves dividindo um nó cheio.
 *
 * Implementa a rotina de particionamento de um nó que atingiu o limite de chaves (Overflow).
 * Utiliza vetores auxiliares em RAM para acomodar temporariamente as 4 chaves e 5 ponteiros para nós auxiliares. 
 * Conforme as especificações de distribuição para uma Árvore-B de ordem m=4, a partição é realizada 
 * de forma a garantir que a chave promovida distribua o mais uniformemente possível os elementos:
 * - O nó original (à esquerda) retém as 2 primeiras chaves.
 * - A 3ª chave (elemento médio) é separada e enviada por referência para ser promovida ao nó pai.
 * - A 4ª chave é transferida para uma nova página criada (sempre à direita).
 *
 * Limpa explicitamente quaisquer resíduos físicos ou dados fantasmas com o valor -1 nas posições vazias 
 * das páginas para evitar inconsistências no arquivo binário de índices.
 *
 * @param file Ponteiro do tipo FILE correspondente ao arquivo de índice binário Árvore-B.
 * @param p_oldpage Ponteiro para a struct do nó cheio que será dividido.
 * @param rrn_oldpage RRN físico correspondente à página cheia no disco.
 * @param key O valor numérico 'codEstacao' transbordado vindo do nível inferior.
 * @param ponteiro O ponteiro de dados (byte offset) associado à chave transbordada.
 * @param r_child O RRN da subárvore à direita gerada pelo split anterior (se aplicável).
 * @param promo_key Ponteiro para capturar a chave que será enviada para o nó pai.
 * @param promo_ponteiro Ponteiro para capturar o byte offset associado à chave promovida.
 * @param promo_r_child Ponteiro para capturar o RRN alocado para a nova página da direita.
 * @param header Ponteiro para a estrutura de cabeçalho da Árvore-B em memória RAM.
 */
void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq, int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita, binaryHeader *header);

                      

#endif