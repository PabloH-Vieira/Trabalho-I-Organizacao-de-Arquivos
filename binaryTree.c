#include "binaryTree.h"
#include "treeUtils.h"
#define HEADER_SIZE 17
#define NODE_SIZE 53
#define PROMOTION 1
#define NO_PROMOTION 0
#define ERROR -1

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


int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, 
                    int *promotionKey, int *promotionPtr, int *promotionRightChild, binaryHeader *header) {
    
    // CASO BASE DA RECURSÃO
    // Se ultrapassou um nó folha, significa que a folha ideal foi localizada no andar anterior. 
    // Configura os parâmetros de promoção para realizar a inserção na folha.
    if (rrnAtual == -1) {
        *promotionKey = chave;
        *promotionPtr = ptr;
        *promotionRightChild = -1; // Folhas não possuem descendentes
        return PROMOTION; 
    }
    
    // Traz a página do disco de exatos 53 bytes para a memória RAM
    binaryNode node;
    readBinaryNode(&node, file, rrnAtual);

    // Busca linear dentro do nó: localiza o índice da chave maior ou igual ao elemento.
    // Esse índice mapeia perfeitamente o ponteiro do filho.
    int i = 0;
    while (i < node.nroChaves && chave > node.chaves[i]) {
        i++;
    }

    // VERIFICAÇÃO DE EXCLUSIVIDADE
    if (i < node.nroChaves && chave == node.chaves[i]) {
        return ERROR; 
    }

    // Variáveis locais para isolar o retorno e o overflow das subárvores inferiores
    int promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow;
    
    // DESCIDA RECURSIVA
    int result = inserirRecursivo(file, node.filhos[i], chave, ptr, 
                                 &promotionKeyBelow, &promotionPtrBelow, &promotionRightChildBelow, header);

    // ANÁLISE DO RETORNO DA RECURSÃO
    // Se o andar inferior estabilizou com sucesso (NO_PROMOTION) ou encontrou erro de duplicidade (ERROR), 
    // repassa o código para os andares superiores sem executar modificações nesta página.
    if (result == NO_PROMOTION || result == ERROR) {
        return result; 
    }
    
    // TRATAMENTO DE OVERFLOW
    // Se o código é PROMOTION, a página filha estourou e enviou uma chave para ser inserida neste nó pai.

    if (node.nroChaves < 3) {
        // CASO DE DISPONIBILIDADE DE ESPAÇO
        // A função inserirNoNo faz o rearranjo e a ordenação interna dos vetores.
        inserirNoNo(&node, promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow);
        
        // Salva a alteração da página em disco de volta no seu RRN original
        writeBinaryNode(&node, file, rrnAtual);
        return NO_PROMOTION; // avisa o pai que a árvore estabilizou
    } 
    else {
        // CASO DE INDISPONIBILIDADE DE ESPAÇO
        // Faz o split
        splitNode(file, &node, rrnAtual, promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow,
                  promotionKey, promotionPtr, promotionRightChild, header);
                  
        return PROMOTION; // Propaga o resultado
    }
}


