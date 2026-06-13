#include "utils.h"
#include "register.h"
#include "header.h"

void preencherCriteriosBusca(CriteriosBusca *criterios, char *campo, char *conteudo){
    // INICIALIZAÇÃO DE CRITÉRIOS (Mapeamento String -> Flag/Valor)
    // Uma escada de if/else if verifica estritamente os nomes de colunas definidos
    // pela especificação do projeto, ignorando entradas inválidas.

    // CAMPOS DE TAMANHO FIXO (INTEIROS)
    // A lógica condicional em linha intercepta o termo "NULO" exigido
    // pelo RunCodes para representar buscas por campos sem valor cadastrado, convertendo-o
    // fisicamente para o inteiro sentinela -1.

    if (strcmp(campo, "codEstacao") == 0){
        criterios->flag_codEstacao = 1;
        criterios->regBusca.codEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    else if (strcmp(campo, "codLinha") == 0){
        criterios->flag_codLinha = 1;
        criterios->regBusca.codLinha = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    else if (strcmp(campo, "codProxEstacao") == 0){
        criterios->flag_codProxEstacao = 1;
        criterios->regBusca.codProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    else if (strcmp(campo, "distProxEstacao") == 0){
        criterios->flag_distProxEstacao = 1;
        criterios->regBusca.distProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    else if (strcmp(campo, "codLinhaIntegra") == 0){
        criterios->flag_codLinhaIntegra = 1;
        criterios->regBusca.codLinhaIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }
    else if (strcmp(campo, "codEstIntegra") == 0){
        criterios->flag_codEstIntegra = 1;
        criterios->regBusca.codEstIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
    }

    // CAMPOS DE TAMANHO VARIÁVEL (STRINGS)
    // O valor em texto puro (já sem aspas duplas da leitura prévia) é copiado 
    // diretamente para o buffer interno da struct de busca. Em pesquisas por strings 
    // nulas, o programa lê explicitamente "NULO" para o campo.

    else if (strcmp(campo, "nomeEstacao") == 0){
        criterios->flag_nomeEstacao = 1;
        strcpy(criterios->regBusca.nomeEstacao, conteudo);
    }
    else if (strcmp(campo, "nomeLinha") == 0){
        criterios->flag_nomeLinha = 1;
        strcpy(criterios->regBusca.nomeLinha, conteudo);
    }
}

int checagemCriteriosBusca(CriteriosBusca *criterios, Registro *regAtual){
    // FILTRO DE VALIDADE
    // Registros marcados como logicamente removidos ('1') são 
    // imediatamente descartados de qualquer resultado de busca.
    if (regAtual->removido == '1')
        return 0; 

    // AVALIAÇÃO DOS CRITÉRIOS
    // A validação ocorre em cascata. Se a flag de um campo está ativa (1), 
    // o valor armazenado no registro físico é comparado com o valor alvo da busca.
    // Qualquer divergência retorna um código de falha (0).

    // Validação de Chave Primária (Única)
    if (criterios->flag_codEstacao == 1 && regAtual->codEstacao != criterios->regBusca.codEstacao)
        return 0;

    // Validação de String Variável 
    // O uso de strcmp garante a equivalência exata do nome da estação,
    // incluindo correspondências nulas quando aplicável.
    if (criterios->flag_nomeEstacao == 1 && strcmp(regAtual->nomeEstacao, criterios->regBusca.nomeEstacao) != 0)
        return 0;

    // Validação de Chave Secundária (Código da Linha)
    if (criterios->flag_codLinha == 1 && regAtual->codLinha != criterios->regBusca.codLinha)
        return 0;

    // Validação de String Variável (Nome da Linha)
    if (criterios->flag_nomeLinha == 1 && strcmp(regAtual->nomeLinha, criterios->regBusca.nomeLinha) != 0)
        return 0;

    // Validação Numérica: Código da Próxima Estação
    if (criterios->flag_codProxEstacao == 1 && regAtual->codProxEstacao != criterios->regBusca.codProxEstacao)
        return 0;

    // Validação Numérica: Distância (em metros/km)
    if (criterios->flag_distProxEstacao == 1 && regAtual->distProxEstacao != criterios->regBusca.distProxEstacao)
        return 0;

    // Validação Numérica: Linha de Integração
    if (criterios->flag_codLinhaIntegra == 1 && regAtual->codLinhaIntegra != criterios->regBusca.codLinhaIntegra)
        return 0;

    // Validação Numérica: Estação de Integração
    if (criterios->flag_codEstIntegra == 1 && regAtual->codEstIntegra != criterios->regBusca.codEstIntegra)
        return 0;

    // SUCESSO ABSOLUTO
    // O registro sobreviveu à cascata de ifs, o que prova matematicamente que ele
    // satisfaz todas as flags que estavam ativadas na struct de critérios.
    return 1; 
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
