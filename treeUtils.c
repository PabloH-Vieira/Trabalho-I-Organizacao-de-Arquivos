#include "treeUtils.h"
#include "binaryTree.h"
#define TAMANHO_CABECALHO 17
#define TAMANHO_NO 53
#define ORDEM 4        // m = 4, máximo de 4 filhos por nó
#define MAX_CHAVES 3   // máximo de 3 chaves por nó

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


void splitNode(FILE *file, binaryNode *noEsq, int rrnEsq, int chaveNova, int ptrNova, int filhoNovoDireita,
                      int *chavePromovida, int *ptrPromovido, int *rrnNovoDireita, binaryHeader *header) {

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
    binaryNode noDir;
    createEmptyBinaryNode(&noDir);
    noDir.tipoNo    = noEsq->tipoNo; // mesmo tipo (folha ou interno)
    noDir.nroChaves = MAX_CHAVES - meio; // 1 chave

    for (int j = 0; j < noDir.nroChaves; j++) {
        noDir.chaves[j]    = chaves[meio + 1 + j];
        noDir.ponteiros[j] = ptrs[meio + 1 + j];
        noDir.filhos[j]    = filhos[meio + 1 + j];
    }
    noDir.filhos[noDir.nroChaves] = filhos[ORDEM];

    // aloca RRN para o nó direito e escreve no arquivo
    *rrnNovoDireita = alocarRRN(file, header);
    writeBinaryNode(&noDir, file, *rrnNovoDireita);
    writeBinaryNode(noEsq, file, rrnEsq);
}

int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, int *chavePromovida, int *ptrPromovido,
                            int *rrnDireita, binaryHeader *header) {
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

void insertKey(FILE *file, int rrnRegistro, int chave, binaryHeader *header) {
    int chavePromovida, ptrPromovido, rrnDireita;

    // árvore vazia — cria a primeira folha que também é a raiz
    if (header->noRaiz == -1) {
        binaryNode raiz;
        createEmptyBinaryNode(&raiz);
        raiz.tipoNo      = -1; // folha e raiz ao mesmo tempo
        raiz.chaves[0]   = chave;
        raiz.ponteiros[0] = rrnRegistro;
        raiz.nroChaves   = 1;

        int rrn = alocarRRN(file, header);
        header->noRaiz = rrn;
        writeBinaryNode(&raiz, file, rrn);
        return;
    }

    int houveSplit = inserirRecursivo(file, header->noRaiz, chave, rrnRegistro,
                                      &chavePromovida, &ptrPromovido, &rrnDireita,
                                      header);

    if (houveSplit) {
        // split chegou até a raiz — cria uma nova raiz
        binaryNode novaRaiz;
        createEmptyBinaryNode(&novaRaiz);
        novaRaiz.tipoNo      = 0; // raiz interna
        novaRaiz.chaves[0]   = chavePromovida;
        novaRaiz.ponteiros[0] = ptrPromovido;
        novaRaiz.filhos[0]   = header->noRaiz;
        novaRaiz.filhos[1]   = rrnDireita;
        novaRaiz.nroChaves   = 1;

        int rrnNovaRaiz = alocarRRN(file, header);
        header->noRaiz  = rrnNovaRaiz;

        // o nó que era raiz vira intermediário
        binaryNode noAntigo;
        readBinaryNode(&noAntigo, file, novaRaiz.filhos[0]);
        if (noAntigo.tipoNo == -1)
            noAntigo.tipoNo = 1; // era folha+raiz, agora só folha (intermediário)
        writeBinaryNode(&noAntigo, file, novaRaiz.filhos[0]);
        writeBinaryNode(&novaRaiz, file, rrnNovaRaiz);
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
}