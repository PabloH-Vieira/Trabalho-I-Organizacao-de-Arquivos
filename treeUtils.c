#include "treeUtils.h"
#define PROMOTION 1
#define NO_PROMOTION 0
#define ERROR -1


int insertRecursive(FILE *file, binaryHeader *header, int rrnAtual, int chave, int posKey, int* promotionKey, int* posPromotion, int *promotionRightChild){
    // Caso Base: nó folha
    if (rrnAtual == -1) {
        *promotionKey = chave;
        *posPromotion = posKey;
        *promotionRightChild = -1; // Nó folha não tem filho direito
        return PROMOTION; // Indica que houve promoção
    }
    
    // Carregar a página do nó atual
    binaryNode node;
    readBinaryNode(&node, file, rrnAtual);

    // Buscar a posição correta para inserção
    int i = 0;
    while (i < node.nroChaves && chave > node.chaves[i]) {
        // Verificar se a chave já existe
        if (chave == node.chaves[i])
            return ERROR; // Chave já existe, não inserir
        // Continuar buscando até encontrar uma chave maior ou chegar ao final das chaves
        if (chave < node.chaves[i])
            break; // Encontrou a posição correta para inserção
        i++;
    }

    // Variáveis para armazenar os resultados da recursão
    int promotionKeyBelow, posPromotionBelow, promotionKeyBelowRRN;
    int result = insertRecursive(file, header, node.filhos[i], chave, i, &promotionKeyBelow, &posPromotionBelow, &promotionKeyBelowRRN);

    if (result == NO_PROMOTION || result == ERROR)
        return result; // Propagar o resultado da recursão
    
    // Se houve promoção, precisamos inserir a chave promovida no nó atual
    if (node.nroChaves < 3) {
        // Inserir a chave promovida no nó atual fazendo a movimentação das chaves e ponteiros
        inserirKey(file, rrnAtual, promotionKeyBelow, posPromotionBelow, promotionKeyBelowRRN, header);
        return NO_PROMOTION; // Não houve promoção adicional
    } 
    // Se o nó atual está cheio, é necessário fazer um split
    else {
        binaryNode newNode;
        createEmptyBinaryNode(&newNode);
        int promoted = splitNode(file, &node, rrnAtual, promotionKeyBelow, posPromotionBelow, promotionKeyBelowRRN, header);

        // Guarda o RRN de onde o novo nó será guardado
        int newPageRRN = createNode(file, header);
        // Escreve os dois nós da lógica de split
        writeBinaryNode(&node, file, header);
        writeBinaryNode(&newNode, file, header);
        *promotionKey = promotionKeyBelow; // Guarda a chave a ser promovida
        *posPromotion = posPromotionBelow; // Guarda a posição da chave a ser promovida
        *promotionRightChild = newPageRRN; // Guarda a posição do nó criado na lógica de split
        return PROMOTION; // Indica que houve promoção
    }
}

int splitNode(FILE *file, binaryNode *node, int rrnAtual, int promotionKeyBelow, int posPromotionBelow, int promotionKeyBelowRRN, binaryHeader *header){

}