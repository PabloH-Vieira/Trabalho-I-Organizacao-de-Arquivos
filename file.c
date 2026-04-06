#include "file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void newHeader(Header *header){
    header->status = '0';
    header->topo = -1;
    header->proxRRN = 0;
    header->nroEstacoes = 0;
    header->nroParesEstacao = 0;
}

void writeHeader(Header *header, FILE *file){
    fseek(file, 0, SEEK_SET);
    // Escreve o cabeçalho no arquivo campo a campo para evitar o padding
    fwrite(&header->status, sizeof(char), 1, file);
    fwrite(&header->topo, sizeof(int), 1, file);
    fwrite(&header->proxRRN, sizeof(int), 1, file);
    fwrite(&header->nroEstacoes, sizeof(int), 1, file);
    fwrite(&header->nroParesEstacao, sizeof(int), 1, file);
}

void readHeader(Header *header, FILE *file){
    fseek(file, 0, SEEK_SET);
    fread(&header->status, sizeof(char), 1, file);
    fread(&header->topo, sizeof(int), 1, file);
    fread(&header->proxRRN, sizeof(int), 1, file);
    fread(&header->nroEstacoes, sizeof(int), 1, file);
    fread(&header->nroParesEstacao, sizeof(int), 1, file);
}

void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual){
    switch (fieldIndex){
        case 0:
            // Verifica se o campo tem valor nulo
            regAtual->codEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
        case 1:
            // Passa o tamanho do nome da estação
            regAtual->tamNomeEstacao = strlen(buffer);
            if (regAtual->tamNomeEstacao > 28) 
                regAtual->tamNomeEstacao = 28;
            memcpy(regAtual->nomeEstacao, buffer, regAtual->tamNomeEstacao);
            regAtual->nomeEstacao[regAtual->tamNomeEstacao] = '\0';
            break;
        case 2:
            regAtual->codLinha = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
        case 3:
            // Passa o tamanho do nome da linha
            regAtual->tamNomeLinha = strlen(buffer);
            if (regAtual->tamNomeLinha > 14) 
                regAtual->tamNomeLinha = 14;
            memcpy(regAtual->nomeLinha, buffer, regAtual->tamNomeLinha);
            regAtual->nomeLinha[regAtual->tamNomeLinha] = '\0';
            break;
        case 4:
            regAtual->codProxEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
        case 5:
            regAtual->distProxEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
        case 6:
            regAtual->codLinhaIntegra = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
        case 7:
            regAtual->codEstIntegra = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
    }
}

