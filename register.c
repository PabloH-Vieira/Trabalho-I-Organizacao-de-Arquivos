#include "register.h"
#include "utils.h"

void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual){
    // ROTEAMENTO DE COLUNAS (Mapeamento CSV -> Struct)
    switch (fieldIndex){
        
        // --- MAPEAMENTO DE INTEIROS E TRATAMENTO DE NULOS ---
        // A especificação exige que campos de tamanho fixo com valores nulos 
        // sejam obrigatoriamente representados pelo inteiro -1.
        case 0:
            regAtual->codEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
            
        // --- TRATAMENTO DE STRINGS DE TAMANHO VARIÁVEL ---
        case 1:
            // O indicador de tamanho guarda a quantidade real de caracteres da string.
            // Para valores nulos (buffer vazio), o strlen retornará 0, satisfazendo a 
            // regra de armazenamento nulo para strings variáveis.
            regAtual->tamNomeEstacao = strlen(buffer);
            
            // TRAVA DE SEGURANÇA (PREVENÇÃO DE OVERFLOW)
            // Impede que strings muito compridas no CSV corrompam os bytes adjacentes da struct.
            if (regAtual->tamNomeEstacao > 28) 
                regAtual->tamNomeEstacao = 28;
                
            memcpy(regAtual->nomeEstacao, buffer, regAtual->tamNomeEstacao);
            
            // Finalizador do campo somente em RAM
            // garante a segurança da leitura em memória durante comparações e impressões).
            regAtual->nomeEstacao[regAtual->tamNomeEstacao] = '\0';
            break;
            
        case 2:
            regAtual->codLinha = (buffer[0] == '\0') ? -1 : atoi(buffer);
            break;
            
        case 3:
            regAtual->tamNomeLinha = strlen(buffer);
            
            // TRAVA DE SEGURANÇA para o segundo campo variável
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
    // INICIALIZAÇÃO DOS CAMPOS NO REGISTRO
    // Garante que todo novo registro escrito nasça como ativo
    // e sem apontar para lixo na pilha de removidos.
    registro->removido = '0';
    registro->proximo = -1;

    // ALOCAÇÃO DO BUFFER DE PREENCHIMENTO DE CAMPOS
    // Cria um espaço de exatos 80 bytes.
    char bufferRegistro[80];
    
    // PREVENÇÃO DE LIXO DE MEMÓRIA
    // Inicializa toda a extensão do buffer com '$'. Isso garante que qualquer byte 
    // não sobrescrito pelos memcpy() a seguir satisfaça a restrição de lixo do arquivo.
    memset(bufferRegistro, '$', 80); 

    // VARIÁVEL DE DESLOCAMENTO PARA O BUFFER
    int offset = 0;

    // PREENCHIMENTO DOS CAMPOS DE TAMANHO FIXO (29 bytes no total)
    // A ordem de cópia segue estritamente o layout visual exigido.
    
    // Status e Ponteiro (5 bytes)
    memcpy(bufferRegistro + offset, &registro->removido, sizeof(char));
    offset += sizeof(char);

    memcpy(bufferRegistro + offset, &registro->proximo, sizeof(int));
    offset += sizeof(int);

    // Identificadores de Estação e Linha (8 bytes)
    memcpy(bufferRegistro + offset, &registro->codEstacao, sizeof(int));
    offset += sizeof(int);

    memcpy(bufferRegistro + offset, &registro->codLinha, sizeof(int));
    offset += sizeof(int);

    // Identificadores de Próxima Estação e Distância (8 bytes)
    memcpy(bufferRegistro + offset, &registro->codProxEstacao, sizeof(int));
    offset += sizeof(int);

    memcpy(bufferRegistro + offset, &registro->distProxEstacao, sizeof(int));
    offset += sizeof(int);

    // Identificadores de Integração (8 bytes)
    memcpy(bufferRegistro + offset, &registro->codLinhaIntegra, sizeof(int));
    offset += sizeof(int);

    memcpy(bufferRegistro + offset, &registro->codEstIntegra, sizeof(int));
    offset += sizeof(int);

    // --- PREENCHIMENTO DOS CAMPOS DE TAMANHO VARIÁVEL ---
    
    // A reatribuição garante segurança absoluta caso o tamanho dos tipos na arquitetura base sofra alterações.
    offset = 29;
    
    // Nome da Estação (Variável 1)
    memcpy(bufferRegistro + offset, &registro->tamNomeEstacao, sizeof(int));
    offset += sizeof(int);
    
    // O if previne acessos ilegais à memória caso a string tenha tamanho 0 (Nulo)
    if (registro->tamNomeEstacao > 0){
        memcpy(bufferRegistro + offset, registro->nomeEstacao, registro->tamNomeEstacao);
        offset += registro->tamNomeEstacao;
    }

    // Nome da Linha (Variável 2)
    memcpy(bufferRegistro + offset, &registro->tamNomeLinha, sizeof(int));
    offset += sizeof(int);
    
    if (registro->tamNomeLinha > 0){
        memcpy(bufferRegistro + offset, registro->nomeLinha, registro->tamNomeLinha);
        // Não é necessário atualizar o offset aqui, pois a linha é a última variável,
        // e o fwrite gravará o vetor limitando estritamente em 80 bytes físicos.
    }

    // PERSISTÊNCIA EM DISCO
    // Grava o buffer no disco.
    fwrite(bufferRegistro, sizeof(char), 80, saida);
}

int readRegistros(Registro *registro, FILE *file){
    // AVALIAÇÃO DE STATUS
    // Percorre o 1º byte do registro para avaliar a sua validade.
    // O fread garante a detecção correta do Fim de Arquivo (EOF).
    if (fread(&registro->removido, sizeof(char), 1, file) != 1)
        return 0; 

    // TRATAMENTO DE REGISTRO DESCARTADO
    // O status '1' indica remoção lógica. 
    // Otimização de I/O: Ao invés de carregar o lixo na RAM, dá um salto físico (fseek)
    // consumindo os 79 bytes restantes, posicionado o cursor no início do próximo registro.
    if (registro->removido == '1') {
        fseek(file, 79, SEEK_CUR); 
        return 2; // Código para indicar que ocorreu um salto
    }

    // CARREGAMENTO SEQUENCIAL
    // A especificação define que registros ocupam sempre 80 bytes no disco.
    // Carrega os 79 bytes restantes de uma só vez para a RAM, engolindo automaticamente todo o lixo ('$').
    char bufferRegistro[79]; 
    if (fread(bufferRegistro, sizeof(char), 79, file) != 79)
        return 0; 
    
    int offset = 0; // Cursor lógico de navegação dentro do buffer da RAM

    // DESEMPACOTAMENTO DOS CAMPOS DE TAMANHO FIXO
    // O uso de memcpy extrai os bytes na sequência física do arquivo.
    
    memcpy(&registro->proximo, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&registro->codEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&registro->codLinha, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&registro->codProxEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&registro->distProxEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&registro->codLinhaIntegra, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    memcpy(&registro->codEstIntegra, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);

    // DESEMPACOTAMENTO DAS STRINGS (CAMPOS VARIÁVEIS)
    
    // Alinhamento de segurança: Garante que a extração variável inicie no byte exato (29 total - 1 do removido).
    offset = 28;
    
    // NOME DA ESTAÇÃO
    memcpy(&registro->tamNomeEstacao, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);
    
    // DEFESA CONTRA BUFFER OVERFLOW
    // Protege a memória contra valores corrompidos no indicador de tamanho.
    if (registro->tamNomeEstacao < 0 || registro->tamNomeEstacao > 28){
        registro->tamNomeEstacao = 0;
        registro->nomeEstacao[0] = '\0';
    }
    else if (registro->tamNomeEstacao > 0){
        memcpy(registro->nomeEstacao, bufferRegistro + offset, registro->tamNomeEstacao);
        // O '\0' é inserido localmente na struct para permitir uso de funções como printf,
        // mas não existe fisicamente no disco, conforme a especificação.
        registro->nomeEstacao[registro->tamNomeEstacao] = '\0';
        offset += registro->tamNomeEstacao;
    }

    // NOME DA LINHA
    memcpy(&registro->tamNomeLinha, bufferRegistro + offset, sizeof(int));
    offset += sizeof(int);
    
    // DEFESA CONTRA BUFFER OVERFLOW
    if (registro->tamNomeLinha < 0 || registro->tamNomeLinha > 15){
        registro->tamNomeLinha = 0;
        registro->nomeLinha[0] = '\0';
    }
    else if (registro->tamNomeLinha > 0){
        memcpy(registro->nomeLinha, bufferRegistro + offset, registro->tamNomeLinha);
        registro->nomeLinha[registro->tamNomeLinha] = '\0';
    }

    return 1; // Leitura estritamente dentro dos padrões, retorno de sucesso
}

void printRegistros(Registro *registro){
    // FILTRO DE EXIBIÇÃO PARA REGISTROS ATIVOS
    // A especificação proíbe terminantemente a exibição de registros que estejam 
    // marcados como logicamente removidos no disco.
    if (registro->removido == '0'){
        
        // FORMATAÇÃO SEQUENCIAL EM LINHA ÚNICA
        // Todos os campos de um registro válido devem ser impressos em uma única linha, 
        // separados por um espaço em branco. A ordem das instruções obedece 
        // ao layout exigido de exibição.
        
        // 1. CÓDIGO DA ESTAÇÃO
        // O valor numérico -1 na struct de memória indica ausência de dados, 
        // devendo ser convertido para a string visual "NULO".
        if (registro->codEstacao == -1)
            printf("NULO ");
        else 
            printf("%d ", registro->codEstacao);

        // 2. NOME DA ESTAÇÃO (Campo Variável)
        // Para strings dinâmicas, o tamanho armazenado igual a 0 representa campo vazio.
        if (registro->tamNomeEstacao == 0)
            printf("NULO ");
        else
            printf("%s ", registro->nomeEstacao);

        // 3. CÓDIGO DA LINHA
        if (registro->codLinha == -1)
            printf("NULO ");
        else
            printf("%d ", registro->codLinha);

         // 4. NOME DA LINHA (Campo Variável)
        if (registro->tamNomeLinha == 0)
            printf("NULO ");
        else
            printf("%s ", registro->nomeLinha);

        // 5. CÓDIGO DA PRÓXIMA ESTAÇÃO
        if (registro->codProxEstacao == -1)
            printf("NULO ");
        else
            printf("%d ", registro->codProxEstacao);
        
        // 6. DISTÂNCIA PARA A PRÓXIMA ESTAÇÃO
        if (registro->distProxEstacao == -1)
            printf("NULO ");
        else
            printf("%d ", registro->distProxEstacao);
        
        // 7. CÓDIGO DA LINHA DE INTEGRAÇÃO
        if (registro->codLinhaIntegra == -1)
            printf("NULO ");
        else
            printf("%d ", registro->codLinhaIntegra);
        
        // 8. CÓDIGO DA ESTAÇÃO DE INTEGRAÇÃO
        // Como este é o último campo obrigatório da sequência, ele determina 
        // a quebra de linha ('\n') em vez do espaço em branco, finalizando o registro
        // e preparando o terminal para a leitura visual do próximo.
        if (registro->codEstIntegra == -1)
            printf("NULO\n");
        else
            printf("%d\n", registro->codEstIntegra);
    }
}

void preencherNovoRegistro(Registro *novoReg){
    char bufferAux[100];
    
    // LEITURA DE CHAVE PRIMÁRIA
    // O código da estação é o único campo estritamente obrigatório (não aceita nulos).
    scanf("%d", &novoReg->codEstacao);

    // PROCESSAMENTO DE STRINGS (TAMANHO VARIÁVEL)
    // A função ScanQuoteString remove as aspas duplas exigidas pela especificação 
    // e isola a string real, permitindo calcular o tamanho exato.
    ScanQuoteString(bufferAux);
    novoReg->tamNomeEstacao = strlen(bufferAux);
    memcpy(novoReg->nomeEstacao, bufferAux, novoReg->tamNomeEstacao);

    // MAPEAMENTO DE INTEIROS E TRATAMENTO DE 'NULO'
    // Lê o buffer como string primeiro para interceptar a palavra-chave "NULO" exigida
    // na entrada da funcionalidade [5] e converte adequadamente para -1.
    scanf("%s", bufferAux);
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codLinha = -1;
    else
        novoReg->codLinha = atoi(bufferAux);

    // Nome da Linha (Variável 2)
    ScanQuoteString(bufferAux);
    novoReg->tamNomeLinha = strlen(bufferAux);
    memcpy(novoReg->nomeLinha, bufferAux, novoReg->tamNomeLinha);

    // Código da Próxima Estação
    scanf("%s", bufferAux);
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codProxEstacao = -1;
    else
        novoReg->codProxEstacao = atoi(bufferAux);

    // Distância para a Próxima Estação
    scanf("%s", bufferAux);
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->distProxEstacao = -1;
    else
        novoReg->distProxEstacao = atoi(bufferAux);

    // Código da Linha de Integração
    scanf("%s", bufferAux);
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codLinhaIntegra = -1;
    else
        novoReg->codLinhaIntegra = atoi(bufferAux);

    // Código da Estação de Integração
    scanf("%s", bufferAux);
    if (strcmp(bufferAux, "NULO") == 0)
        novoReg->codEstIntegra = -1;
    else
        novoReg->codEstIntegra = atoi(bufferAux);

    // INICIALIZAÇÃO DOS CAMPOS DE METADADOS DO REGISTRO
    // Como se trata de uma inserção nativa, o registro nasce como válido ('0') e 
    // desvinculado da pilha de reaproveitamento de espaço (-1).
    novoReg->removido = '0';
    novoReg->proximo = -1;
}

void updateRegistro(Registro *registro, char camposUpdate[][50], char valoresUpdate[][100], int p){
    // PROCESSAMENTO DAS ATUALIZAÇÕES
    // Itera sobre o número de atualizações 'p' requisitadas para este registro específico.
    for (int k = 0; k < p; k++){
        
        // ATUALIZAÇÃO DE CAMPOS FIXOS (Chaves e Numéricos)
        // A especificação define que valores nulos informados pelo usuário ("NULO") 
        // devem substituir o conteúdo atual pelo inteiro -1.
        
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
        
        // ATUALIZAÇÃO DE CAMPOS VARIÁVEIS (Strings)
        // A especificação define que campos variáveis marcados como "NULO" não devem 
        // ocupar espaço além do indicador de tamanho, que deve receber 0.
        
        else if (strcmp(camposUpdate[k], "nomeEstacao") == 0){
            if (strcmp(valoresUpdate[k], "NULO") == 0)
                registro->tamNomeEstacao = 0; // Esvazia o registro logicamente
            else{
                // Expande ou altera o conteúdo da string e atualiza o indicador de tamanho
                registro->tamNomeEstacao = strlen(valoresUpdate[k]);
                memcpy(registro->nomeEstacao, valoresUpdate[k], registro->tamNomeEstacao);
            }
        }
        
        else if (strcmp(camposUpdate[k], "nomeLinha") == 0){
            if (strcmp(valoresUpdate[k], "NULO") == 0)
                registro->tamNomeLinha = 0; // Esvazia o registro logicamente
            else{
                // Expande ou altera o conteúdo da string e atualiza o indicador de tamanho
                registro->tamNomeLinha = strlen(valoresUpdate[k]);
                memcpy(registro->nomeLinha, valoresUpdate[k], registro->tamNomeLinha);
            }
        }
    }
}