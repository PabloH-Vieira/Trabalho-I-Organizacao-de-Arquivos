#include "binaryTree.h"

#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53

void readBinaryHeader(binaryHeader *header, FILE *file, int rrn) {
    fseek(file, 0, SEEK_SET);
    fread(&header->status, sizeof(char), 1, file);
    fread(&header->noRaiz, sizeof(int), 1, file);
    fread(&header->topo, sizeof(int), 1, file);
    fread(&header->proxRRN, sizeof(int), 1, file);
    fread(&header->nroNos, sizeof(int), 1, file);
}

void createBinaryHeader(binaryHeader *header) {
    header->status = '0';
    header->noRaiz = -1;
    header->topo = -1;
    header->proxRRN = 0;
    header->nroNos = 0;
}

void writeBinaryHeader(binaryHeader *header, FILE *file) {
    fseek(file, 0, SEEK_SET);
    fwrite(&header->status, sizeof(char), 1, file);
    fwrite(&header->noRaiz, sizeof(int), 1, file);
    fwrite(&header->topo, sizeof(int), 1, file);
    fwrite(&header->proxRRN, sizeof(int), 1, file);
    fwrite(&header->nroNos, sizeof(int), 1, file);
}

int readBinaryNode(binaryNode *node, FILE *file, int rrn) {
    // posição = cabeçalho + rrn * tamanho fixo do nó (53 bytes)
    fseek(file, TAMANHO_CABECALHO + rrn * TAMANHO_NO, SEEK_SET);
    fread(&node->removido, sizeof(char), 1, file);
    fread(&node->proximo, sizeof(int), 1, file);
    fread(&node->tipoNo, sizeof(int), 1, file);
    fread(&node->nroChaves, sizeof(int), 1, file);
    fread(node->chaves, sizeof(int), 3, file);
    fread(node->ponteiros, sizeof(int), 3, file);
    fread(node->filhos, sizeof(int), 4, file);
    return 1;
}

void writeBinaryNode(binaryNode *node, FILE *file, int rrn) {
    // mesma lógica do read — posiciona no lugar certo antes de escrever
    fseek(file, TAMANHO_CABECALHO + rrn * TAMANHO_NO, SEEK_SET);
    fwrite(&node->removido, sizeof(char), 1, file);
    fwrite(&node->proximo, sizeof(int), 1, file);
    fwrite(&node->tipoNo, sizeof(int), 1, file);
    fwrite(&node->nroChaves, sizeof(int), 1, file);
    fwrite(node->chaves, sizeof(int), 3, file);
    fwrite(node->ponteiros, sizeof(int), 3, file);
    fwrite(node->filhos, sizeof(int), 4, file);
}

binaryNode *createEmptyBinaryNode() {
    binaryNode *node = malloc(sizeof(binaryNode));
    if (node == NULL)
        return NULL;

    node->removido = '0';
    node->proximo = -1;
    node->tipoNo = -1; // começa como folha
    node->nroChaves = 0;

    for (int i = 0; i < 3; i++) {
        node->chaves[i] = -1;
        node->ponteiros[i] = -1;
    }
    for (int i = 0; i < 4; i++)
        node->filhos[i] = -1;

    return node;
}