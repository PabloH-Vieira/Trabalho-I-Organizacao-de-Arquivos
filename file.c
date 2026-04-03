#include "file.h"
#include <stdio.h>

    void newHeader(Header *header){
        header->status = '0';
        header->topo = -1;
        header->proxRRN = 0;
        header->nroEstacoes = 0;
        header->nroParesEstacao = 0;
    }

    void writeHeader(Header *header, FILE* file){
        fseek(file, 0, SEEK_SET);
        //Escreve o cabeçalho no arquivo campo a campo para evitar o padding
        fwrite(&header->status, sizeof(char), 1, file);
        fwrite(&header->topo, sizeof(int), 1, file);
        fwrite(&header->proxRRN, sizeof(int), 1, file);
        fwrite(&header->nroEstacoes, sizeof(int), 1, file);
        fwrite(&header->nroParesEstacao, sizeof(int), 1, file);
    }

    void readHeader(Header *header, FILE* file){
        fseek(file, 0, SEEK_SET);
        fread(&header->status, sizeof(char), 1, file);
        fread(&header->topo, sizeof(int), 1, file);
        fread(&header->proxRRN, sizeof(int), 1, file);
        fread(&header->nroEstacoes, sizeof(int), 1, file);
        fread(&header->nroParesEstacao, sizeof(int), 1, file);
    }

    void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual){
        switch(fieldIndex){
            case 0:
                //Verifica se o campo tem valor nulo
                regAtual->codEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 1:
                strcpy(regAtual->nomeEstacao, buffer);
                //Passa o tamanho do nome da estação
                regAtual->tamNomeEstacao = strlen(regAtual->nomeEstacao);
                break;
            case 2:
                regAtual->codLinha = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 3:
                strcpy(regAtual->nomeLinha, buffer);
                //Passa o tamanho do nome da linha
                regAtual->tamNomeLinha = strlen(regAtual->nomeLinha);
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

    void writeRegistros(Registro *registro, FILE* saida, Header *cabecalho){
        //Escreve o registro no arquivo de saída
        fwrite(registro, sizeof(Registro), 1, saida);
        cabecalho->proxRRN++; //Incrementa o RRN
        cabecalho->nroEstacoes++; //Incrementa o número de estações

        //Preenche campos não presentes no CSV
        registro->removido = '0';
        registro->proximo = -1;
    }

    int readRegistros(Registro *registro, FILE* file){
            //Lê o registro campo a campo
            if(fread(&registro -> removido, sizeof(char), 1, file) != 1)
                return 0; //Retorna 0 se não foi possível ler o campo de "removido", indicando o fim do arquivo
            fread(&registro->proximo, sizeof(int), 1, file);
            fread(&registro->codEstacao, sizeof(int), 1, file);
            fread(&registro->codLinha, sizeof(int), 1, file);
            fread(&registro->codProxEstacao, sizeof(int), 1, file);
            fread(&registro->distProxEstacao, sizeof(int), 1, file);
            fread(&registro->codLinhaIntegra, sizeof(int), 1, file);
            fread(&registro->codEstIntegra, sizeof(int), 1, file);
            fread(&registro->tamNomeEstacao, sizeof(int), 1, file);
            fread(&registro->nomeEstacao, sizeof(char), registro->tamNomeEstacao, file);
            registro->nomeEstacao[registro->tamNomeEstacao] = '\0'; //Caractere de terminação da string
            fread(&registro->tamNomeLinha, sizeof(int), 1, file);
            fread(&registro->nomeLinha, sizeof(char), registro->tamNomeLinha, file);
            registro->nomeLinha[registro->tamNomeLinha] = '\0'; //Caractere de terminação da string
            return 1; //Retorna 1 se o registro foi lido com sucesso
    }

    void printRegistros(Registro *registro){
        //verifica se o registro foi removido logicamente. Se não, imprime os campos do registro
        if(registro->removido == '0'){
            //Imprime o campo de código da estação, com verificação de valor nulo
            if (registro->codEstacao == -1)
                printf("NULO ");
            else 
                printf("%d ", registro->codEstacao);

            //Imprime o campo de nome da estação
            printf("%s ", registro->nomeEstacao);

             //Imprime o campo de código da linha, com verificação de valor nulo
            if (registro->codLinha == -1)
                printf("NULO ");
            else
                printf("%d ", registro->codLinha);

            //Imprime o campo de nome da linha
            printf("%s ", registro->nomeLinha);

            //Imprime o campo de código da próxima estação, com verificação de valor nulo
            if (registro->codProxEstacao == -1)
                printf("NULO ");
            else
                printf("%d ", registro->codProxEstacao);
            
            //Imprime o campo de distância para a próxima estação, com verificação de valor nulo
            if (registro->distProxEstacao == -1)
                printf("NULO ");
            else
                printf("%d ", registro->distProxEstacao);
            
            //Imprime o campo de código da linha de integração, com verificação de valor nulo
            if (registro->codLinhaIntegra == -1)
                printf("NULO ");
            else
                printf("%d ", registro->codLinhaIntegra);
            
            //Imprime o campo de código da estação de integração, com verificação de valor nulo
            if (registro->codEstIntegra == -1)
                printf("NULO\n");
            else
                printf("%d\n", registro->codEstIntegra);
        }
    }

    void preencherCriteriosBusca(CriteriosBusca *criterios, char *campo, char *conteudo){
        //Verifica se o campo é o de código da estação. Se sim, ativa a flag e atribui o valor a ser buscado
        if (strcmp(campo, "codEstacao") == 0) {
            criterios -> flag_codEstacao = 1;
            criterios->regBusca.codEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);

        //Verifica se o campo é o de nome da estação. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "nomeEstacao") == 0) {
            criterios -> flag_nomeEstacao = 1;
            strcpy(criterios->regBusca.nomeEstacao, conteudo);
            
        //Verifica se o campo é o de código da linha. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "codLinha") == 0) {
            criterios -> flag_codLinha = 1;
            criterios->regBusca.codLinha = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);

        //Verifica se o campo é o de nome da linha. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "nomeLinha") == 0) {
            criterios -> flag_nomeLinha = 1;
            strcpy(criterios->regBusca.nomeLinha, conteudo);

        //Verifica se o campo é o de código da próxima estação. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "codProxEstacao") == 0) {
            criterios -> flag_codProxEstacao = 1;
            criterios->regBusca.codProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);

        //Verifica se o campo é o de distância para a próxima estação. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "distProxEstacao") == 0) {
            criterios -> flag_distProxEstacao = 1;
            criterios -> regBusca.distProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);

        //Verifica se o campo é o de código da linha de integração. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "codLinhaIntegra") == 0) {
            criterios -> flag_codLinhaIntegra = 1;
            criterios -> regBusca.codLinhaIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);

        //Verifica se o campo é o de código da estação de integração. Se sim, ativa a flag e atribui o valor a ser buscado
        } else if (strcmp(campo, "codEstIntegra") == 0) {
            criterios -> flag_codEstIntegra = 1;
            criterios -> regBusca.codEstIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
        } 
    }

    int checagemCriteriosBusca(CriteriosBusca *criterios, Registro *regAtual){
        if(regAtual -> removido == '1')
            return 0; //Registro foi removido logicamente, não atende aos critérios de busca

        //Verifica se o campo de código da estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_codEstacao == 1 && regAtual->codEstacao != criterios -> regBusca.codEstacao)
            return 0;

        //Verifica se o campo de nome da estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_nomeEstacao == 1 && strcmp(regAtual->nomeEstacao, criterios -> regBusca.nomeEstacao) != 0)
            return 0;

        //Verifica se o campo de código da linha é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_codLinha == 1 && regAtual->codLinha != criterios -> regBusca.codLinha)
            return 0;

        //Verifica se o campo de nome da linha é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_nomeLinha == 1 && strcmp(regAtual->nomeLinha, criterios -> regBusca.nomeLinha) != 0)
            return 0;

        //Verifica se o campo de código da próxima estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_codProxEstacao == 1 && regAtual->codProxEstacao != criterios -> regBusca.codProxEstacao)
            return 0;

        //Verifica se o campo de distância para a próxima estação é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_distProxEstacao == 1 && regAtual->distProxEstacao != criterios -> regBusca.distProxEstacao)
            return 0;

        //Verifica se o campo de código da linha de integração é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_codLinhaIntegra == 1 && regAtual->codLinhaIntegra != criterios -> regBusca.codLinhaIntegra)
            return 0;

        //Verifica se o campo de código da estação de integração é um critério de busca ativo e se o valor do registro atual é diferente do valor buscado. Se sim, o registro não atende aos critérios de busca
        if (criterios -> flag_codEstIntegra == 1 && regAtual->codEstIntegra != criterios -> regBusca.codEstIntegra)
            return 0;

        return 1; //O registro atende a todos os critérios de busca
    }

    void BinarioNaTela(char *arquivo) {
        FILE *fs;
        if (arquivo == NULL || !(fs = fopen(arquivo, "rb"))) {
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
        for (unsigned long i = 0; i < fl; i++) {
            cs += (unsigned long)mb[i];
        }

        printf("%lf\n", (cs / (double)100));

        free(mb);
        fclose(fs);
    }

    void ScanQuoteString(char *str) {
        char R;

        while ((R = getchar()) != EOF && isspace(R))
            ; // ignorar espaços, \r, \n...

        if (R == 'N' || R == 'n') { // campo NULO
            getchar();
            getchar();
            getchar();       // ignorar o "ULO" de NULO.
            strcpy(str, ""); // copia string vazia
        } else if (R == '\"') {
            if (scanf("%[^\"]", str) != 1) { // ler até o fechamento das aspas
                strcpy(str, "");
            }
            getchar();         // ignorar aspas fechando
        } else if (R != EOF) { // vc tá tentando ler uma string que não tá entre
                            // aspas! Fazer leitura normal %s então, pois deve
                            // ser algum inteiro ou algo assim...
            str[0] = R;
            scanf("%s", &str[1]);
        } else { // EOF
            strcpy(str, "");
        }
    }

    void CreateTable(char *inputFileName, char *outputFileName) {
        FILE* entrada = fopen(inputFileName, "r");

        //Justificar o wb+
        FILE* saida = fopen(outputFileName, "wb+");

        //Inicializa o cabeçalho do arquivo de saída
        Header cabecalho;
        newHeader(&cabecalho);

        //Escreve o cabeçalho no arquivo de saída
        writeHeader(&cabecalho, saida);

        //Variáveis auxiliares para a leitura do CSV
        char buffer[256];   //Buffer para armazenar os caracteres lidos no CSV
        int posBuffer = 0; //Índice para rastrear a posição atual no buffer
        char c; //Char que armazena o caractere lido do arquivo de entrada
        int fieldIndex = 0; //Índice para rastrear qual campo do registro está sendo preenchido
        Registro regAtual;

        //Pula a primeira linha do arquivo de entrada (cabeçalho)
        while(fread(&c, sizeof(char), 1, entrada) == 1 && c != '\n');

        //Loop de leitura dos campos do arquivo de entrada (caractere a caractere)
        while(fread(&c, sizeof(char), 1, entrada) == 1){
            if(c == ',' || c == '\n'){ //Verifica a condição de final de um campo ou de um registro
                buffer[posBuffer] = '\0'; //Sinaliza o final da string no buffer

                //Função que preenche o campo correspondente no registro atual com base no índice do campo
                writeCampos(buffer, fieldIndex, &regAtual);
                posBuffer = 0; //Sinaliza que o buffer está vazio para a leitura do próximo campo
                
                //Se o registro foi completamente preenchido (quebra de linha), escreve no binário. Se não, preenche o próximo campo
                if (c == '\n') {
                    writeRegistros(&regAtual, saida, &cabecalho);
                    fieldIndex = 0; //Os campos foram todos preenchidos, o próximo campo será o primeiro do próximo registro
                } else {
                    fieldIndex++; //Sinaliza que a leitura de um campo foi concluída, passando para o próximo
                }
            }
            
            //A leitura do campo ainda não foi concluída, continua lendo os caracteres
            else{
                //Adiciona o caractere ao buffer
                buffer[posBuffer] = c;
                posBuffer++; //Incrementa o índice do buffer para a próxima posição
            }
        }

        cabecalho.status = '1'; //Sinaliza que o arquivo foi criado corretamente
        fseek(saida, 0, SEEK_SET); //Posiciona o ponteiro do arquivo no início para atualizar o cabeçalho
        writeHeader(&cabecalho, saida); //Atualiza os valores do cabeçalho no arquivo de saída
        fclose(entrada);
        fclose(saida);
        BinarioNaTela(outputFileName);
    }

    void Select(char *FileName){
        FILE *file = fopen(FileName, "rb");
        
        //Acessar o cabeçalho do arquivo
        Header cabecalho;
        readHeader(&cabecalho, file);

        //Verificar o status do arquivo
        if (cabecalho.status == '0') {
            printf("Falha no processamento do arquivo.\n");
            fclose(file);
            return;
        }

        //Verificar se há registros no arquivo
        if (cabecalho.nroEstacoes == 0) {
            printf("Registro inexistente.\n");
            fclose(file);
            return;
        }

        //Loop para ler os registros
        Registro regAtual;
        int totalRegistros = cabecalho.nroEstacoes;
        //A condição de parada do loop é quando a função readRegistros retornar 0, indicando que não há mais registros para ler
        while(readRegistros(&regAtual, file))
            printRegistros(&regAtual);
        fclose(file);
    }

    void Where(char *FileName, int nroBuscas){
        FILE *file = fopen(FileName, "rb");

        //Acessar o cabeçalho do arquivo
        Header cabecalho;
        readHeader(&cabecalho, file);
        //Verificar o status do arquivo
        if (cabecalho.status == '0') {
            printf("Falha no processamento do arquivo.\n");
            fclose(file);
            return;
        }

        int nroCampos; //Variável para armazenar o número de campos a serem buscados em cada busca
        //Ler os N pares de campo e conteúdo
        for(int i = 0; i < nroBuscas; i++){
            //Lê o número de campos a serem buscados para cada busca
            scanf("%d", &nroCampos);
            CriteriosBusca criterios = {0}; //Inicializa a struct de critérios de busca, zerando as flags

            //Ler os campos e conteúdos a serem buscados, preenchendo a struct de critérios de busca
            for (int j = 0; j < nroCampos; j++){
                char campo[20];
                char conteudo[21];
                scanf("%s", campo);
                ScanQuoteString(conteudo);

                //Atribuir flags aos campos a serem buscados
                preencherCriteriosBusca(&criterios, campo, conteudo);
            }

            //Posicionar o ponteiro do arquivo no início dos registros para a execução de múltiplas buscas
            fseek(file, 17, SEEK_SET);

            //Acessar os registros e fazer as verificações para imprimir os registros que atendem às condições de busca
            Registro regAtual;
            int registrosEncontrados = 0; //Variável para contar o número de registros encontrados que atendem aos critérios de busca
            while(readRegistros(&regAtual, file)){
                if (checagemCriteriosBusca(&criterios, &regAtual)){
                    printRegistros(&regAtual);
                    registrosEncontrados++;
                }
            }
            if (!registrosEncontrados)
                printf("Registro inexistente.\n");
        }
        fclose(file);
    }