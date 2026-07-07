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
    
    // REAPROVEITAMENTO DA PILHA DE REMOVIDOS
    if (header->topo != -1){
        
        // Seleciona o RRN que está no topo da lista encadeada de removidos.
        rrn = header->topo;
        
        // Carrega temporariamente o nó removido da pilha na RAM para extrair 
        // qual é a próxima página disponível na ordem de encadeamento.
        binaryNode topo;
        readBinaryNode(&topo, file, rrn);
        
        // O cabeçalho da Árvore-B herda o ponteiro, removendo o nó atual da pilha
        header->topo = topo.proximo;
    } 
    else{
        // AUMENTO DO TAMANHO FÍSICO DO ARQUIVO
        // A pilha está vazia. Aloca o RRN ao final do arquivo 
        // e incrementa o campo de proxRRN para a próxima chamada.
        rrn = header->proxRRN;
        header->proxRRN++;
    }
    
    // ATUALIZAÇÃO DE CAMPO DE CONTAGEM
    header->nroNos++;
    
    return rrn; // Retorna o offset lógico pronto para a escrita da nova página
}


void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita) {
    // VARIÁVEL DE CURSOR (PROCESSAMENTO DA DIREITA PARA A ESQUERDA)
    // Inicia o processamento a partir da última chave válida atualmente presente no nó.
    int i = node->nroChaves - 1;

    // Percorre o nó de trás para frente enquanto encontrar chaves estritamente maiores 
    // que a nova chave a ser inserida.
    while (i >= 0 && chave < node->chaves[i]) {
        // Desloca a chave de busca para a direita
        node->chaves[i + 1]   = node->chaves[i];
        
        // Desloca o ponteiro de dados (PR) associado
        node->ponteiros[i + 1] = node->ponteiros[i];
        
        // Desloca o ponteiro da subárvore (P) correspondente à direita do elemento
        // Ocorre o ajuste do índice (+2 e +1) para preservar o alinhamento correto do layout.
        node->filhos[i + 2]   = node->filhos[i + 1];
        
        i--; // Retrocede o cursor para avaliar o elemento anterior
    }

    // Após abrir o "vazio" ordenado na posição correta (i + 1), os novos dados 
    // são injetados diretamente na estrutura de forma atômica.
    node->chaves[i + 1] = chave;
    node->ponteiros[i + 1] = ptr; // Registra o offset físico no arquivo de dados
    node->filhos[i + 2] = filhoDireita; // Vincula a subárvore da direita originada
    
    // ATUALIZAÇÃO DE CONTAGEM LOCAL
    node->nroChaves++;
}


void insertKey(FILE *file, int byteOffsetRegistro, int chave, binaryHeader *header) {
    int chavePromovida, ptrPromovido, rrnDireita;

    // CASO DE ÁRVORE-B TOTALMENTE VAZIA
    // Identifica a flag noRaiz == -1 para criar o primeiro nó do índice.
    if (header->noRaiz == -1) { 
        binaryNode raiz;
        createEmptyBinaryNode(&raiz);
        
        // Quando nó-folha = nó-raiz, o tipoNo deve ser -1.
        raiz.tipoNo = -1; 
        raiz.chaves[0] = chave;
        raiz.ponteiros[0] = byteOffsetRegistro; // Armazena o ponteiro de referência para o arquivo de dados
        raiz.nroChaves = 1;

        // Aloca um RRN livre (da pilha ou fim do arquivo) e atualiza campos do cabeçalho
        int rrn = alocarRRN(file, header);
        header->noRaiz = rrn; // A árvore deixa de ser vazia, apontando para o RRN recém-criado
        
        // Persiste a nova página folha/raiz de exatos 53 bytes no disco
        writeBinaryNode(&raiz, file, rrn);
        return;
    }

    // CASO DA ÁRVORE JÁ EXISTENTE (DESCIDA RECURSIVA)
    // Avalia o estouro de chaves (nroChaves > m-1) nas folhas e propaga os Splits para cima
    int houveSplit = inserirRecursivo(file, header->noRaiz, chave, byteOffsetRegistro,
                                      &chavePromovida, &ptrPromovido, &rrnDireita,
                                      header);

    // CASO DO TRATAMENTO DE SPLIT DO NÓ RAIZ
    // Se o estouro de nós atingiu o topo absoluto da árvore, a raiz anterior foi 
    // dividida em duas. É obrigatório criar uma nova página pai para unificá-las.
    if (houveSplit == PROMOTION) {
        binaryNode novaRaiz;    
        createEmptyBinaryNode(&novaRaiz);
        
        // Nova página é uma raiz com descendentes (tipoNo = 0).
        novaRaiz.tipoNo = 0; 
        novaRaiz.chaves[0] = chavePromovida; // Recebe o elemento promovido do split
        novaRaiz.ponteiros[0] = ptrPromovido;   // Vincula o byte offset associado à chave promovida
        
        // O filho da esquerda aponta para a raiz antiga. O filho da direita aponta 
        // para a nova página gerada à direita pelo split.
        novaRaiz.filhos[0]   = header->noRaiz;
        novaRaiz.filhos[1]   = rrnDireita;
        novaRaiz.nroChaves   = 1;

        // Reserva o RRN para persistência da nova página pai
        int rrnNovaRaiz = alocarRRN(file, header);

        // Escreve a nova raiz estruturada em disco
        writeBinaryNode(&novaRaiz, file, rrnNovaRaiz);
        
        // Redireciona o campo noRaiz do cabeçalho para apontar para o novo nó topo.
        header -> noRaiz = rrnNovaRaiz;
        writeBinaryHeader(header, file);
    }
}