void splitNode(FILE *file, binaryNode *p_oldpage, int rrn_oldpage, 
               int key, int ponteiro, int r_child, 
               int *promo_key, int *promo_ponteiro, int *promo_r_child, 
               binaryHeader *header) {
    
    // CRIAÇÃO DOS ARRAYS temporários
    // Permitem inserir a nova chave e realizar a ordenação antes da divisão física do nó.
    int tempchaves[4];       
    int tempponteiros[4];    
    int tempfilhos[5]; // Ordem m=4 implica em até 5 filhos possíveis para 4 chaves transitórias

    // Transfere o estado atual do nó cheio em disco para os vetores temporários
    int i;
    for (i = 0; i < 3; i++){
        tempchaves[i] = p_oldpage->chaves[i];
        tempponteiros[i] = p_oldpage->ponteiros[i];
        tempfilhos[i] = p_oldpage->filhos[i];
    }
    tempfilhos[3] = p_oldpage->filhos[3]; // Copia a referência do quarto descendente original

    // INSERÇÃO ORDENADA
    // Percorre o vetor de temporário de trás para frente, deslocando elementos maiores 
    // para abrir o espaço correto para o novo elemento vindo de baixo.
    for (i = 3; (key < tempchaves[i - 1]) && (i > 0); i--) {
        tempchaves[i] = tempchaves[i - 1];
        tempponteiros[i] = tempponteiros[i - 1];
        tempfilhos[i + 1] = tempfilhos[i];
    }
    
    // Insere paralelamente a chave, o offset de dados e o filho correspondente à direita
    tempchaves[i] = key;
    tempponteiros[i] = ponteiro;
    tempfilhos[i + 1] = r_child;

    // ALOCAÇÃO DE ESPAÇO PARA O NOVO NÓ
    // Se a pilha de removidos do cabeçalho tiver nós livres (topo != -1), 
    // ele é reaproveitado; caso contrário, estende o fim do arquivo incrementando proxRRN.
    *promo_r_child = alocarRRN(file, header);               

    binaryNode p_newpage;
    createEmptyBinaryNode(&p_newpage); // Inicializa a struct da direita limpando lixo de memória

    // ATUALIZAÇÃO DOS TIPOS DOS NÓS
    // Se o nó que está dividindo era a raiz interna (tipoNo == 0),
    // ele perderá esse status. Se ele for um nó folha, rebaixa ambos para folha (-1);
    // se possuir descendentes, rebaixa ambos para nós intermediários (tipoNo == 1).
    if (p_oldpage->tipoNo == 0) {
        if (p_oldpage->filhos[0] == -1) {
            p_oldpage->tipoNo = -1; 
            p_newpage.tipoNo = -1;  
        } else {
            p_oldpage->tipoNo = 1;  
            p_newpage.tipoNo = 1;   
        }
    } else {
        p_newpage.tipoNo = p_oldpage->tipoNo; // Se não era raiz, herda o nível do nó
    }

    // DISTRIBUIÇÃO DE ELEMENTOS
    
    // NÓ ESQUERDO
    // Guarda os 2 primeiros elementos de menor valor.
    p_oldpage->chaves[0] = tempchaves[0];
    p_oldpage->ponteiros[0] = tempponteiros[0];
    p_oldpage->filhos[0] = tempfilhos[0];

    p_oldpage->chaves[1] = tempchaves[1];
    p_oldpage->ponteiros[1] = tempponteiros[1];
    p_oldpage->filhos[1] = tempfilhos[1];
    p_oldpage->filhos[2] = tempfilhos[2];

    // Zera os índices da antiga terceira chave colocando o valor sentinela -1.
    p_oldpage->chaves[2] = -1;
    p_oldpage->ponteiros[2] = -1;
    p_oldpage->filhos[3] = -1;
    p_oldpage->nroChaves = 2; // Atualiza a taxa de ocupação local para 2 chaves ativas

    // CONFIGURAÇÃO DA PROMOÇÃO
    // Isola a 3ª chave e envia para as variáveis de referência.
    // Elas serão coletadas pela função recursiva mãe para inserção no nó pai.
    *promo_key = tempchaves[2];
    *promo_ponteiro = tempponteiros[2];

    // NÓ DIREITO
    // Herda a 4ª chave (maior valor). Preserva os filhos.
    p_newpage.chaves[0] = tempchaves[3];
    p_newpage.ponteiros[0] = tempponteiros[3];
    p_newpage.filhos[0] = tempfilhos[3];
    p_newpage.filhos[1] = tempfilhos[4];
    p_newpage.nroChaves = 1; // Nova página nasce com exatamente 1 chave ativa

    // PERSISTÊNCIA
    writeBinaryNode(p_oldpage, file, rrn_oldpage);
    writeBinaryNode(&p_newpage, file, *promo_r_child);
}


