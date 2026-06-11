#include "treeUtils.h"
#include "binaryTree.h"
#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4        // m = 4, máximo de 4 filhos por nó
#define MAX_CHAVES 3   // máximo de 3 chaves por nó
#define PROMOTION 1
#define NO_PROMOTION 0
#define ERROR -1

int alocarRRN(FILE *file, binaryHeader *header) {
    int rrn;
    if (header->topo != -1) {
        // reaproveta o topo da pilha de removidos
        rrn = header->topo;
        binaryNode topo;
        readBinaryNode(&topo, file, rrn);
        header->topo = topo.proximo;
    } else {
        rrn = header->proxRRN;
        header->proxRRN++;
    }
    header->nroNos++;
    return rrn;
}

void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita) {
    int i = node->nroChaves - 1;

    // desloca as chaves maiores para abrir espaço
    while (i >= 0 && chave < node->chaves[i]) {
        node->chaves[i + 1]   = node->chaves[i];
        node->ponteiros[i + 1] = node->ponteiros[i];
        node->filhos[i + 2]   = node->filhos[i + 1];
        i--;
    }

    node->chaves[i + 1] = chave;
    node->ponteiros[i + 1] = ptr;
    node->filhos[i + 2] = filhoDireita;
    node->nroChaves++;
}

/*
void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq, int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita, binaryHeader *header) {

    // vetor temporário com as 4 chaves (3 antigas + 1 nova)
    int tempChaves[ORDEM], tempPonteiros[ORDEM], tempFilhos[ORDEM + 1];

    // copia o conteúdo atual do nó para os vetores temporários
    for (int i = 0; i < 3; i++) {
        tempChaves[i] = noEsq->chaves[i];
        tempPonteiros[i] = noEsq->ponteiros[i];
        tempFilhos[i] = noEsq->filhos[i];
    }
    tempFilhos[3] = noEsq->filhos[3];

    // insere a nova chave nos vetores temporários mantendo a ordem
    int i = 2;
    
    while (i >= 0 && tempChaves[i] > chaveNova) {
        tempChaves[i + 1] = tempChaves[i];
        tempPonteiros[i + 1] = tempPonteiros[i];
        tempFilhos[i + 2] = tempFilhos[i + 1];
        i--;
    }

    // Insere a nova chave na posição correta
    tempChaves[i + 1] = chaveNova;
    tempPonteiros[i + 1] = ptrNova;
    tempFilhos[i + 2] = filhoNovoDireita;

    // com m=4: esquerdo fica com 2 chaves, promove índice 2, direito fica com 1
    *chavePromovida = tempChaves[2];
    *ptrPromovido   = tempPonteiros[2];

    // reconstrói o nó esquerdo com as chaves antes do meio
    noEsq->nroChaves = 2;
    for (int j = 0; j < 2; j++) {
        noEsq->chaves[j]    = tempChaves[j];
        noEsq->ponteiros[j] = tempPonteiros[j];
        noEsq->filhos[j]    = tempFilhos[j];
    }
    noEsq->filhos[2] = tempFilhos[2];

    // Inserindo valores padrões para a chave removida do nó esquerdo
    noEsq->chaves[2] = -1;
    noEsq->ponteiros[2] = -1;
    noEsq->filhos[3] = -1; 


    // cria o nó direito com as chaves depois do meio
    binaryNode noDir;
    createEmptyBinaryNode(&noDir);
    noDir.tipoNo = noEsq->tipoNo; // mesmo tipo (folha ou interno)
    if (noDir.tipoNo == 0)
        noDir.tipoNo = 1;
    noDir.nroChaves = 1;
    noDir.chaves[0] = tempChaves[3];
    noDir.ponteiros[0] = tempPonteiros[3];
    noDir.filhos[0] = tempFilhos[3];
    noDir.filhos[1] = tempFilhos[4];

    // aloca RRN para o nó direito e escreve no arquivo
    *rrnNovoDireita = alocarRRN(file, header);
    writeBinaryNode(&noDir, file, *rrnNovoDireita);
    writeBinaryNode(noEsq, file, rrnEsq);
}
*/


