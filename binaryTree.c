#include "binaryTree.h"
#define HEADER_SIZE sizeof(binaryHeader)
#define NODE_SIZE sizeof(binaryNode)
#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53

void readBinaryHeader(binaryHeader *header, FILE *file) {
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

// função para escrever o cabeçalho no arquivo binário
void writeBinaryHeader(binaryHeader *header, FILE *file){
    fseek(file, 0, SEEK_SET); // Garante que o ponteiro do arquivo esteja no início
    fwrite(&header->status, sizeof(char), 1, file);
    fwrite(&header->noRaiz, sizeof(int), 1, file);
    fwrite(&header->topo, sizeof(int), 1, file);
    fwrite(&header->proxRRN, sizeof(int), 1, file);
    fwrite(&header->nroNos, sizeof(int), 1, file);
}

int readBinaryNode(binaryNode *node, FILE *file, int rrn){
    fseek(file, rrn * sizeof(binaryNode), SEEK_SET);
    fread(&node->removido, sizeof(char), 1, file);
    fread(&node->proximo, sizeof(int), 1, file);
    fread(&node->tipoNo, sizeof(int), 1, file);
    fread(&node->nroChaves, sizeof(int), 1, file);
    fread(node->chaves, sizeof(int), 3, file);
    fread(node->ponteiros, sizeof(int), 3, file);
    fread(node->filhos, sizeof(int), 4, file);
    return 1;
}

// função para escrever um nó da árvore binária
void writeBinaryNode(binaryNode *node, FILE *file, int rrn){
    fseek(file, HEADER_SIZE + NODE_SIZE*(rrn), SEEK_SET);
    fwrite(&node->removido, sizeof(char), 1, file);
    fwrite(&node->proximo, sizeof(int), 1, file);
    fwrite(&node->tipoNo, sizeof(int), 1, file);
    fwrite(&node->nroChaves, sizeof(int), 1, file);
    fwrite(node->chaves, sizeof(int), 3, file);
    fwrite(node->ponteiros, sizeof(int), 3, file);
    fwrite(node->filhos, sizeof(int), 4, file);
}

void createEmptyBinaryNode(binaryNode* node){

}