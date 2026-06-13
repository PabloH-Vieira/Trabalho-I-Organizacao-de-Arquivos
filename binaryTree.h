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
    int tipoNo; // '-1' para nó folha, '0' para nó raiz, '1' para nó intermediário
    int nroChaves; // Número de chaves atualmente no nó
    int chaves[3]; // Vetor de chaves
    int ponteiros[3]; // Vetor de ponteiros para o byte offset dps registros correspondentes às chaves
    int filhos[4]; // Vetor de RRN dos nós filhos
}binaryNode;

// Funções de manipulação da árvore binária
// função para ler o cabeçalho do arquivo binário
void readBinaryHeader(binaryHeader *header, FILE *file);

// Função para criar um novo cabeçalho para o arquivo de índice
void createBinaryHeader(binaryHeader *header);

// função para escrever o cabeçalho no arquivo binário
void writeBinaryHeader(binaryHeader *header, FILE *file);

// função para ler um nó da árvore binária
int readBinaryNode(binaryNode *node, FILE *file, int rrn);

// função para escrever um nó da árvore binária
void writeBinaryNode(binaryNode *node, FILE *file, int rrn);

// função que cria um nó vazio
void createEmptyBinaryNode(binaryNode *newNode);

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

/**
 * @brief Algoritmo recursivo de remoção: Navega pelas páginas tratando Underflows na subida.
 * @return int Retorna 1 se o nó atual terminou com menos chaves do que o mínimo permitido; 0 caso estável.
 */
int removerRecursivo(FILE *file, int rrnAtual, int rrnPai, int indiceNoPai, int chave, binaryHeader *header);


#endif