/*
int insertRecursive(FILE *file, binaryHeader *header, int rrnAtual, int chave, int posKey, int* promotionKey,
                     int* posPromotion, int *promotionRightChild){
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
*/

int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, 
                    int *promotionKey, int *promotionPtr, int *promotionRightChild, binaryHeader *header) {
    
    // Caso Base: ultrapassou um nó folha, promove para o nó folha da recursão anterior
    if (rrnAtual == -1) {
        *promotionKey = chave;
        *promotionPtr = ptr;
        *promotionRightChild = -1;
        return PROMOTION; 
    }
    
    // Carrega a página do nó atual
    binaryNode node;
    readBinaryNode(&node, file, rrnAtual);

    // Busca linear simples para encontrar o ponteiro de descida correto
    int i = 0;
    while (i < node.nroChaves && chave > node.chaves[i]) {
        i++;
    }

    // Verificação para evitar chave duplicada
    if (i < node.nroChaves && chave == node.chaves[i]) {
        return ERROR; 
    }

    // Variáveis locais para capturar o que sobe do andar de baixo
    int promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow;
    
    // Descida recursiva
    int result = inserirRecursivo(file, node.filhos[i], chave, ptr, 
                                 &promotionKeyBelow, &promotionPtrBelow, &promotionRightChildBelow, header);

    // Se a recursão voltou dizendo que não houve quebra de nó, ou deu erro, encerra aqui.
    if (result == NO_PROMOTION || result == ERROR) {
        return result; 
    }
    
    // --- TRATAMENTO DO RETORNO: O andar de baixo explodiu e mandou uma chave para cima ---

    if (node.nroChaves < 3) {
        // TEM ESPAÇO: Insere a chave promovida neste nó.
        // A função inserirNoNo faz o shift (Insertion Sort disfarçado) sozinha!
        inserirNoNo(&node, promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow);
        
        // Grava o nó modificado de volta e avisa o andar de cima que estabilizou
        writeBinaryNode(&node, file, rrnAtual);
        return NO_PROMOTION; 
    } 
    else {
        // OVERFLOW NESTE ANDAR TAMBÉM: Cascata de Splits!
        // O splitNode faz a divisão, cria a nova página, salva as DUAS no disco 
        // e já preenche as variáveis da promoção para o andar superior.
        splitNode(file, &node, rrnAtual, promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow,
                  promotionKey, promotionPtr, promotionRightChild, header);
                  
        return PROMOTION; // Avisa o andar de cima que este nó também explodiu
    }
}

void insertKey(FILE *file, int byteOffsetRegistro, int chave, binaryHeader *header) {
    int chavePromovida, ptrPromovido, rrnDireita;

    // Caso de árvore vazia: cria a primeira folha que também é a raiz
    if (header->noRaiz == -1) { 
        binaryNode raiz;
        createEmptyBinaryNode(&raiz);
        //raiz.tipoNo = -1; // folha e raiz ao mesmo tempo
        raiz.tipoNo = 0;
        raiz.chaves[0] = chave;
        raiz.ponteiros[0] = byteOffsetRegistro;
        raiz.nroChaves = 1;

        int rrn = alocarRRN(file, header);
        header->noRaiz = rrn;
        writeBinaryNode(&raiz, file, rrn);
        return;
    }

    // Caso que a árvore já existe: insere recursivamente
    int houveSplit = inserirRecursivo(file, header->noRaiz, chave, byteOffsetRegistro,
                                      &chavePromovida, &ptrPromovido, &rrnDireita,
                                      header);

    if (houveSplit == PROMOTION) {
        // split chegou até a raiz: cria uma nova raiz
        binaryNode novaRaiz;    
        createEmptyBinaryNode(&novaRaiz);
        novaRaiz.tipoNo      = 0; // raiz interna
        novaRaiz.chaves[0]   = chavePromovida;
        novaRaiz.ponteiros[0] = ptrPromovido;
        novaRaiz.filhos[0]   = header->noRaiz;
        novaRaiz.filhos[1]   = rrnDireita;
        novaRaiz.nroChaves   = 1;

        int rrnNovaRaiz = alocarRRN(file, header);

        // o nó que era raiz vira intermediário
        //binaryNode noAntigo;
        //readBinaryNode(&noAntigo, file, header -> noRaiz);
        //if (noAntigo.tipoNo == 0)
            //noAntigo.tipoNo = 1; // Era raiz com filhos, agora é intermediário

        //writeBinaryNode(&noAntigo, file, header -> noRaiz);
        writeBinaryNode(&novaRaiz, file, rrnNovaRaiz);
        header -> noRaiz = rrnNovaRaiz;
        writeBinaryHeader(header, file);
    }
}

