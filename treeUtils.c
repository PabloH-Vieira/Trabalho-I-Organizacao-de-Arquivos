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
    
    //GERENCIAMENTO DINÂMICO DE ESPAÇO (REAPROVEITAMENTO DA PILHA DE REMOVIDOS)
    // Verifica se existem páginas da Árvore-B que foram liberadas anteriormente 
    // por rotinas de concatenação e estão disponíveis para reuso.
    if (header->topo != -1) {
        
        // REAPROVEITAMENTO DE PÁGINA
        // Seleciona o RRN que está no topo da lista encadeada de removidos.
        rrn = header->topo;
        
        // Carrega temporariamente o nó removido da pilha na RAM para extrair 
        // qual é a próxima página disponível na ordem de encadeamento.
        binaryNode topo;
        readBinaryNode(&topo, file, rrn);
        
        // O cabeçalho da Árvore-B herda o ponteiro, removendo o nó atual da pilha
        header->topo = topo.proximo;
    } else {
        
        // AUMENTO DO TAMANHO FÍSICO DO ARQUIVO
        // A pilha está vazia. Aloca o RRN sequencial disponível ao final do arquivo 
        // e incrementa o campo de proxRRN para a próxima chamada.
        rrn = header->proxRRN;
        header->proxRRN++;
    }
    
    // ATUALIZAÇÃO DE CAMPO DE CONTAGEM
    // Como um novo nó (reaproveitado ou estendido) foi efetivamente ativado 
    // na topologia da árvore, incrementa o contador total de nós ativos.
    header->nroNos++;
    
    return rrn; // Retorna o offset lógico pronto para a escrita da nova página
}


void inserirNoNo(binaryNode *node, int chave, int ptr, int filhoDireita) {
    // VARIÁVEL DE CURSOR (PROCESSAMENTO DA DIREITA PARA A ESQUERDA)
    // Inicia o processamento a partir da última chave válida atualmente presente no nó.
    int i = node->nroChaves - 1;

    // DESLOCAMENTO EM CASCATA
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

    // CONFIGURAÇÃO POSICIONAL DOS ELEMENTOS
    // Após abrir o "vazio" ordenado na posição correta (i + 1), os novos dados 
    // são injetados diretamente na estrutura de forma atômica.
    node->chaves[i + 1] = chave;
    node->ponteiros[i + 1] = ptr; // Registra o offset físico no arquivo de dados
    node->filhos[i + 2] = filhoDireita; // Vincula a subárvore da direita originada
    
    // ATUALIZAÇÃO DE CONTAGEM LOCAL
    // Incrementa o contador de chaves ativas na página para refletir o novo estado do nó.
    node->nroChaves++;
}