void writeRegistros(Registro *registro, FILE *saida, Header *cabecalho){
    // Preenche campos não presentes no CSV
    registro->removido = '0';
    registro->proximo = -1;

    // Vetor com tamanho fixo de 80 bytes do registro
    char bufferRegistro[80];
    memset(bufferRegistro, '$', 80); // Inicializa o buffer do registro com zeros para evitar lixo

    // Variável para controlar o alocação em cada byte do vetor
    int offset = 0;

    // Preenche o campo de removido no buffer do registro
    memcpy(bufferRegistro + offset, &registro->removido, sizeof(char));
    offset += sizeof(char);

    // Preenche o campo de próximo no buffer do registro
    memcpy(bufferRegistro + offset, &registro->proximo, sizeof(int));
    offset += sizeof(int);

    // Preenche o campo de código da estação no buffer do registro
    memcpy(bufferRegistro + offset, &registro->codEstacao, sizeof(int));
    offset += sizeof(int);

    // Preenche o campo de código da linha no buffer do registro
    memcpy(bufferRegistro + offset, &registro->codLinha, sizeof(int));
    offset += sizeof(int);

    // Preenche o campo de código da próxima estação no buffer do registro
    memcpy(bufferRegistro + offset, &registro->codProxEstacao, sizeof(int));
    offset += sizeof(int);

    // Preenche o campo de distância para a próxima estação no buffer do registro
    memcpy(bufferRegistro + offset, &registro->distProxEstacao, sizeof(int));
    offset += sizeof(int);

    // Prenche o campo do código da linha de integração no buffer do registro
    memcpy(bufferRegistro + offset, &registro->codLinhaIntegra, sizeof(int));
    offset += sizeof(int);

    // Preenche o campo do código da estação de integração no buffer do registro
    memcpy(bufferRegistro + offset, &registro->codEstIntegra, sizeof(int));
    offset += sizeof(int);

    // Preenche o campo do tamanho do nome da estação e a quantidade exata de bytes da string no buffer do registro
    offset = 29;
    memcpy(bufferRegistro + offset, &registro->tamNomeEstacao, sizeof(int));
    offset += sizeof(int);
    if (registro->tamNomeEstacao > 0){
        memcpy(bufferRegistro + offset, registro->nomeEstacao, registro->tamNomeEstacao);
        offset += registro->tamNomeEstacao;
    }

    // Preenche o campo do tamanho do nome da linha e a quantidade exata de bytes da string no buffer do registro
    memcpy(bufferRegistro + offset, &registro->tamNomeLinha, sizeof(int));
    offset += sizeof(int);
    if (registro->tamNomeLinha > 0){
        memcpy(bufferRegistro + offset, registro->nomeLinha, registro->tamNomeLinha);
    }

    // Escreve o buffer do registro no arquivo de saída
    fwrite(bufferRegistro, sizeof(char), 80, saida);
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

int readRegistros(Registro *registro, FILE *file){
    char bufferRegistro[80];
    int offset = 0;

    // Lê 80 bytes do arquivo para preencher o buffer do registro
    if (fread(bufferRegistro, sizeof(char), 80, file) != 80)
        return 0; // Falha na leitura ou fim do arquivo

    // Lê o valor do campo de removido e guarda no buffer do registro
    memcpy(&registro->removido, bufferRegistro + offset, sizeof(char));
    offset += sizeof(char);

    // Lê o valor do campo de próximo e guarda no buffer do registro
    memcpy(&registro->proximo, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o valor do campo de código da estação e guarda no buffer do registro
    memcpy(&registro->codEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o valor do campo de código da linha e guarda no buffer do registro
    memcpy(&registro->codLinha, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o valor do campo de código da próxima estação e guarda no buffer do registro
    memcpy(&registro->codProxEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o valor do campo de distância para a próxima estação e guarda no buffer do registro
    memcpy(&registro->distProxEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o valor do campo de código da linha de integração e guarda no buffer do registro
    memcpy(&registro->codLinhaIntegra, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o valor do campo de código da estação de integração e guarda no buffer do registro
    memcpy(&registro->codEstIntegra, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // Lê o tamanho do campo de nome da estação e guarda a string na quantidade exata de bytes no buffer do registro
    offset = 29;
    memcpy(&registro->tamNomeEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);
    // Validar tamanho para evitar buffer overflow
    if (registro->tamNomeEstacao < 0 || registro->tamNomeEstacao > 28){
        registro->tamNomeEstacao = 0;
        registro->nomeEstacao[0] = '\0';
    }
    else if (registro->tamNomeEstacao > 0){
        memcpy(registro->nomeEstacao, bufferRegistro + offset, registro->tamNomeEstacao);
        registro->nomeEstacao[registro->tamNomeEstacao] = '\0';
        offset += registro->tamNomeEstacao;
    }

    // Lê o tamanho do campo de nome da linha e guarda a string na quantidade exata de bytes no buffer do registro
    memcpy(&registro->tamNomeLinha, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);
    // Validar tamanho para evitar buffer overflow
    if (registro->tamNomeLinha < 0 || registro->tamNomeLinha > 15){
        registro->tamNomeLinha = 0;
        registro->nomeLinha[0] = '\0';
    }
    else if (registro->tamNomeLinha > 0){
        memcpy(registro->nomeLinha, bufferRegistro + offset, registro->tamNomeLinha);
        registro->nomeLinha[registro->tamNomeLinha] = '\0';
    }

    return 1; // Retorna 1 se o registro foi lido com sucesso
}

void printRegistros(Registro *registro){
    // verifica se o registro foi removido logicamente. Se não, imprime os campos do registro
    if (registro->removido == '0'){
        // Imprime o campo de código da estação, com verificação de valor nulo
        if (registro->codEstacao == -1)
            printf("NULO ");
        else 
            printf("%d ", registro->codEstacao);

        // Imprime o campo de nome da estação ou "NULO"
            if (registro->tamNomeEstacao == 0)
                printf("NULO ");
            else
                printf("%s ", registro->nomeEstacao);

        // Imprime o campo de código da linha, com verificação de valor nulo
        if (registro->codLinha == -1)
            printf("NULO ");
        else
            printf("%d ", registro->codLinha);

         //Imprime o campo de nome da linha ou "NULO"
        if (registro->tamNomeLinha == 0)
            printf("NULO ");
        else
            printf("%s ", registro->nomeLinha);

        // Imprime o campo de código da próxima estação, com verificação de valor nulo
        if (registro->codProxEstacao == -1)
            printf("NULO ");
        else
            printf("%d ", registro->codProxEstacao);
        
        // Imprime o campo de distância para a próxima estação, com verificação de valor nulo
        if (registro->distProxEstacao == -1)
            printf("NULO ");
        else
            printf("%d ", registro->distProxEstacao);
        
        // Imprime o campo de código da linha de integração, com verificação de valor nulo
        if (registro->codLinhaIntegra == -1)
            printf("NULO ");
        else
            printf("%d ", registro->codLinhaIntegra);
        
        // Imprime o campo de código da estação de integração, com verificação de valor nulo
        if (registro->codEstIntegra == -1)
            printf("NULO\n");
        else
            printf("%d\n", registro->codEstIntegra);
    }
}

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

    // Verifica se o campo de código da estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codEstacao == 1 && regAtual->codEstacao != criterios->regBusca.codEstacao)
        return 0;

    // Verifica se o campo de nome da estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_nomeEstacao == 1 && strcmp(regAtual->nomeEstacao, criterios->regBusca.nomeEstacao) != 0)
        return 0;

    // Verifica se o campo de código da linha é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codLinha == 1 && regAtual->codLinha != criterios->regBusca.codLinha)
        return 0;

    // Verifica se o campo de nome da linha é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_nomeLinha == 1 && strcmp(regAtual->nomeLinha, criterios->regBusca.nomeLinha) != 0)
        return 0;

    // Verifica se o campo de código da próxima estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codProxEstacao == 1 && regAtual->codProxEstacao != criterios->regBusca.codProxEstacao)
        return 0;

    // Verifica se o campo de distância para a próxima estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_distProxEstacao == 1 && regAtual->distProxEstacao != criterios->regBusca.distProxEstacao)
        return 0;

    // Verifica se o campo de código da linha de integração é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codLinhaIntegra == 1 && regAtual->codLinhaIntegra != criterios->regBusca.codLinhaIntegra)
        return 0;

    // Verifica se o campo de código da estação de integração é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
    if (criterios->flag_codEstIntegra == 1 && regAtual->codEstIntegra != criterios->regBusca.codEstIntegra)
        return 0;

    return 1; // O registro atende a todos os critérios de busca
}

void preencherNovoRegistro(Registro *novoReg){
    char bufferAux[100];
    // Lê o código da estação e preenche o campo
    scanf("%d", &novoReg->codEstacao);

    // Lê o nome da estação e preenche os campos de tamanho e nome da estação
    ScanQuoteString(bufferAux);
    novoReg->tamNomeEstacao = strlen(bufferAux);
    memcpy(novoReg->nomeEstacao, bufferAux, novoReg->tamNomeEstacao);

    // Lê o código da linha e preenche o campo
    scanf("%s", bufferAux);
    // Verificar se o campo é nulo
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codLinha = -1;
    else
        novoReg->codLinha = atoi(bufferAux);

    // Lê o nome da linha e preenche os campos de tamanho e nome da linha
    ScanQuoteString(bufferAux);
    novoReg->tamNomeLinha = strlen(bufferAux);
    memcpy(novoReg->nomeLinha, bufferAux, novoReg->tamNomeLinha);

    // Lê o código da próxima estação e preenche o campo
    scanf("%s", bufferAux);
    // Verificar se o campo é nulo
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codProxEstacao = -1;
    else
        novoReg->codProxEstacao = atoi(bufferAux);

    // Lê a distância para a próxima estação e preenche o campo
    scanf("%s", bufferAux);
    // Verificar se o campo é nulo
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->distProxEstacao = -1;
    else
        novoReg->distProxEstacao = atoi(bufferAux);

    // Lê o código da linha de integração e preenche o campo
    scanf("%s", bufferAux);
    // Verificar se o campo é nulo
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codLinhaIntegra = -1;
    else
        novoReg->codLinhaIntegra = atoi(bufferAux);

    // Lê o código da estação de integração e preenche o campo
    scanf("%s", bufferAux);
    // Verificar se o campo é nulo
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codEstIntegra = -1;
    else
        novoReg->codEstIntegra = atoi(bufferAux);

    // Setando campos de controle para o novo registro
    novoReg->removido = '0';
    novoReg->proximo = -1;
}

void updateRegistro(Registro *registro, char camposUpdate[][50], char valoresUpdate[][100], int p){
    // Aplica atualizacoes no registro em memoria
    for (int k = 0; k < p; k++){
        if (strcmp(camposUpdate[k], "codEstacao") == 0)
            registro->codEstacao = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
        
        else if (strcmp(camposUpdate[k], "codLinha") == 0)        
            registro->codLinha = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
        
        else if (strcmp(camposUpdate[k], "codProxEstacao") == 0)
            registro->codProxEstacao = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
        
        else if (strcmp(camposUpdate[k], "distProxEstacao") == 0)
            registro->distProxEstacao = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
        
        else if (strcmp(camposUpdate[k], "codLinhaIntegra") == 0)
            registro->codLinhaIntegra = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
        
        else if (strcmp(camposUpdate[k], "codEstIntegra") == 0)
            registro->codEstIntegra = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
        
        else if (strcmp(camposUpdate[k], "nomeEstacao") == 0){
            if (strcmp(valoresUpdate[k], "NULO") == 0)
                registro->tamNomeEstacao = 0;
            else{
                registro->tamNomeEstacao = strlen(valoresUpdate[k]);
                memcpy(registro->nomeEstacao, valoresUpdate[k], registro->tamNomeEstacao);
            }
        }
        else if (strcmp(camposUpdate[k], "nomeLinha") == 0){
            if (strcmp(valoresUpdate[k], "NULO") == 0)
                registro->tamNomeLinha = 0;
            else{
                registro->tamNomeLinha = strlen(valoresUpdate[k]);
                memcpy(registro->nomeLinha, valoresUpdate[k], registro->tamNomeLinha);
            }
        }
    }
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

void CreateTable(char *inputFileName, char *outputFileName){
    FILE *entrada = fopen(inputFileName, "r");
    FILE *saida = fopen(outputFileName, "wb+");

    if (entrada == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Inicializa o cabeçalho do arquivo de saída
    Header cabecalho;
    newHeader(&cabecalho);

    // Escreve o cabeçalho no arquivo de saída
    writeHeader(&cabecalho, saida);

    // Variáveis auxiliares para a leitura do CSV
    char buffer[256];   // Buffer para armazenar os caracteres lidos no CSV
    int posBuffer = 0;  // Índice para rastrear a posição atual no buffer
    char c;             // Char que armazena o caractere lido do arquivo de entrada
    int fieldIndex = 0; // Índice para rastrear qual campo do registro está sendo preenchido
    
    Registro regAtual;
    memset(&regAtual, 0, sizeof(Registro));  // Inicializa o registro atual com zeros
    // Preenche os campos variáveis com '$'
    memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
    memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));

    Estacoes nomesEPares = {0};    // Estrutura para armazenar as estações e os pares de estações únicas

    // Pula a primeira linha do arquivo de entrada (cabeçalho)
    while(fread(&c, sizeof(char), 1, entrada) == 1 && c != '\n');

    // Loop de leitura dos campos do arquivo de entrada (caractere a caractere)
    while(fread(&c, sizeof(char), 1, entrada) == 1){
        if (c == '\r')
            continue;
        if (c == ',' || c == '\n'){   // Verifica a condição de final de um campo ou de um registro
            buffer[posBuffer] = '\0'; // Sinaliza o final da string no buffer

            // Função que preenche o campo correspondente no registro atual com base no índice do campo
            writeCampos(buffer, fieldIndex, &regAtual);
            posBuffer = 0; // Sinaliza que o buffer está vazio para a leitura do próximo campo

            // Se o registro foi completamente preenchido (quebra de linha), escreve no binário. Se não, preenche o próximo campo
            if (c == '\n'){
                // Verificar se a estação atual é única
                isEstacaoUnica(&nomesEPares, regAtual.nomeEstacao);
                // Verificar se o par de estações atual é único
                isParUnico(&nomesEPares, regAtual.codEstacao, regAtual.codProxEstacao);

                writeRegistros(&regAtual, saida, &cabecalho);
                cabecalho.proxRRN++; // Incrementa o próximo RRN disponível no cabeçalho para o próximo registro a ser escrito
                fieldIndex = 0; // Os campos foram todos preenchidos, o próximo campo será o primeiro do próximo registro

                // Limpar o registro atual para a leitura do próximo registro
                memset(&regAtual, 0, sizeof(Registro));
                // Preencher campos variáveis com '$' para evitar lixo
                memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
                memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));
            }
            else
                fieldIndex++; // Sinaliza que a leitura de um campo foi concluída, passando para o próximo
        }

        // A leitura do campo ainda não foi concluída, continua lendo os caracteres
        else{
            // Adiciona o caractere ao buffer
            buffer[posBuffer] = c;
            posBuffer++; // Incrementa o índice do buffer para a próxima posição
        }
    }

    if (fieldIndex > 0 || posBuffer > 0){
        buffer[posBuffer] = '\0';
        writeCampos(buffer, fieldIndex, &regAtual);
        isEstacaoUnica(&nomesEPares, regAtual.nomeEstacao);
        isParUnico(&nomesEPares, regAtual.codEstacao, regAtual.codProxEstacao);
        writeRegistros(&regAtual, saida, &cabecalho);
        cabecalho.proxRRN++;
    }

    cabecalho.nroEstacoes = nomesEPares.numEstacoes; // Atribui o número de estações únicas ao campo correspondente no cabeçalho
    cabecalho.nroParesEstacao = nomesEPares.numParesEstacao; // Atribui o número de pares de estações únicas ao campo correspondente no cabeçalho
    cabecalho.status = '1';         // Sinaliza que o arquivo foi criado corretamente
    fseek(saida, 0, SEEK_SET);      // Posiciona o ponteiro do arquivo no início para atualizar o cabeçalho
    writeHeader(&cabecalho, saida); // Atualiza os valores do cabeçalho no arquivo de saída
    fclose(entrada);
    fclose(saida);
    BinarioNaTela(outputFileName);
}

void Select(char *FileName){
    FILE *file = fopen(FileName, "rb");

    // Acessar o cabeçalho do arquivo
    Header cabecalho;
    readHeader(&cabecalho, file);

    // Verificar o status do arquivo
    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return;
    }

    // Verificar se há registros no arquivo
    if (cabecalho.nroEstacoes == 0){
        printf("Registro inexistente.\n");
        fclose(file);
        return;
    }

    // Loop para ler os registros
    Registro regAtual;

    while (1){
        memset(&regAtual, 0, sizeof(Registro));
        // Inicializar campos variáveis com '$'
        memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
        memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));

        if (!readRegistros(&regAtual, file))
            break;

        printRegistros(&regAtual);
    }
    fclose(file);
}

void Where(char *FileName, int nroBuscas){
    FILE *file = fopen(FileName, "rb");

    // Acessar o cabeçalho do arquivo
    Header cabecalho;
    readHeader(&cabecalho, file);
    // Verificar o status do arquivo
    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return;
    }

    int nroCampos; // Variável para armazenar o número de campos a serem buscados em cada busca
    // Ler os N pares de campo e conteúdo
    for (int i = 0; i < nroBuscas; i++){
        // Lê o número de campos a serem buscados para cada busca
        scanf("%d", &nroCampos);
        CriteriosBusca criterios = {0}; // Inicializa a struct de critérios de busca, zerando as flags

        // Ler os campos e conteúdos a serem buscados, preenchendo a struct de critérios de busca
        for (int j = 0; j < nroCampos; j++){
            char campo[30] = {0}; //Limpa o campo para evitar lixo de memória
            char conteudo[30] = {0}; //Limpa o conteúdo para evitar lixo de memória
            scanf("%s", campo);
            //A depender do campo, lê como inteiro ou string
            if (strcmp(campo, "nomeEstacao") == 0 || strcmp(campo, "nomeLinha") == 0)
                ScanQuoteString(conteudo);
            else
                scanf("%s", conteudo);

            // Atribuir flags aos campos a serem buscados
            preencherCriteriosBusca(&criterios, campo, conteudo);
        }

        // Posicionar o ponteiro do arquivo no início dos registros para a execução de múltiplas buscas
        fseek(file, 17, SEEK_SET);

        // Acessar os registros e fazer as verificações para imprimir os registros que atendem às condições de busca
        Registro regAtual;
        int registrosEncontrados = 0; // Variável para contar o número de registros encontrados que atendem aos critérios de busca

        while (1){
            memset(&regAtual, 0, sizeof(Registro));
            // Inicializar campos variáveis com '$' para evitar lixo
            memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
            memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));

            if (!readRegistros(&regAtual, file))
                break;

            if (checagemCriteriosBusca(&criterios, &regAtual)){
                printRegistros(&regAtual);
                printf("\n");
                registrosEncontrados++;
            }
        }
        if (!registrosEncontrados)
            printf("Registro inexistente.\n");
    }
    fclose(file);
}

