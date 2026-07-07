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
 * Função recursiva de descida: Insere uma chave navegando pelas páginas em disco.
 *
 * Realiza a busca em profundidade pelo nó folha apropriado para a inserção. Na subida da 
 * recursão, lida com dois cenários possíveis:
 * 1. NO PROMOTION: O nó possui espaço livre e insere o elemento ordenando as chaves.
 * 2. PROMOTION: O nó sofre Overflow, realizando um Split e promovendo o elemento do meio para a página pai.
 * Recebe como parâmetros: file (ponteiro do tipo FILE para arquivo de índice binário), rrnAtual (RRN do nó atual na recursão),
 * chave (valor numérico 'codEstacao' que está sendo inserido), ptr (byte offset da chave no arquivo de dados),
 * promotionKey (ponteiro para guardar a chave promovida em caso de Split), promotionPtr (ponteiro para guardar 
 * o byte offset da chave promovida), promotionRightChild (ponteiro para guardar o RRN da 
 * nova subárvore à direita gerada pelo Split), header (ponteiro para o cabeçalho da Árvore-B).
 * Retorna PROMOTION (houve split), NO_PROMOTION, ou ERROR (chave duplicada).
 */
int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, int *chavePromovida, int *ptrPromovido, 
                            int *rrnDireita, binaryHeader *header);


/**
 * Função auxiliar Split: Trata o overflow de chaves dividindo um nó cheio.
 *
 * Implementa a rotina de split de um nó com Overflow. Utiliza vetores auxiliares em RAM para acomodar
 * temporariamente as 4 chaves e 5 ponteiros para nós auxiliares. 
 * - O nó original (à esquerda) retém as 2 primeiras chaves.
 * - A 3ª chave (elemento médio) é separada e enviada por referência para ser promovida ao nó pai.
 * - A 4ª chave é transferida para uma nova página criada (sempre à direita).
 * Recebe como parâmetros: file (ponteiro do tipo FILE para arquivo de índice binário), 
 * p_oldpage (ponteiro para onó cheio que será dividido), rrn_oldpage (RRN da página cheia),
 * key (valor numérico 'codEstacao' que não coube no nó), ponteiro (byte offset da chave que não coube),
 * r_child (RRN da subárvore à direita gerada pelo split anterior), promo_key (ponteiro para guardar a chave 
 * que será promovida), promo_ponteiro (ponteiro para guardar o byte offset da chave promovida),
 * promo_r_child (ponteiro para guardar o RRN alocado para a nova página da direita), header (ponteiro o cabeçalho da Árvore-B).
 */
void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq, int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita, binaryHeader *header);

/**
 * Função recursiva de remoção: Navega pelas páginas tratando Underflows na subida.
 * etorna 1 se o nó atual terminou com menos chaves do que o mínimo permitido; 0 caso estável.
 */
int removerRecursivo(FILE *file, int rrnAtual, int rrnPai, int indiceNoPai, int chave, binaryHeader *header);


#endif