int searchKey(FILE *file, int chave, binaryHeader *header) {
    // A busca inicia-se obrigatoriamente no nó raiz apontado pelo campo do cabeçalho.
    int rrnAtual = header->noRaiz;

    // O laço prossegue descendo pelos níveis da árvore até encontrar o registro ou 
    // receber o resultado -1, indicando que ultrapassou um nó folha.
    while (rrnAtual != -1) {
        
        // Traz a página específica de 53 bytes do disco para processamento em memória RAM.
        binaryNode no;
        readBinaryNode(&no, file, rrnAtual);

        // BUSCA LINEAR DENTRO DO NÓ 
        // Percorre os vetores internos da página carregada. Como as chaves estão estritamente 
        // ordenadas, o laço avança enquanto o elemento do nó for menor que a chave procurada.
        int i = 0;
        while (i < no.nroChaves && chave > no.chaves[i])
            i++;

        // VERIFICAÇÃO DE CORRESPONDÊNCIA
        if (i < no.nroChaves && chave == no.chaves[i])
            // Retorna o ponteiro PR (byte offset) para o arquivo de dados correspondente.
            return no.ponteiros[i]; 

        // PROCESSO DE DESCIDA
        rrnAtual = no.filhos[i];
    }

    // CHAVE INEXISTENTE
    return -1; 
}


void empilharNoRemovido(FILE *file, int rrn, binaryHeader *header) {
    // Executa o "Push" na pilha de páginas removidas. Salva o RRN antigo no topo
    // e atualiza o encadeamento através do campo 'proximo' do nó.
    binaryNode no;
    readBinaryNode(&no, file, rrn);
    
    no.removido = '1';
    no.proximo  = header->topo; // Nó destruído passa a apontar para o antigo topo
    header->topo = rrn;         // O topo agora passa a ser a página recém-liberada
    header->nroNos--;           // Decrementa o contador de nós ativos na árvore-B
    
    writeBinaryNode(&no, file, rrn);
}


int encontrarSucessor(FILE *file, int rrnFilhoDireita, int *chaveSucc, int *ptrSucc) {
    // BUSCA PELO SUCESSOR IMEDIATO
    int rrnAtual = rrnFilhoDireita;
    binaryNode no;

    while (1) {
        readBinaryNode(&no, file, rrnAtual);
        if (no.filhos[0] == -1)
            break; // Nó folha alcançado
        rrnAtual = no.filhos[0];
    }

    // Retorna por referência a menor chave da folha e seu byte offset associado
    *chaveSucc = no.chaves[0];
    *ptrSucc   = no.ponteiros[0];
    return rrnAtual;
}