// Função auxiliar para recalcular estações e pares únicos baseado nos registros ativos
void recalcularEstacoesPares(FILE *arquivoBinario, Header *cabecalho){
    Estacoes estacoes = {0};

    // Posiciona no início dos registros
    fseek(arquivoBinario, 17, SEEK_SET);

    Registro regAtual;

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

void Delete(char *FileName, int nroRemocoes){
    FILE *arquivoBinario = fopen(FileName, "rb+");
    if (arquivoBinario == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Ler o cabeçalho do arquivo e guardar na struct de cabeçalho
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);

    // Verifica consistencia
    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // Marca como inconsistente durante a execucao
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);

    // Loop para fazer as N remoções, lendo os critérios de busca e percorrendo os registros
    for (int i = 0; i < nroRemocoes; i++){
        int m;
        scanf("%d", &m);

        // Zerando as flags manualmente para garantir que lixo de memoria nao interfira
        CriteriosBusca criterios = {0};

        // Preenche a struct dos critérios de busca
        for (int j = 0; j < m; j++){
            char nomeCampo[50];
            char valorCampo[100];
            scanf("%s", nomeCampo);
            //A depender do campo, lê como inteiro ou string
            if (strcmp(nomeCampo, "nomeEstacao") == 0 || strcmp(nomeCampo, "nomeLinha") == 0)
                ScanQuoteString(valorCampo);
            else
                scanf("%s", valorCampo);
            preencherCriteriosBusca(&criterios, nomeCampo, valorCampo);
        }

        Registro regAtual;
        int rrn = 0;

        // Pula o cabecalho de 17 bytes para comecar a ler os registros
        fseek(arquivoBinario, 17, SEEK_SET);

        // Percorre os registros para encontrar os que atendem aos critérios de remoção
        while (readRegistros(&regAtual, arquivoBinario)){
            // Se nao esta removido e bate com a busca
            if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)){
                // Atualiza flags de remocao
                regAtual.removido = '1';
                regAtual.proximo = cabecalho.topo; // aponta pro antigo topo da pilha
                cabecalho.topo = rrn;              // atualiza o topo do cabecalho com o RRN atual
                
                // Calcula a posicao exata do inicio deste registro (17 do cabecalho + rrn * 80 do tamanho fixo)
                long posicaoRegistro = 17 + (rrn * 80);
                fseek(arquivoBinario, posicaoRegistro, SEEK_SET);

                // Escreve apenas os campos alterados para manter os outros bytes intactos
                fwrite(&regAtual.removido, sizeof(char), 1, arquivoBinario);
                fwrite(&regAtual.proximo, sizeof(int), 1, arquivoBinario);

                // Retorna o ponteiro para o final do registro atual para a proxima iteracao do while nao quebrar
                fseek(arquivoBinario, posicaoRegistro + 80, SEEK_SET);
            }
            rrn++;
        }
        // Volta para o primeiro registro para a proxima busca do laco nroRemocoes
        fseek(arquivoBinario, 17, SEEK_SET);
    }

    // Recalcula estações e pares únicos baseado nos registros ainda ativos
    recalcularEstacoesPares(arquivoBinario, &cabecalho);

    // Retorna status para consistente e salva cabecalho
    cabecalho.status = '1';
    // Atualiza o cabeçalho no arquivo
    writeHeader(&cabecalho, arquivoBinario);
    fclose(arquivoBinario);
    BinarioNaTela(FileName);
}