int searchKey(FILE *file, int chave, binaryHeader *header) {
    int rrnAtual = header->noRaiz;

    while (rrnAtual != -1) {
        binaryNode no;
        readBinaryNode(&no, file, rrnAtual);

        int i = 0;
        while (i < no.nroChaves && chave > no.chaves[i])
            i++;

        if (i < no.nroChaves && chave == no.chaves[i])
            return no.ponteiros[i]; // achou — retorna o RRN do registro

        // desce pelo filho correto
        rrnAtual = no.filhos[i];
    }

    return -1; // não encontrou
}

void empilharNoRemovido(FILE *file, int rrn, binaryHeader *header) {
    binaryNode no;
    readBinaryNode(&no, file, rrn);
    no.removido = '1';
    no.proximo  = header->topo;
    header->topo = rrn;
    header->nroNos--;
    writeBinaryNode(&no, file, rrn);
}

int encontrarSucessor(FILE *file, int rrnFilhoDireita, int *chaveSucc, int *ptrSucc) {
    int rrnAtual = rrnFilhoDireita;
    binaryNode no;

    while (1) {
        readBinaryNode(&no, file, rrnAtual);
        if (no.filhos[0] == -1)
            break; // chegou na folha
        rrnAtual = no.filhos[0];
    }

    *chaveSucc = no.chaves[0];
    *ptrSucc   = no.ponteiros[0];
    return rrnAtual;
}

void redistribuir(FILE *file, int rrnPai, int indiceFilho, int lado) {
    binaryNode pai, noEsq, noDir;
    readBinaryNode(&pai, file, rrnPai);

    int rrnEsq, rrnDir, indiceSeparador;

    if (lado == 0) {
        // redistribuição com irmão direito
        rrnEsq         = pai.filhos[indiceFilho];
        rrnDir         = pai.filhos[indiceFilho + 1];
        indiceSeparador = indiceFilho;
    } else {
        // redistribuição com irmão esquerdo
        rrnEsq         = pai.filhos[indiceFilho - 1];
        rrnDir         = pai.filhos[indiceFilho];
        indiceSeparador = indiceFilho - 1;
    }

    readBinaryNode(&noEsq, file, rrnEsq);
    readBinaryNode(&noDir, file, rrnDir);

    // junta tudo num vetor temporário para redistribuir uniformemente
    int totalChaves = noEsq.nroChaves + 1 + noDir.nroChaves;
    int chaves[7], ptrs[7], filhos[8];
    int k = 0, f = 0;

    for (int i = 0; i < noEsq.nroChaves; i++) {
        chaves[k]  = noEsq.chaves[i];
        ptrs[k]    = noEsq.ponteiros[i];
        filhos[f]  = noEsq.filhos[i];
        k++; f++;
    }
    filhos[f++] = noEsq.filhos[noEsq.nroChaves];

    // chave separadora do pai entra no meio
    chaves[k]  = pai.chaves[indiceSeparador];
    ptrs[k]    = pai.ponteiros[indiceSeparador];
    k++;

    for (int i = 0; i < noDir.nroChaves; i++) {
        chaves[k]  = noDir.chaves[i];
        ptrs[k]    = noDir.ponteiros[i];
        filhos[f]  = noDir.filhos[i];
        k++; f++;
    }
    filhos[f] = noDir.filhos[noDir.nroChaves];

    // divide uniformemente — esquerdo fica com uma chave a mais se ímpar
    int metadeEsq = totalChaves / 2;
    int meio      = metadeEsq; // índice da nova chave separadora

    // reconstrói nó esquerdo
    noEsq.nroChaves = metadeEsq;
    for (int i = 0; i < MAX_CHAVES; i++) {
        noEsq.chaves[i]    = (i < metadeEsq) ? chaves[i] : -1;
        noEsq.ponteiros[i] = (i < metadeEsq) ? ptrs[i]   : -1;
        noEsq.filhos[i]    = (i <= metadeEsq) ? filhos[i] : -1;
    }
    noEsq.filhos[MAX_CHAVES] = -1;

    // nova chave separadora sobe para o pai
    pai.chaves[indiceSeparador]    = chaves[meio];
    pai.ponteiros[indiceSeparador] = ptrs[meio];

    // reconstrói nó direito com o restante
    int qtdDir = totalChaves - metadeEsq - 1;
    noDir.nroChaves = qtdDir;
    for (int i = 0; i < MAX_CHAVES; i++) {
        noDir.chaves[i]    = (i < qtdDir) ? chaves[meio + 1 + i] : -1;
        noDir.ponteiros[i] = (i < qtdDir) ? ptrs[meio + 1 + i]   : -1;
        noDir.filhos[i]    = (i <= qtdDir) ? filhos[metadeEsq + 1 + i] : -1;
    }
    noDir.filhos[MAX_CHAVES] = -1;

    writeBinaryNode(&pai,   file, rrnPai);
    writeBinaryNode(&noEsq, file, rrnEsq);
    writeBinaryNode(&noDir, file, rrnDir);
}