int inserirRecursivo(FILE *file, int rrnAtual, int chave, int ptr, 
                    int *promotionKey, int *promotionPtr, int *promotionRightChild, binaryHeader *header) {
    
    // CASO BASE DA RECURSÃO (FLUXO DE SUBIDA INICIAL)
    // Se ultrapassou um nó folha (ponteiro para subárvore inexistente == -1), 
    // significa que a folha ideal foi localizada no andar anterior. 
    // Configura os parâmetros de promoção para forçar a inserção física na folha.
    if (rrnAtual == -1) {
        *promotionKey = chave;
        *promotionPtr = ptr;
        *promotionRightChild = -1; // Folhas não possuem descendentes
        return PROMOTION; 
    }
    
    // OPERAÇÃO EM RAM (LEITURA E PESQUISA INTERNA)
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
    // A especificação proíbe a duplicidade do campo 'codEstacao'. 
    // Se a chave já existe no nó atual, o processamento é imediatamente abortado.
    if (i < node.nroChaves && chave == node.chaves[i]) {
        return ERROR; 
    }

    // Variáveis locais para isolar o retorno e o overflow das subárvores inferiores
    int promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow;
    
    // DESCIDA RECURSIVA
    // Avança na árvore em profundidade seguindo pelo ponteiro do filho correspondente
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
        // O nó atual possui menos de 3 chaves.
        // Delega para o inserirNoNo fazer o rearranjo e a ordenação interna dos vetores.
        inserirNoNo(&node, promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow);
        
        // Persiste a mutação da página em disco de volta no seu RRN original
        writeBinaryNode(&node, file, rrnAtual);
        return NO_PROMOTION; // Interrompe a cadeia de Splits: avisa o pai que a árvore estabilizou
    } 
    else {
        // CASO DE INDISPONIBILIDADE DE ESPAÇO: SPLIT EM CASCATA
        // O nó já atingiu o limite máximo de 3 chaves. Não há espaço físico.
        // Dispara o splitNode, dividindo os elementos uniformemente, criando uma nova 
        // página à direita e configurando as variáveis de promoção para o nó pai.
        splitNode(file, &node, rrnAtual, promotionKeyBelow, promotionPtrBelow, promotionRightChildBelow,
                  promotionKey, promotionPtr, promotionRightChild, header);
                  
        return PROMOTION; // Propaga o resultado: avisa a chamada recursiva anterior que este nó também dividiu
    }
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
    // Executa a busca em profundidade no disco a partir da raiz atual.
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
        
        // Nova página é uma raiz interna com descendentes (tipoNo = 0).
        novaRaiz.tipoNo      = 0; 
        novaRaiz.chaves[0]   = chavePromovida; // Recebe o elemento promovido do split
        novaRaiz.ponteiros[0] = ptrPromovido;   // Vincula o byte offset associado à chave promovida
        
        // ACOPLAMENTO DE SUBÁRVORES
        // O filho da esquerda aponta para a raiz antiga. O filho da direita aponta 
        // para a nova página gerada à direita pelo split.
        novaRaiz.filhos[0]   = header->noRaiz;
        novaRaiz.filhos[1]   = rrnDireita;
        novaRaiz.nroChaves   = 1;

        // Reserva o RRN para persistência da nova página pai
        int rrnNovaRaiz = alocarRRN(file, header);

        // Escreve a nova raiz estruturada em disco
        writeBinaryNode(&novaRaiz, file, rrnNovaRaiz);
        
        // CONSOLIDAÇÃO DO TOPO
        // Redireciona o campo noRaiz do cabeçalho para apontar para o novo nó topo.
        header -> noRaiz = rrnNovaRaiz;
        writeBinaryHeader(header, file);
    }
}