void Insert(char *FileName, int nroInsercoes){
    FILE *arquivoBinario = fopen(FileName, "rb+");
    if (arquivoBinario == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);

    // Verifica consistencia
    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // Marca como inconsistente
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);

    for (int i = 0; i < nroInsercoes; i++){
        Registro novoReg;
        // Inicializar o registro com zeros para evitar lixo
        memset(&novoReg, 0, sizeof(Registro));
        // Inicializar campos variáveis com '$' para evitar lixo
        memset(novoReg.nomeEstacao, '$', sizeof(novoReg.nomeEstacao));
        memset(novoReg.nomeLinha, '$', sizeof(novoReg.nomeLinha));
        preencherNovoRegistro(&novoReg);
    
        // Lógica de reaproveitamento de espaço
        long posicaoEscrita;

        if (cabecalho.topo == -1){
            // Escreve no final do arquivo
            posicaoEscrita = 17 + (cabecalho.proxRRN * 80);
            cabecalho.proxRRN++;
        }
        else{
            // Reaproveita o espaco do topo da pilha
            int rrnReaproveitado = cabecalho.topo;
            posicaoEscrita = 17 + (rrnReaproveitado * 80);

            // Le o campo de proximo do registro atual
            fseek(arquivoBinario, posicaoEscrita + 1, SEEK_SET); // Pula 1 byte do 'removido'
            int proximoDaPilha;
            fread(&proximoDaPilha, sizeof(int), 1, arquivoBinario);

            cabecalho.topo = proximoDaPilha; // Atualiza o topo do cabecalho
        }
        fseek(arquivoBinario, posicaoEscrita, SEEK_SET);

        //Escrever o registro no arquivo binário
        writeRegistros(&novoReg, arquivoBinario, &cabecalho);
    }

    // Recalcula estações e pares únicos baseado nos registros inclusos
    recalcularEstacoesPares(arquivoBinario, &cabecalho);

    cabecalho.status = '1';
    writeHeader(&cabecalho, arquivoBinario);
    fclose(arquivoBinario);
    BinarioNaTela(FileName);
}