void redistribuir(FILE *file, int rrnPai, int indiceFilho, int lado) {
    // REDISTRIBUIÇÃO UNIFORME DE CHAVES
    binaryNode pai, noEsq, noDir;
    readBinaryNode(&pai, file, rrnPai);

    int rrnEsq, rrnDir, indiceSeparador;

    if (lado == 0) {
        // Caso de balanceamento com irmão adjacente à direita
        rrnEsq         = pai.filhos[indiceFilho];
        rrnDir         = pai.filhos[indiceFilho + 1];
        indiceSeparador = indiceFilho;
    } else {
        // Caso de balanceamento com irmão adjacente à esquerda
        rrnEsq         = pai.filhos[indiceFilho - 1];
        rrnDir         = pai.filhos[indiceFilho];
        indiceSeparador = indiceFilho - 1;
    }

    readBinaryNode(&noEsq, file, rrnEsq);
    readBinaryNode(&noDir, file, rrnDir);


    // Coleta todos os elementos e a chave separadora do pai para redistribuí-los entre os nós.
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

    // CÁLCULO DE DIVISÃO UNIFORME
    int metadeEsq = totalChaves / 2;
    int meio      = metadeEsq; 

    // Reconstrução do Nó Esquerdo
    noEsq.nroChaves = metadeEsq;
    for (int i = 0; i < 3; i++) {
        noEsq.chaves[i]    = (i < metadeEsq) ? chaves[i] : -1;
        noEsq.ponteiros[i] = (i < metadeEsq) ? ptrs[i]   : -1;
        noEsq.filhos[i]    = (i <= metadeEsq) ? filhos[i] : -1;
    }
    noEsq.filhos[3] = -1;

    // Promoção da nova chave separadora para o nó pai
    pai.chaves[indiceSeparador]    = chaves[meio];
    pai.ponteiros[indiceSeparador] = ptrs[meio];

    // Reconstrução do Nó Direito
    int qtdDir = totalChaves - metadeEsq - 1;
    noDir.nroChaves = qtdDir;
    for (int i = 0; i < 3; i++) {
        noDir.chaves[i]    = (i < qtdDir) ? chaves[meio + 1 + i] : -1;
        noDir.ponteiros[i] = (i < qtdDir) ? ptrs[meio + 1 + i]   : -1;
        noDir.filhos[i]    = (i <= qtdDir) ? filhos[metadeEsq + 1 + i] : -1;
    }
    noDir.filhos[3] = -1;

    // Escrita das 3 páginas modificadas de volta para o disco
    writeBinaryNode(&pai,   file, rrnPai);
    writeBinaryNode(&noEsq, file, rrnEsq);
    writeBinaryNode(&noDir, file, rrnDir);
}


int concatenar(FILE *file, int rrnPai, int indiceFilho, binaryHeader *header) {
    // CONCATENAÇÃO DE PÁGINAS (FUSÃO)
    binaryNode pai, noEsq, noDir;
    readBinaryNode(&pai, file, rrnPai);

    // Prioriza a fusão armazenando tudo no nó esquerdo.
    int rrnEsq         = pai.filhos[indiceFilho - 1];
    int rrnDir         = pai.filhos[indiceFilho];
    int indiceSeparador = indiceFilho - 1;

    readBinaryNode(&noEsq, file, rrnEsq);
    readBinaryNode(&noDir, file, rrnDir);

    // A chave separadora do pai desce, ligando-se no nó esquerdo
    noEsq.chaves[noEsq.nroChaves]    = pai.chaves[indiceSeparador];
    noEsq.ponteiros[noEsq.nroChaves] = pai.ponteiros[indiceSeparador];
    noEsq.filhos[noEsq.nroChaves + 1] = noDir.filhos[0];
    noEsq.nroChaves++;

    // Guarda todas as chaves remanescentes do nó direito no nó esquerdo
    for (int i = 0; i < noDir.nroChaves; i++) {
        noEsq.chaves[noEsq.nroChaves]    = noDir.chaves[i];
        noEsq.ponteiros[noEsq.nroChaves] = noDir.ponteiros[i];
        noEsq.filhos[noEsq.nroChaves + 1] = noDir.filhos[i + 1];
        noEsq.nroChaves++;
    }

    // Balanceia as chaves do nó pai, deslocando os elementos vazios para o fim
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

    // REMOÇÃO FÍSICA E REAPROVEITAMENTO
    empilharNoRemovido(file, rrnDir, header);

    // Avalia e retorna se a perda da chave separadora gerou Underflow no próprio pai
    return (pai.nroChaves < (4 / 2) - 1);
}


void removeKey(FILE *file, int chave, binaryHeader *header) {
    if (header->noRaiz == -1)
        return; // Árvore vazia ignora comandos de deleção

    // Dispara a cadeia recursiva de busca e correção de balanço a partir do topo
    removerRecursivo(file, header->noRaiz, -1, -1, chave, header);

    // Se a antiga raiz esvaziou devido a uma concatenação de nós intermediários, 
    // a árvore diminui de altura e o primeiro filho herda a posição de raiz.
    binaryNode raiz;
    readBinaryNode(&raiz, file, header->noRaiz);
    if (raiz.nroChaves == 0 && raiz.filhos[0] != -1) {
        int rrnRaizVelha = header->noRaiz;
        header->noRaiz   = raiz.filhos[0]; // Nova raiz herda o nível abaixo
        
        // Coloca a raiz esvaziada na pilha de removidos
        empilharNoRemovido(file, rrnRaizVelha, header);

        // Atualiza o campo de identificação da nova página topo
        binaryNode novaRaiz;
        readBinaryNode(&novaRaiz, file, header->noRaiz);
        novaRaiz.tipoNo = (novaRaiz.filhos[0] == -1) ? -1 : 0; // Vira folha ou raiz
        writeBinaryNode(&novaRaiz, file, header->noRaiz);
    }
    
    // Salva os campos com valores atualizados
    writeBinaryHeader(header, file);
}