int concatenar(FILE *file, int rrnPai, int indiceFilho, binaryHeader *header) {
    binaryNode pai, noEsq, noDir;
    readBinaryNode(&pai, file, rrnPai);

    // concatena sempre com o irmão esquerdo
    int rrnEsq         = pai.filhos[indiceFilho - 1];
    int rrnDir         = pai.filhos[indiceFilho];
    int indiceSeparador = indiceFilho - 1;

    readBinaryNode(&noEsq, file, rrnEsq);
    readBinaryNode(&noDir, file, rrnDir);

    // chave separadora do pai desce para o nó esquerdo
    noEsq.chaves[noEsq.nroChaves]    = pai.chaves[indiceSeparador];
    noEsq.ponteiros[noEsq.nroChaves] = pai.ponteiros[indiceSeparador];
    noEsq.filhos[noEsq.nroChaves + 1] = noDir.filhos[0];
    noEsq.nroChaves++;

    // copia todas as chaves do nó direito para o esquerdo
    for (int i = 0; i < noDir.nroChaves; i++) {
        noEsq.chaves[noEsq.nroChaves]    = noDir.chaves[i];
        noEsq.ponteiros[noEsq.nroChaves] = noDir.ponteiros[i];
        noEsq.filhos[noEsq.nroChaves + 1] = noDir.filhos[i + 1];
        noEsq.nroChaves++;
    }

    // remove a chave separadora do pai e ajusta os filhos
    for (int i = indiceSeparador; i < pai.nroChaves - 1; i++) {
        pai.chaves[i]    = pai.chaves[i + 1];
        pai.ponteiros[i] = pai.ponteiros[i + 1];
        pai.filhos[i + 1] = pai.filhos[i + 2];
    }
    pai.chaves[pai.nroChaves - 1]    = -1;
    pai.ponteiros[pai.nroChaves - 1] = -1;
    pai.filhos[pai.nroChaves]        = -1;
    pai.nroChaves--;

    writeBinaryNode(&noEsq, file, rrnEsq);
    writeBinaryNode(&pai,   file, rrnPai);

    // o nó direito é destruído — empilha como removido
    empilharNoRemovido(file, rrnDir, header);

    // retorna se o pai ficou com underflow
    return (pai.nroChaves < (ORDEM / 2) - 1);
}