void Update(char *FileName, int nroAtualizacoes) {
        FILE *arquivoBinario = fopen(FileName, "rb+");
        if (arquivoBinario == NULL) {
            printf("Falha no processamento do arquivo.\n");
            return;
        }

        Header cabecalho;
        readHeader(&cabecalho, arquivoBinario);

        // Verifica consistencia
        if (cabecalho.status == '0') {
            printf("Falha no processamento do arquivo.\n");
            fclose(arquivoBinario);
            return;
        }

        for (int i = 0; i < nroAtualizacoes; i++) {
            int m;
            scanf("%d", &m);
            CriteriosBusca criterios = {0}; // Inicializa a struct de critérios de busca, zerando as flags

            // Le as condicoes de busca (WHERE)
            for (int j = 0; j < m; j++) {
                char nomeCampo[50];
                char valorCampo[100];
                scanf("%s", nomeCampo);
                //A depender do campo, lê como inteiro ou string
                if (strcmp(nomeCampo, "nomeEstacao") == 0 || strcmp(nomeCampo, "nomeLinha") == 0)
                    ScanQuoteString(valorCampo);
                else
                    scanf("%s", valorCampo);
                preencherCriteriosBusca(&criterios, nomeCampo, valorCampo);
            }

            //Cria matrizes para guardar os campos que serão atualizados e seus respectivos valores
            int p;
            scanf("%d", &p);
            char camposUpdate[10][50];
            char valoresUpdate[10][100];
            
            // Le os campos a serem atualizados (SET)
            for (int k = 0; k < p; k++) {
                scanf("%s", camposUpdate[k]);
                //A depender do campo, lê como inteiro ou string
                if (strcmp(camposUpdate[k], "nomeEstacao") == 0 || strcmp(camposUpdate[k], "nomeLinha") == 0)
                    ScanQuoteString(valoresUpdate[k]);
                else
                    scanf("%s", valoresUpdate[k]);
            }

            Registro regAtual;
            int rrn = 0;

            //Criar função para pular registro (modularizar)
            fseek(arquivoBinario, 17, SEEK_SET);

            // Busca sequencial no arquivo
            while (readRegistros(&regAtual, arquivoBinario)) {
                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    // Aplica atualizacoes no registro em memoria
                    updateRegistro(&regAtual, camposUpdate, valoresUpdate, p);
                    
                    // Volta para o inicio do registro para sobrescrever
                    long posicaoRegistro = 17 + (rrn * 80);
                    fseek(arquivoBinario, posicaoRegistro, SEEK_SET);
                    
                    //Atualiza os registros atualizados no arquivo binário
                    writeRegistros(&regAtual, arquivoBinario, &cabecalho);

                    // Pula para o final deste registro para continuar o while
                    fseek(arquivoBinario, posicaoRegistro + 80, SEEK_SET);
                }
                rrn++;
            }
            //Volta para o início dos registros 
            fseek(arquivoBinario, 17, SEEK_SET);
        }

        // Recalcula estações e pares únicos baseado nos registros atualizados
        recalcularEstacoesPares(arquivoBinario, &cabecalho);

        cabecalho.status = '1';
        writeHeader(&cabecalho, arquivoBinario);
        fclose(arquivoBinario);
        BinarioNaTela(FileName);
    }
