#include "treeUtils.h"

#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4        // m = 4, máximo de 4 filhos por nó
#define MAX_CHAVES 3   // máximo de 3 chaves por nó

/*
 * Retorna o RRN onde um novo nó vai ser escrito.
 * Se houver nó removido na pilha, reaproveit ele.
 * Caso contrário, usa o proxRRN e incrementa.
 */
static int alocarRRN(FILE *file, binaryHeader *header) {
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

/*
 * Insere a chave e o ponteiro para o registro na posição correta
 * dentro do nó, mantendo a ordem crescente. Também empurra o
 * filho à direita para a posição certa caso venha de um split.
 */
static void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita) {
    int i = node->nroChaves - 1;

    // desloca as chaves maiores para abrir espaço
    while (i >= 0 && node->chaves[i] > chave) {
        node->chaves[i + 1]   = node->chaves[i];
        node->ponteiros[i + 1] = node->ponteiros[i];
        node->filhos[i + 2]   = node->filhos[i + 1];
        i--;
    }

    node->chaves[i + 1]    = chave;
    node->ponteiros[i + 1] = ptr;
    node->filhos[i + 2]    = filhoDireita;
    node->nroChaves++;
}

/*
 * Faz o split de um nó cheio. Com m=4, cada nó tem no máximo 3 chaves.
 * Quando um nó estoura (teria 4 chaves), dividimos assim:
 *   - nó esquerdo fica com 2 chaves (o nó original, modificado)
 *   - chave do meio é promovida para o pai
 *   - nó direito (novo) fica com 1 chave
 *
 * A chave promovida é a primeira do nó direito, conforme especificação.
 */
static void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq,
                      int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita,
                      binaryHeader *header) {

    // vetor temporário com as 4 chaves (3 antigas + 1 nova)
    int chaves[ORDEM],    ptrs[ORDEM],    filhos[ORDEM + 1];

    // copia o conteúdo atual do nó para os vetores temporários
    for (int i = 0; i < MAX_CHAVES; i++) {
        chaves[i]  = noEsq->chaves[i];
        ptrs[i]    = noEsq->ponteiros[i];
        filhos[i]  = noEsq->filhos[i];
    }
    filhos[MAX_CHAVES] = noEsq->filhos[MAX_CHAVES];

    // insere a nova chave nos vetores temporários mantendo a ordem
    int i = MAX_CHAVES - 1;
    while (i >= 0 && chaves[i] > chaveNova) {
        chaves[i + 1]  = chaves[i];
        ptrs[i + 1]    = ptrs[i];
        filhos[i + 2]  = filhos[i + 1];
        i--;
    }
    chaves[i + 1]  = chaveNova;
    ptrs[i + 1]    = ptrNova;
    filhos[i + 2]  = filhoNovoDireita;

    // com m=4: esquerdo fica com 2 chaves, promove índice 2, direito fica com 1
    int meio = 2; // índice da chave promovida (primeira do nó direito)

    *chavePromovida = chaves[meio];
    *ptrPromovido   = ptrs[meio];

    // reconstrói o nó esquerdo com as chaves antes do meio
    noEsq->nroChaves = meio; // 2 chaves
    for (int j = 0; j < meio; j++) {
        noEsq->chaves[j]    = chaves[j];
        noEsq->ponteiros[j] = ptrs[j];
        noEsq->filhos[j]    = filhos[j];
    }
    noEsq->filhos[meio] = filhos[meio];
    // limpa as posições que sobraram
    for (int j = meio; j < MAX_CHAVES; j++) {
        noEsq->chaves[j]    = -1;
        noEsq->ponteiros[j] = -1;
    }
    noEsq->filhos[MAX_CHAVES] = -1;

    // cria o nó direito com as chaves depois do meio
    binaryNode *noDir = createEmptyBinaryNode();
    noDir->tipoNo    = noEsq->tipoNo; // mesmo tipo (folha ou interno)
    noDir->nroChaves = MAX_CHAVES - meio; // 1 chave

    for (int j = 0; j < noDir->nroChaves; j++) {
        noDir->chaves[j]    = chaves[meio + 1 + j];
        noDir->ponteiros[j] = ptrs[meio + 1 + j];
        noDir->filhos[j]    = filhos[meio + 1 + j];
    }
    noDir->filhos[noDir->nroChaves] = filhos[ORDEM];

    // aloca RRN para o nó direito e escreve no arquivo
    *rrnNovoDireita = alocarRRN(file, header);
    writeBinaryNode(noDir, file, *rrnNovoDireita);
    writeBinaryNode(noEsq, file, rrnEsq);

    free(noDir);
}