int searchKey(FILE *file, int chave, binaryHeader *header) {
    // INICIALIZAÇÃO DO CURSOR DE DISCO
    // A busca inicia-se obrigatoriamente no nó raiz apontado pelo campo do cabeçalho.
    int rrnAtual = header->noRaiz;

    // NAVEGAÇÃO VERTICAL POR PÁGINAS
    // O laço prossegue descendo pelos níveis da árvore até encontrar o registro ou 
    // atingir uma referência nula (-1), indicando que ultrapassou um nó folha.
    while (rrnAtual != -1) {
        
        // OPERAÇÃO DE I/O (LEITURA)
        // Traz a página específica de 53 bytes do disco para processamento em memória RAM.
        binaryNode no;
        readBinaryNode(&no, file, rrnAtual);

        // BUSCA LINEAR IDENTRO DO NÓ 
        // Percorre os vetores internos da página carregada. Como as chaves estão estritamente 
        // ordenadas (C1 < C2 < ... < Cq-1), o laço avança enquanto o elemento do nó 
        // for menor que a chave procurada.
        int i = 0;
        while (i < no.nroChaves && chave > no.chaves[i])
            i++;

        // VERIFICAÇÃO DE CORRESPONDÊNCIA
        // Se o cursor parou em um índice válido e o conteúdo da chave for idêntico ao alvo,
        // o registro foi localizado com sucesso.
        if (i < no.nroChaves && chave == no.chaves[i])
            // Retorna o ponteiro PR (byte offset) para o arquivo de dados correspondente.
            return no.ponteiros[i]; 

        // PROCESSO DE DESCIDA (SELEÇÃO DE SUBÁRVORE)
        // Se não houve match, o índice 'i' reflete perfeitamente a posição do ponteiro do filho (Pj)
        // que delimita o intervalo numérico da chave, guiando a descida para o próximo nível.
        rrnAtual = no.filhos[i];
    }

    // FRACASSO NA BUSCA (CHAVE INEXISTENTE)
    // O cursor atingiu o valor -1, o que prova que a chave não existe na Árvore-B.
    return -1; 
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

void splitNode(FILE *file, binaryNode *p_oldpage, int rrn_oldpage, 
               int key, int ponteiro, int r_child, 
               int *promo_key, int *promo_ponteiro, int *promo_r_child, 
               binaryHeader *header) {
    
    // CRIAÇÃO DOS ARRAYS temporários
    // Estruturas alocadas temporariamente na Stack de RAM com tamanho (MAXKEYS + 1).
    // Permitem injetar a nova chave e realizar a ordenação interna antes da divisão física do nó.
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
    // Percorre o vetor de trabalho de trás para frente, deslocando elementos maiores 
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
    // Invoca o gerenciador dinâmico. Se a pilha de removidos do cabeçalho tiver nós livres (topo != -1), 
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

    // DISTRIBUIÇÃO UNIFORME DE ELEMENTOS (REGRA M=4)
    
    // CONFIGURAÇÃO DO NÓ ESQUERDO (PÁGINA ORIGINAL)
    // Retém os 2 primeiros elementos de menor valor da partição.
    p_oldpage->chaves[0] = tempchaves[0];
    p_oldpage->ponteiros[0] = tempponteiros[0];
    p_oldpage->filhos[0] = tempfilhos[0];

    p_oldpage->chaves[1] = tempchaves[1];
    p_oldpage->ponteiros[1] = tempponteiros[1];
    p_oldpage->filhos[1] = tempfilhos[1];
    p_oldpage->filhos[2] = tempfilhos[2];

    // TRATAMENTO DE LIMPEZA FÍSICA
    // Zera os índices residuais da antiga terceira chave colocando o valor sentinela -1.
    p_oldpage->chaves[2] = -1;
    p_oldpage->ponteiros[2] = -1;
    p_oldpage->filhos[3] = -1;
    p_oldpage->nroChaves = 2; // Atualiza a taxa de ocupação local para 2 chaves ativas

    // CONFIGURAÇÃO DOS PARÂMETROS DE PROMOÇÃO
    // Isola a 3ª chave (elemento médio da partição) e envia para as variáveis de referência.
    // Elas serão coletadas pela função recursiva mãe para inserção no nó pai.
    *promo_key = tempchaves[2];
    *promo_ponteiro = tempponteiros[2];

    // CONFIGURAÇÃO DO NÓ DIREITO (NOVA PÁGINA CRIADA)
    // Herda a 4ª chave (maior valor). Conforme especificado pelo projeto, 
    // o nó da direita recebe os elementos restantes, preservando as propriedades dos filhos (subárvores).
    p_newpage.chaves[0] = tempchaves[3];
    p_newpage.ponteiros[0] = tempponteiros[3];
    p_newpage.filhos[0] = tempfilhos[3];
    p_newpage.filhos[1] = tempfilhos[4];
    p_newpage.nroChaves = 1; // Nova página nasce com exatamente 1 chave ativa

    // PERSISTÊNCIA
    // Grava as duas páginas de exatos 53 bytes cada diretamente nos seus respectivos RRNs no arquivo.
    // Isso garante o balanceamento exato exigido na especificação da Árvore-B.
    writeBinaryNode(p_oldpage, file, rrn_oldpage);
    writeBinaryNode(&p_newpage, file, *promo_r_child);
}