#include "binaryTree.h"
#define HEADER_SIZE 17
#define NODE_SIZE 53

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
    fseek(file, HEADER_SIZE + (rrn * NODE_SIZE), SEEK_SET);
    fread(&node->removido, sizeof(char), 1, file);
    fread(&node->proximo, sizeof(int), 1, file);
    fread(&node->tipoNo, sizeof(int), 1, file);
    fread(&node->nroChaves, sizeof(int), 1, file);
    for (int i = 0; i < 3; i++) {
        fread(&node->chaves[i], sizeof(int), 1, file);
        fread(&node->ponteiros[i], sizeof(int), 1, file);
    }
    fread(node->filhos, sizeof(int), 4, file);
    return 1;
}

// função para escrever um nó da árvore binária
void writeBinaryNode(binaryNode *node, FILE *file, int rrn) {
    fseek(file, HEADER_SIZE + NODE_SIZE * rrn, SEEK_SET);
    fwrite(&node->removido, sizeof(char), 1, file);
    fwrite(&node->proximo, sizeof(int), 1, file);
    fwrite(&node->tipoNo, sizeof(int), 1, file);
    fwrite(&node->nroChaves, sizeof(int), 1, file);
    // intercala chave e ponteiro: C1, PR1, C2, PR2, C3, PR3
    for (int i = 0; i < 3; i++) {
        fwrite(&node->chaves[i], sizeof(int), 1, file);
        fwrite(&node->ponteiros[i], sizeof(int), 1, file);
    }
    fwrite(node->filhos, sizeof(int), 4, file);
}



void createEmptyBinaryNode(binaryNode* newNode){
    newNode->removido = '0'; // Nó válido
    newNode->proximo = -1; // Não utilizado para nós válidos
    newNode->tipoNo = -1; // Nó folha
    newNode->nroChaves = 0; // Inicialmente, não há chaves no nó
    for (int i = 0; i < 3; i++){
        newNode->chaves[i] = -1; // Inicializa as chaves como -1 para indicar que estão vazias
        newNode->ponteiros[i] = -1; // Inicializa os ponteiros como -1 para indicar que estão vazios
        newNode->filhos[i] = -1;
    }
    newNode->filhos[3] = -1; 
}
