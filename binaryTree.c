#include "binaryTree.h"
#define HEADER_SIZE sizeof(binaryHeader)
#define NODE_SIZE sizeof(binaryNode)

// função para ler o cabeçalho do arquivo binário
void readBinaryHeader(binaryHeader *header, FILE *file, int rrn){
    fseek(file, rrn * sizeof(binaryHeader), SEEK_SET);
    fread(&header->status, sizeof(char), 1, file);
    fread(&header->noRaiz, sizeof(int), 1, file);
    fread(&header->topo, sizeof(int), 1, file);
    fread(&header->proxRRN, sizeof(int), 1, file);
    fread(&header->nroNos, sizeof(int), 1, file);
}

void createBinaryHeader(binaryHeader *header){
    header->status = '0'; // Inicia como inconsistente
    header->noRaiz = -1; // Inicialmente, não há nó raiz
    header->topo = -1; // Inicialmente, a pilha de nós removidos está vazia
    header->proxRRN = 0; // O próximo RRN disponível para escrita é 0
    header->nroNos = 0; // Inicialmente, não há nós na árvore
}

// função para escrever o cabeçalho no arquivo binário
void writeBinaryHeader(binaryHeader *header, FILE *file){
    fseek(file, HEADER_SIZE, SEEK_SET); // Garante que o ponteiro do arquivo esteja no início
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
    fread(node->chaves, sizeof(int), 3, file);
    fread(node->ponteiros, sizeof(int), 3, file);
    fread(node->filhos, sizeof(int), 4, file);
    return 1; // Sucesso na leitura
}

// função para escrever um nó da árvore binária
void writeBinaryNode(binaryNode *node, FILE *file, int rrn){
    // Posiciona o ponteiro do arquivo no local correto para escrever o nó
    fseek(file, HEADER_SIZE + (rrn * NODE_SIZE), SEEK_SET);
    fwrite(&node->removido, sizeof(char), 1, file);
    fwrite(&node->proximo, sizeof(int), 1, file);
    fwrite(&node->tipoNo, sizeof(int), 1, file);
    fwrite(&node->nroChaves, sizeof(int), 1, file);
    fwrite(node->chaves, sizeof(int), 3, file);
    fwrite(node->ponteiros, sizeof(int), 3, file);
    fwrite(node->filhos, sizeof(int), 4, file);
}

// função que cria um nó vazio
void createEmptyBinaryNode(binaryNode *newNode){
    newNode->removido = '0'; // Nó válido
    newNode->proximo = -1; // Não utilizado para nós válidos
    newNode->tipoNo = -1; // Nó folha
    newNode->nroChaves = 0; // Inicialmente, não há chaves no nó
    for (int i = 0; i < 3; i++){
        newNode->chaves[i] = -1; // Inicializa as chaves como -1 para indicar que estão vazias
        newNode->ponteiros[i] = -1; // Inicializa os ponteiros como -1 para indicar que estão vazios
    }
    for (int i = 0; i < 4; i++){
        newNode->filhos[i] = -1; // Inicializa os filhos como -1 para indicar que estão vazios
    }
}

int createNode(binaryNode* toRecordNode,FILE *file, binaryHeader *header){
    int rrn;

    // Lógica de reaproveitamento ou criação no fim do arquivo
    if (header->topo == -1){
        rrn = header->proxRRN; 
        header->proxRRN++; 
    } else {
        rrn = header->topo;
        binaryNode antigoNo;
        readBinaryNode(&antigoNo, file, rrn); 
        header->topo = antigoNo.proximo; 
    }

    writeBinaryNode(&toRecordNode, file, rrn); 
    header->nroNos++; 
    return rrn;
}