#include "register.h"
#include "utils.h"

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

int readRegistros(Registro *registro, FILE *file){
    // Lê apenas 1 byte
    if (fread(&registro->removido, sizeof(char), 1, file) != 1)
        return 0; // Fim do arquivo

    // Se estiver removido, pula o resto do registro e retorna um código específico para indicar que o registro foi pulado
    if (registro->removido == '1') {
        fseek(file, 79, SEEK_CUR); // Pula os 79 bytes restantes
        return 2; // 2 é o código para indicar que o registro foi removido e pulado
    }

    char bufferRegistro[79]; // Buffer para armazenar os bytes do registro lidos do arquivo
    int offset = 0;

    // Lê 80 bytes do arquivo para preencher o buffer do registro
    if (fread(bufferRegistro, sizeof(char), 79, file) != 79)
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