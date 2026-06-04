#include "utils.h"
#include "register.h"
#include "header.h"

void preencherCriteriosBusca(CriteriosBusca *criterios, char *campo, char *conteudo){
    // Verifica se o campo é o de código da estação. Se sim, ativa a flag e atribui o valor a ser buscado
    if (strcmp(campo, "codEstacao") == 0){
        criterios->flag_codEstacao = 1;
        criterios->regBusca.codEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    // Verifica se o campo é o de nome da estação. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "nomeEstacao") == 0){
        criterios->flag_nomeEstacao = 1;
        strcpy(criterios->regBusca.nomeEstacao, conteudo);
    }
    // Verifica se o campo é o de código da linha. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "codLinha") == 0){
        criterios->flag_codLinha = 1;
        criterios->regBusca.codLinha = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    // Verifica se o campo é o de nome da linha. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "nomeLinha") == 0){
        criterios->flag_nomeLinha = 1;
        strcpy(criterios->regBusca.nomeLinha, conteudo);
    }
    // Verifica se o campo é o de código da próxima estação. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "codProxEstacao") == 0){
        criterios->flag_codProxEstacao = 1;
        criterios->regBusca.codProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    // Verifica se o campo é o de distância para a próxima estação. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "distProxEstacao") == 0){
        criterios->flag_distProxEstacao = 1;
        criterios->regBusca.distProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    // Verifica se o campo é o de código da linha de integração. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "codLinhaIntegra") == 0){
        criterios->flag_codLinhaIntegra = 1;
        criterios->regBusca.codLinhaIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    // Verifica se o campo é o de código da estação de integração. Se sim, ativa a flag e atribui o valor a ser buscado
    else if (strcmp(campo, "codEstIntegra") == 0){
        criterios->flag_codEstIntegra = 1;
        criterios->regBusca.codEstIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
}

int checagemCriteriosBusca(CriteriosBusca *criterios, Registro *regAtual){
    if (regAtual->removido == '1')
        return 0; // Registro foi removido logicamente, não atende aos critérios de busca

    // Verifica se o campo de código da estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codEstacao == 1 && regAtual->codEstacao != criterios->regBusca.codEstacao)
        return 0;

    // Verifica se o campo de nome da estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_nomeEstacao == 1 && strcmp(regAtual->nomeEstacao, criterios->regBusca.nomeEstacao) != 0)
        return 0;

    // Verifica se o campo de código da linha é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codLinha == 1 && regAtual->codLinha != criterios->regBusca.codLinha)
        return 0;

    // Verifica se o campo de nome da linha é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_nomeLinha == 1 && strcmp(regAtual->nomeLinha, criterios->regBusca.nomeLinha) != 0)
        return 0;

    // Verifica se o campo de código da próxima estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codProxEstacao == 1 && regAtual->codProxEstacao != criterios->regBusca.codProxEstacao)
        return 0;

    // Verifica se o campo de distância para a próxima estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_distProxEstacao == 1 && regAtual->distProxEstacao != criterios->regBusca.distProxEstacao)
        return 0;

    // Verifica se o campo de código da linha de integração é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codLinhaIntegra == 1 && regAtual->codLinhaIntegra != criterios->regBusca.codLinhaIntegra)
        return 0;

    // Verifica se o campo de código da estação de integração é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. 
    // Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codEstIntegra == 1 && regAtual->codEstIntegra != criterios->regBusca.codEstIntegra)
        return 0;

    return 1; // O registro atende a todos os critérios de busca
}

void lerCriteriosUsuario(CriteriosBusca *criterios, int quantidade) {
    char nomeCampo[50];
    char valorCampo[100];
    
    for (int j = 0; j < quantidade; j++) {
        scanf("%s", nomeCampo);
        if (strcmp(nomeCampo, "nomeEstacao") == 0 || strcmp(nomeCampo, "nomeLinha") == 0) {
            ScanQuoteString(valorCampo);
        } else {
            scanf("%s", valorCampo);
        }
        preencherCriteriosBusca(criterios, nomeCampo, valorCampo);
    }
}