/*
 * Inserção recursiva na árvore. Desce até a folha certa e sobe
 * promovendo chaves quando há overflow (split).
 *
 * Retorna 1 se houve promoção (split), 0 caso contrário.
 * A chave/ptr/filho promovidos são retornados pelos ponteiros.
 */
static int inserirRecursivo(FILE *file, int rrnAtual,
                            int chave, int ptr,
                            int *chavePromovida, int *ptrPromovido, int *rrnDireita,
                            binaryHeader *header) {
    // chegou numa subárvore vazia — promove a chave diretamente
    if (rrnAtual == -1) {
        *chavePromovida = chave;
        *ptrPromovido   = ptr;
        *rrnDireita     = -1;
        return 1;
    }

    binaryNode no;
    readBinaryNode(&no, file, rrnAtual);

    int chaveSubindo, ptrSubindo, rrnSubindo;
    int houveSplit = 0;

    if (no.filhos[0] == -1) {
        // nó folha — insere direto aqui se couber, ou promove
        chaveSubindo = chave;
        ptrSubindo   = ptr;
        rrnSubindo   = -1;
        houveSplit   = 1; // vai tentar inserir abaixo (caso base)
    } else {
        // nó interno — encontra o filho certo para descer
        int i = no.nroChaves - 1;
        while (i >= 0 && chave < no.chaves[i])
            i--;
        i++; // filho à direita da última chave menor

        houveSplit = inserirRecursivo(file, no.filhos[i], chave, ptr,
                                      &chaveSubindo, &ptrSubindo, &rrnSubindo,
                                      header);
    }

    if (!houveSplit)
        return 0;

    // tem chave para inserir neste nó
    if (no.nroChaves < MAX_CHAVES) {
        // cabe sem split
        inserirNoNo(&no, chaveSubindo, ptrSubindo, rrnSubindo);
        writeBinaryNode(&no, file, rrnAtual);
        return 0;
    }

    // nó cheio — faz split e promove
    splitNode(file, &no, rrnAtual,
              chaveSubindo, ptrSubindo, rrnSubindo,
              chavePromovida, ptrPromovido, rrnDireita,
              header);
    return 1;
}

/*
 * Ponto de entrada da inserção. Cuida do caso especial de split
 * na raiz (quando a raiz precisa ser substituída por uma nova raiz).
 */
void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header) {
    int chavePromovida, ptrPromovido, rrnDireita;

    // árvore vazia — cria a primeira folha que também é a raiz
    if (header->noRaiz == -1) {
        binaryNode *raiz = createEmptyBinaryNode();
        raiz->tipoNo      = -1; // folha e raiz ao mesmo tempo
        raiz->chaves[0]   = chave;
        raiz->ponteiros[0] = rrnRegistro;
        raiz->nroChaves   = 1;

        int rrn = alocarRRN(file, header);
        header->noRaiz = rrn;
        writeBinaryNode(raiz, file, rrn);
        free(raiz);
        return;
    }

    int houveSplit = inserirRecursivo(file, header->noRaiz, chave, rrnRegistro,
                                      &chavePromovida, &ptrPromovido, &rrnDireita,
                                      header);

    if (houveSplit) {
        // split chegou até a raiz — cria uma nova raiz
        binaryNode *novaRaiz = createEmptyBinaryNode();
        novaRaiz->tipoNo      = 0; // raiz interna
        novaRaiz->chaves[0]   = chavePromovida;
        novaRaiz->ponteiros[0] = ptrPromovido;
        novaRaiz->filhos[0]   = header->noRaiz;
        novaRaiz->filhos[1]   = rrnDireita;
        novaRaiz->nroChaves   = 1;

        int rrnNovaRaiz = alocarRRN(file, header);
        header->noRaiz  = rrnNovaRaiz;

        // o nó que era raiz vira intermediário
        binaryNode noAntigo;
        readBinaryNode(&noAntigo, file, novaRaiz->filhos[0]);
        if (noAntigo.tipoNo == -1)
            noAntigo.tipoNo = 1; // era folha+raiz, agora só folha (intermediário)
        writeBinaryNode(&noAntigo, file, novaRaiz->filhos[0]);

        writeBinaryNode(novaRaiz, file, rrnNovaRaiz);
        free(novaRaiz);
    }
}

/*
 * Busca uma chave na árvore a partir da raiz.
 * Retorna o RRN do registro no arquivo de dados, ou -1 se não encontrar.
 */
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