int removerRecursivo(FILE *file, int rrnAtual, int rrnPai, int indiceNoPai, int chave, binaryHeader *header) {
    if (rrnAtual == -1)
        return 0; // chave não encontrada

    binaryNode no;
    readBinaryNode(&no, file, rrnAtual);

    int i = 0;
    while (i < no.nroChaves && chave > no.chaves[i])
        i++;

    if (i < no.nroChaves && chave == no.chaves[i]) {
        // encontrou a chave
        if (no.filhos[0] != -1) {
            // nó interno — troca pela sucessora imediata (menor da subárvore direita)
            int chaveSucc, ptrSucc;
            encontrarSucessor(file, no.filhos[i + 1], &chaveSucc, &ptrSucc);

            no.chaves[i]    = chaveSucc;
            no.ponteiros[i] = ptrSucc;
            writeBinaryNode(&no, file, rrnAtual);

            // agora remove a sucessora da folha onde ela estava
            int underflow = removerRecursivo(file, no.filhos[i + 1], rrnAtual,
                                              i + 1, chaveSucc, header);
            if (!underflow)
                return 0;
        } else {
            // nó folha — remove diretamente deslocando as chaves
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
        // não encontrou aqui — desce pelo filho correto
        int underflow = removerRecursivo(file, no.filhos[i], rrnAtual,
                                          i, chave, header);
        if (!underflow)
            return 0;
    }

    // verifica underflow no nó atual (mínimo é ceil(m/2) - 1 = 1 chave)
    readBinaryNode(&no, file, rrnAtual);
    int minChaves = (ORDEM / 2) - 1;
    if (no.nroChaves >= minChaves || rrnPai == -1)
        return 0; // sem underflow, ou é a raiz

    binaryNode pai;
    readBinaryNode(&pai, file, rrnPai);

    // tenta redistribuir com irmão direito primeiro
    if (indiceNoPai < pai.nroChaves) {
        binaryNode irmaoDir;
        readBinaryNode(&irmaoDir, file, pai.filhos[indiceNoPai + 1]);
        if (irmaoDir.nroChaves > minChaves) {
            redistribuir(file, rrnPai, indiceNoPai, 0);
            return 0;
        }
    }

    // tenta redistribuir com irmão esquerdo
    if (indiceNoPai > 0) {
        binaryNode irmaoEsq;
        readBinaryNode(&irmaoEsq, file, pai.filhos[indiceNoPai - 1]);
        if (irmaoEsq.nroChaves > minChaves) {
            redistribuir(file, rrnPai, indiceNoPai, 1);
            return 0;
        }
    }

    // redistribuição não foi possível — concatena com irmão esquerdo
    if (indiceNoPai > 0)
        return concatenar(file, rrnPai, indiceNoPai, header);

    // se não tem irmão esquerdo, concatena com o direito
    // (empurra o nó atual para a esquerda do par)
    return concatenar(file, rrnPai, indiceNoPai + 1, header);
}

void removeKey(FILE *file, int chave, binaryHeader *header) {
    if (header->noRaiz == -1)
        return; // árvore vazia

    removerRecursivo(file, header->noRaiz, -1, -1, chave, header);

    // se a raiz ficou sem chaves após uma concatenação, a árvore diminui de altura
    binaryNode raiz;
    readBinaryNode(&raiz, file, header->noRaiz);
    if (raiz.nroChaves == 0 && raiz.filhos[0] != -1) {
        int rrnRaizVelha = header->noRaiz;
        header->noRaiz   = raiz.filhos[0];
        empilharNoRemovido(file, rrnRaizVelha, header);

        // atualiza tipo do novo nó raiz
        binaryNode novaRaiz;
        readBinaryNode(&novaRaiz, file, header->noRaiz);
        novaRaiz.tipoNo = (novaRaiz.filhos[0] == -1) ? -1 : 0;
        writeBinaryNode(&novaRaiz, file, header->noRaiz);
    }
    
    // salva o cabeçalho atualizado no arquivo (topo, nroNos e noRaiz podem ter mudado)
    writeBinaryHeader(header, file);
}

/*
void splitNode(FILE *file, binaryNode *p_oldpage, int rrn_oldpage, int key, int ponteiro, int r_child, int *promo_key, int *promo_ponteiro, int *promo_r_child, binaryHeader *header) {
    // 1. Criação dos Arrays de Trabalho (Work Arrays) com tamanho MAXKEYS + 1
    int workchaves[4];       
    int workponteiros[4];    // Acompanha a chave de dados de forma paralela
    int workfilhos[5];       // Tamanho MAXKEYS + 2 para os RRNs dos filhos

    // Copia todos os dados do nó cheio para os arrays de trabalho
    int i;
    for (i = 0; i < 3; i++) {
        workchaves[i] = p_oldpage->chaves[i];
        workponteiros[i] = p_oldpage->ponteiros[i];
        workfilhos[i] = p_oldpage->filhos[i];
    }
    workfilhos[i] = p_oldpage->filhos[i]; // Copia o último filho (índice 3)

    // 2. Inserção Ordenada da nova chave transbordada (Algoritmo do slide)
    // Varre de trás para frente empurrando os elementos maiores para abrir espaço
    for (i = 3; (key < workchaves[i - 1]) && (i > 0); i--) {
        workchaves[i] = workchaves[i - 1];
        workponteiros[i] = workponteiros[i - 1];
        workfilhos[i + 1] = workfilhos[i];
    }
    // Insere a chave e seus respectivos ponteiros na posição correta 'i'
    workchaves[i] = key;
    workponteiros[i] = ponteiro;
    workfilhos[i + 1] = r_child;

    // 3. Inicialização da nova página (Equivalente ao getpage() e pageinit())
    *promo_r_child = alocarRRN(file, header);
    binaryNode p_newpage;
    createEmptyBinaryNode(&p_newpage);
        if (p_oldpage->tipoNo == 0) {
        // Se era a raiz, os dois nós resultantes deixam de ser raiz
        if (p_oldpage->filhos[0] == -1) {
            p_oldpage->tipoNo = -1; // Viram folhas comuns
            p_newpage.tipoNo = -1;
        } else {
            p_oldpage->tipoNo = 1;  // Viram nós internos comuns
            p_newpage.tipoNo = 1;
        }
    } else {
        // Se não era raiz, o da direita apenas herda o tipo do esquerdo
        p_newpage.tipoNo = p_oldpage->tipoNo;
    }
    //p_newpage.tipoNo = p_oldpage->tipoNo; // Herda o tipo (se era folha -1 ou interno 1)

    // 4. Distribuição das Chaves (Regra: Esquerdo com 2, Promove 1, Direito com 1)
    
    // --- Configuração do Nó Esquerdo (p_oldpage) ---
    p_oldpage->chaves[0] = workchaves[0];
    p_oldpage->ponteiros[0] = workponteiros[0];
    p_oldpage->filhos[0] = workfilhos[0];

    p_oldpage->chaves[1] = workchaves[1];
    p_oldpage->ponteiros[1] = workponteiros[1];
    p_oldpage->filhos[1] = workfilhos[1];
    
    p_oldpage->filhos[2] = workfilhos[2];

    // Limpeza crucial dos resíduos antigos (fantasmas) para não quebrar o Checksum
    p_oldpage->chaves[2] = -1;
    p_oldpage->ponteiros[2] = -1;
    p_oldpage->filhos[3] = -1;
    p_oldpage->nroChaves = 2;

    // --- Configuração da Chave Promovida ---
    *promo_key = workchaves[2];
    *promo_ponteiro = workponteiros[2];

    // --- Configuração do Nó Direito (p_newpage) ---
    p_newpage.chaves[0] = workchaves[3];
    p_newpage.ponteiros[0] = workponteiros[3];
    p_newpage.filhos[0] = workfilhos[3];
    p_newpage.filhos[1] = workfilhos[4];
    p_newpage.nroChaves = 1;
    writeBinaryNode(&p_newpage, file, *promo_r_child);
    writeBinaryNode(p_oldpage, file, rrn_oldpage);
    writeBinaryHeader(header, file);
}
*/

void splitNode(FILE *file, binaryNode *p_oldpage, int rrn_oldpage, 
               int key, int ponteiro, int r_child, 
               int *promo_key, int *promo_ponteiro, int *promo_r_child, 
               binaryHeader *header) {
    
    // 1. Criação dos Arrays de Trabalho (Tamanho MAXKEYS + 1)
    int workchaves[4];       
    int workponteiros[4];    
    int workfilhos[5];       // 5 filhos possíveis para 4 chaves temporárias

    // Copia os dados atuais do nó cheio para os arrays de trabalho
    int i;
    for (i = 0; i < 3; i++) {
        workchaves[i] = p_oldpage->chaves[i];
        workponteiros[i] = p_oldpage->ponteiros[i];
        workfilhos[i] = p_oldpage->filhos[i];
    }
    workfilhos[3] = p_oldpage->filhos[3]; // Copia o quarto e último filho original

    // 2. Inserção Ordenada da nova chave transbordada (Algoritmo do material base)
    // Varre de trás para frente arrastando elementos maiores para abrir o espaço correto
    for (i = 3; (key < workchaves[i - 1]) && (i > 0); i--) {
        workchaves[i] = workchaves[i - 1];
        workponteiros[i] = workponteiros[i - 1];
        workfilhos[i + 1] = workfilhos[i];
    }
    // Insere a chave promotora vinda de baixo e seu respectivo filho direito síncronos
    workchaves[i] = key;
    workponteiros[i] = ponteiro;
    workfilhos[i + 1] = r_child;

    // 3. Alocação de metadados para a nova página da direita
    // Se você tiver uma função auxiliar "alocarRRN", pode usá-la aqui:
    *promo_r_child = alocarRRN(file, header);               

    binaryNode p_newpage;
    createEmptyBinaryNode(&p_newpage);

    // 4. Ajuste fino do tipoNo (Evita o bug de múltiplos nós raízes fantasmas)
    if (p_oldpage->tipoNo == 0) {
        if (p_oldpage->filhos[0] == -1) {
            p_oldpage->tipoNo = -1; // Antiga raiz vira folha comum
            p_newpage.tipoNo = -1;  // Nova página da direita vira folha comum
        } else {
            p_oldpage->tipoNo = 1;  // Antiga raiz vira nó interno comum
            p_newpage.tipoNo = 1;   // Nova página da direita vira nó interno comum
        }
    } else {
        p_newpage.tipoNo = p_oldpage->tipoNo; // Se não era raiz, herda o tipo normalmente
    }

    // 5. Distribuição das Chaves (Regra m=4: Esquerdo com 2, Promove a 3ª, Direito com 1)
    
    // --- Configuração do Nó Esquerdo (p_oldpage) ---
    p_oldpage->chaves[0] = workchaves[0];
    p_oldpage->ponteiros[0] = workponteiros[0];
    p_oldpage->filhos[0] = workfilhos[0];

    p_oldpage->chaves[1] = workchaves[1];
    p_oldpage->ponteiros[1] = workponteiros[1];
    p_oldpage->filhos[1] = workfilhos[1];
    
    p_oldpage->filhos[2] = workfilhos[2];

    // Limpeza crucial dos resíduos físicos (fantasmas) para não corromper o Checksum
    p_oldpage->chaves[2] = -1;
    p_oldpage->ponteiros[2] = -1;
    p_oldpage->filhos[3] = -1;
    p_oldpage->nroChaves = 2;

    // --- Configuração das Variáveis de Promoção para o Andar de Cima ---
    *promo_key = workchaves[2];
    *promo_ponteiro = workponteiros[2];

    // --- Configuração do Nó Direito (p_newpage) ---
    p_newpage.chaves[0] = workchaves[3];
    p_newpage.ponteiros[0] = workponteiros[3];
    p_newpage.filhos[0] = workfilhos[3];
    p_newpage.filhos[1] = workfilhos[4];
    p_newpage.nroChaves = 1;

    // 6. Persistência Física no Arquivo de Índices
    writeBinaryNode(p_oldpage, file, rrn_oldpage);
    writeBinaryNode(&p_newpage, file, *promo_r_child);
}