int isEstacaoUnica(Estacoes *estacoes, char *nomeEstAtual){
    if (nomeEstAtual[0] == '\0') 
        return 0; // Se o nome da estação for vazio, não contabiliza

    for (int i = 0; i < estacoes->numEstacoes; i++) {
        if (strcmp(estacoes->nomesEstacoes[i], nomeEstAtual) == 0) {
            return 0;
        }
    }
    // Adiciona a nova estação diretamente na struct
    strcpy(estacoes->nomesEstacoes[estacoes->numEstacoes], nomeEstAtual);
    estacoes->numEstacoes++;
    return 1;
}

int isParUnico(Estacoes *estacoes, int EstA, int estB){
    // Se o campo de próxima estação for vazio, não contabiliza
    if (estB == -1) 
        return 0;

    // Percorre a lista de pares para buscar correspondências
    for (int i = 0; i < estacoes->numParesEstacao; i++) {
        // Se encontrar EstA na primeira coluna e estB na segunda, o par já existe
        if (estacoes->paresEstacoes[i][0] == EstA && estacoes->paresEstacoes[i][1] == estB)
            return 0; 
    }

    // Se é um par novo, adiciona nas colunas correspondentes e incrementa o total
    estacoes->paresEstacoes[estacoes->numParesEstacao][0] = EstA;
    estacoes->paresEstacoes[estacoes->numParesEstacao][1] = estB;
    estacoes->numParesEstacao++;
    
    return 1;
}

void recalcularEstacoesPares(FILE *arquivoBinario, Header *cabecalho){
    // Estrutura para armazenar as estações e os pares de estações únicas encontrados durante a leitura dos registros
    Estacoes estacoes = {0};

    // Posiciona no início dos registros
    fseek(arquivoBinario, 17, SEEK_SET);

    Registro regAtual;

    // Loop que lê cada registro do arquivo binário e atualiza as contagens de estações e pares de estações únicas
    while (readRegistros(&regAtual, arquivoBinario)){
        // Processa apenas registros não removidos
        if (regAtual.removido == '0'){
            // Verifica se a estação é única
            isEstacaoUnica(&estacoes, regAtual.nomeEstacao);
            
            // Verifica se o par de estações é único
            isParUnico(&estacoes, regAtual.codEstacao, regAtual.codProxEstacao);
        }
    }

    // Atualiza o cabeçalho com os novos valores
    cabecalho->nroEstacoes = estacoes.numEstacoes;
    cabecalho->nroParesEstacao = estacoes.numParesEstacao;
}

void BinarioNaTela(char *arquivo){
    FILE *fs;
    if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))){
        fprintf(stderr,
                "ERRO AO ESCREVER O BINARIO NA TELA (função binarioNaTela): "
                "não foi possível abrir o arquivo que me passou para leitura. "
                "Ele existe e você tá passando o nome certo? Você lembrou de "
                "fechar ele com fclose depois de usar?\n");
        return;
    }

    fseek(fs, 0, SEEK_END);
    size_t fl = ftell(fs);

    fseek(fs, 0, SEEK_SET);
    unsigned char *mb = (unsigned char *)malloc(fl);
    fread(mb, 1, fl, fs);

    unsigned long cs = 0;
    for (unsigned long i = 0; i < fl; i++){
        cs += (unsigned long)mb[i];
    }

    printf("%lf\n", (cs / (double)100));

    free(mb);
    fclose(fs);
}

void ScanQuoteString(char *str){
    char R;

    while ((R = getchar()) != EOF && isspace(R)); // ignorar espaços, \r, \n...

    if (R == 'N' || R == 'n'){ // campo NULO
        getchar();
        getchar();
        getchar();       // ignorar o "ULO" de NULO.
        strcpy(str, ""); // copia string vazia
    }
    else if (R == '\"'){
        if (scanf("%[^\"]", str) != 1)
        { // ler até o fechamento das aspas
            strcpy(str, "");
        }
        getchar(); // ignorar aspas fechando
    }
    else if (R != EOF){   // vc tá tentando ler uma string que não tá entre
        // aspas! Fazer leitura normal %s então, pois deve
        // ser algum inteiro ou algo assim...
        str[0] = R;
        scanf("%s", &str[1]);
    }
    else{ // EOF
        strcpy(str, "");
    }
}