int removerRecursivo(FILE *file, int rrnAtual, int rrnPai, int indiceNoPai, int chave, binaryHeader *header) {
    // CASO BASE: Chave de busca não localizada na árvore de índices
    if (rrnAtual == -1)
        return 0; 

    binaryNode no;
    readBinaryNode(&no, file, rrnAtual);

    int i = 0;
    while (i < no.nroChaves && chave > no.chaves[i])
        i++;

    if (i < no.nroChaves && chave == no.chaves[i]) {
        // CASO DE CHAVE ENCONTRADA
        if (no.filhos[0] != -1) {
            // Caso de Nó Interno. Troca a chave pelo seu sucessor imediato da folha antes da remoção.
            int chaveSucc, ptrSucc;
            encontrarSucessor(file, no.filhos[i + 1], &chaveSucc, &ptrSucc);

            no.chaves[i]    = chaveSucc;
            no.ponteiros[i] = ptrSucc;
            writeBinaryNode(&no, file, rrnAtual);

            // Propaga a recursão para remover o elemento duplicado da folha de origem
            int underflow = removerRecursivo(file, no.filhos[i + 1], rrnAtual,
                                              i + 1, chaveSucc, header);
            if (!underflow)
                return 0; // Árvore estabilizada abaixo
        } else {
            // Caso de Nó Folha. Efetua a remoção rearranjando o vetor dentro do nó.
            for (int j = i; j < no.nroChaves - 1; j++) {
                no.chaves[j]    = no.chaves[j + 1];
                no.ponteiros[j] = no.ponteiros[j + 1];
            }
            no.chaves[no.nroChaves - 1]    = -1;
            no.ponteiros[no.nroChaves - 1] = -1;
            no.nroChaves--;
            writeBinaryNode(&no, file, rrnAtual);
        }
    } else {
        // CONTINUAÇÃO DA DESCIDA RECURSIVA
        int underflow = removerRecursivo(file, no.filhos[i], rrnAtual,
                                          i, chave, header);
        if (!underflow)
            return 0; // Subárvore inferior estabilizou
    }

    // ANÁLISE DE UNDERFLOW NA SUBIDA DA RECURSÃO
    readBinaryNode(&no, file, rrnAtual);
    int minChaves = (4 / 2) - 1; // Taxa de ocupação mínima para m=4 é 1 chave por nó
    
    if (no.nroChaves >= minChaves || rrnPai == -1)
        return 0; // Sem problemas de ocupação local, ou atingiu a raiz da árvore

    binaryNode pai;
    readBinaryNode(&pai, file, rrnPai);

    // TRATAMENTO DE UNDERFLOW
    // Tenta redistribuição com o irmão adjacente à direita
    if (indiceNoPai < pai.nroChaves) {
        binaryNode irmaoDir;
        readBinaryNode(&irmaoDir, file, pai.filhos[indiceNoPai + 1]);
        if (irmaoDir.nroChaves > minChaves) {
            redistribuir(file, rrnPai, indiceNoPai, 0);
            return 0; // Balanceado com sucesso
        }
    }

    // Tenta redistribuição com o irmão adjacente à esquerda
    if (indiceNoPai > 0) {
        binaryNode irmaoEsq;
        readBinaryNode(&irmaoEsq, file, pai.filhos[indiceNoPai - 1]);
        if (irmaoEsq.nroChaves > minChaves) {
            redistribuir(file, rrnPai, indiceNoPai, 1);
            return 0; // Balanceado com sucesso
        }
    }

    // Concatenação com irmão esquerdo
    if (indiceNoPai > 0)
        return concatenar(file, rrnPai, indiceNoPai, header);

    // Concatenação com irmão direito
    return concatenar(file, rrnPai, indiceNoPai + 1, header);
}