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
                //Passa o tamanho do nome da estação
                regAtual->tamNomeEstacao = strlen(buffer);
                memcpy(regAtual->nomeEstacao, buffer, regAtual->tamNomeEstacao);
                break;
            case 2:
                regAtual->codLinha = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 3:
                //Passa o tamanho do nome da linha
                regAtual->tamNomeLinha = strlen(buffer);
                memcpy(regAtual->nomeLinha, buffer, regAtual->tamNomeLinha);
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
        //Preenche campos não presentes no CSV
        registro->removido = '0';
        registro->proximo = -1;

        //Vetor com tamanho fixo de 80 bytes do registro
        char bufferRegistro[80];
        memset(bufferRegistro, '$', 80); //Inicializa o buffer do registro com '$'

        int offset = 0;

        // Campos Fixos
        memcpy(bufferRegistro + offset, &registro->removido, sizeof(char));
        offset += sizeof(char); 

        memcpy(bufferRegistro + offset, &registro->proximo, sizeof(int));
        offset += sizeof(int);

        memcpy(bufferRegistro + offset, &registro->codEstacao, sizeof(int));
        offset += sizeof(int);

        memcpy(bufferRegistro + offset, &registro->codLinha, sizeof(int));
        offset += sizeof(int);

        memcpy(bufferRegistro + offset, &registro->codProxEstacao, sizeof(int));
        offset += sizeof(int);

        memcpy(bufferRegistro + offset, &registro->distProxEstacao, sizeof(int));
        offset += sizeof(int);

        memcpy(bufferRegistro + offset, &registro->codLinhaIntegra, sizeof(int));
        offset += sizeof(int);

        memcpy(bufferRegistro + offset, &registro->codEstIntegra, sizeof(int));
        offset += sizeof(int);

        // Campos Variáveis (Contíguos)
        memcpy(bufferRegistro + offset, &registro->tamNomeEstacao, sizeof(int));
        offset += sizeof(int);
        if (registro->tamNomeEstacao > 0){
            memcpy(bufferRegistro + offset, registro->nomeEstacao, registro->tamNomeEstacao);
            // Avançar o tamanho da string
            offset += registro->tamNomeEstacao; 
        }

        memcpy(bufferRegistro + offset, &registro->tamNomeLinha, sizeof(int));
        offset += sizeof(int);
        if (registro->tamNomeLinha > 0){
            memcpy(bufferRegistro + offset, registro->nomeLinha, registro->tamNomeLinha);
            // Avançar o tamanho da string
            offset += registro->tamNomeLinha;
        }

        // Escreve os 80 bytes exatos no arquivo (os '$' que sobraram já estão no final)
        fwrite(bufferRegistro, sizeof(char), 80, saida);
        cabecalho->proxRRN++;
        cabecalho->nroEstacoes++;
    }

    void printRegistros(Registro *registro){
        //verifica se o registro foi removido logicamente.
        if(registro->removido == '0'){
            //Imprime o campo de código da estação
            if (registro->codEstacao == -1)
                printf("NULO ");
            else 
                printf("%d ", registro->codEstacao);

            //Imprime o campo de nome da estação ou "NULO"
            if (registro->tamNomeEstacao == 0)
                printf("NULO ");
            else
                printf("%s ", registro->nomeEstacao);

             //Imprime o campo de código da linha
            if (registro->codLinha == -1)
                printf("NULO ");
            else
                printf("%d ", registro->codLinha);

            //Imprime o campo de nome da linha ou "NULO"
            if (registro->tamNomeLinha == 0)
                printf("NULO ");
            else
                printf("%s ", registro->nomeLinha);

            //Imprime o campo de código da próxima estação
            if (registro->codProxEstacao == -1)
                printf("NULO ");
            else
                printf("%d ", registro->codProxEstacao);
            
            //Imprime o campo de distância para a próxima estação
            if (registro->distProxEstacao == -1)
                printf("NULO ");
            else
                printf("%d ", registro->distProxEstacao);
            
            //Imprime o campo de código da linha de integração
            if (registro->codLinhaIntegra == -1)
                printf("NULO ");
            else
                printf("%d ", registro->codLinhaIntegra);
            
            //Imprime o campo de código da estação de integração
            if (registro->codEstIntegra == -1)
                printf("NULO\n");
            else
                printf("%d\n", registro->codEstIntegra);
        }
    }

    void preencherCriteriosBusca(CriteriosBusca *criterios, char *campo, char *conteudo){
        // Seta as flags e os valores de busca dependendo do campo passado
        if (strcmp(campo, "codEstacao") == 0) {
            criterios->flag_codEstacao = 1;
            criterios->regBusca.codEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
        } else if (strcmp(campo, "nomeEstacao") == 0) {
            criterios->flag_nomeEstacao = 1;
            strcpy(criterios->regBusca.nomeEstacao, conteudo);
        } else if (strcmp(campo, "codLinha") == 0) {
            criterios->flag_codLinha = 1;
            criterios->regBusca.codLinha = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
        } else if (strcmp(campo, "nomeLinha") == 0) {
            criterios->flag_nomeLinha = 1;
            strcpy(criterios->regBusca.nomeLinha, conteudo);
        } else if (strcmp(campo, "codProxEstacao") == 0) {
            criterios->flag_codProxEstacao = 1;
            criterios->regBusca.codProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
        } else if (strcmp(campo, "distProxEstacao") == 0) {
            criterios->flag_distProxEstacao = 1;
            criterios->regBusca.distProxEstacao = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
        } else if (strcmp(campo, "codLinhaIntegra") == 0) {
            criterios->flag_codLinhaIntegra = 1;
            criterios->regBusca.codLinhaIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
        } else if (strcmp(campo, "codEstIntegra") == 0) {
            criterios->flag_codEstIntegra = 1;
            criterios->regBusca.codEstIntegra = (strcmp(conteudo, "NULO") == 0) ? -1 : atoi(conteudo);
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
        
        // Verifica se o arquivo CSV existe 
        if (entrada == NULL) {
            printf("Falha no processamento do arquivo.\n");
            return;
        }

        FILE* saida = fopen(outputFileName, "wb+");
        if (saida == NULL) {
            printf("Falha no processamento do arquivo.\n");
            fclose(entrada);
            return;
        }

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
        memset(&regAtual, 0, sizeof(Registro)); //Inicializa o registro atual com zeros
        
        //Preenche os campos variáveis com '$' para evitar lixo não determinístico 
        memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
        memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));

        //Pula a primeira linha do arquivo de entrada (cabeçalho)
        while(fread(&c, sizeof(char), 1, entrada) == 1 && c != '\n');

        //Loop de leitura dos campos do arquivo de entrada (caractere a caractere)
        while(fread(&c, sizeof(char), 1, entrada) == 1){
            if (c == '\r') continue;
            if(c == ',' || c == '\n'){ //Verifica a condição de final de um campo ou de um registro
                buffer[posBuffer] = '\0'; //Sinaliza o final da string no buffer

                //Função que preenche o campo correspondente no registro atual com base no índice do campo
                writeCampos(buffer, fieldIndex, &regAtual);
                posBuffer = 0; //Sinaliza que o buffer está vazio para a leitura do próximo campo
                
                //Se o registro foi completamente preenchido (quebra de linha), escreve no binário. Se não, preenche o próximo campo
                if (c == '\n') {
                    writeRegistros(&regAtual, saida, &cabecalho);
                    fieldIndex = 0; //Os campos foram todos preenchidos, o próximo campo será o primeiro do próximo registro

                     //Limpar o registro atual para a leitura do próximo registro
                    memset(&regAtual, 0, sizeof(Registro));
                    //Preencher campos variáveis com '$' para evitar lixo 
                    memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
                    memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));
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

        if (fieldIndex > 0 || posBuffer > 0) {
            buffer[posBuffer] = '\0';
            writeCampos(buffer, fieldIndex, &regAtual);
            writeRegistros(&regAtual, saida, &cabecalho);
        }

        cabecalho.status = '1'; //Sinaliza que o arquivo foi criado corretamente
        fseek(saida, 0, SEEK_SET); //Posiciona o ponteiro do arquivo no início para atualizar o cabeçalho
        writeHeader(&cabecalho, saida); //Atualiza os valores do cabeçalho no arquivo de saída
        fclose(entrada);
        fclose(saida);
        BinarioNaTela(outputFileName);
    }

    int readRegistros(Registro *registro, FILE* file) {
        char buffer[80];
        
        // Tenta ler o bloco exato de 80 bytes do arquivo
        // Se não conseguir ler 80 bytes, chegou no fim do arquivo
        if (fread(buffer, sizeof(char), 80, file) != 80) {
            return 0; 
        }

        int offset = 0;

        // Lendo campos fixos (avançando o offset exato do tamanho de cada variável)
        memcpy(&registro->removido, buffer + offset, sizeof(char));
        offset += sizeof(char);

        memcpy(&registro->proximo, buffer + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&registro->codEstacao, buffer + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&registro->codLinha, buffer + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&registro->codProxEstacao, buffer + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&registro->distProxEstacao, buffer + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&registro->codLinhaIntegra, buffer + offset, sizeof(int));
        offset += sizeof(int);

        memcpy(&registro->codEstIntegra, buffer + offset, sizeof(int));
        offset += sizeof(int);

        // Lendo o tamanho do nome da estação
        memcpy(&registro->tamNomeEstacao, buffer + offset, sizeof(int));
        offset += sizeof(int);

        // Se a string existir, lê exatamente o tamanho dela
        if (registro->tamNomeEstacao > 0) {
            memcpy(registro->nomeEstacao, buffer + offset, registro->tamNomeEstacao);
            registro->nomeEstacao[registro->tamNomeEstacao] = '\0'; // Finaliza a string pro printf funcionar
            offset += registro->tamNomeEstacao;
        } else {
            registro->nomeEstacao[0] = '\0';
        }

        // Lendo o tamanho do nome da linha
        memcpy(&registro->tamNomeLinha, buffer + offset, sizeof(int));
        offset += sizeof(int);

        // Se a string existir, lê exatamente o tamanho dela
        if (registro->tamNomeLinha > 0) {
            memcpy(registro->nomeLinha, buffer + offset, registro->tamNomeLinha);
            registro->nomeLinha[registro->tamNomeLinha] = '\0'; // Finaliza a string pro printf funcionar
        } else {
            registro->nomeLinha[0] = '\0';
        }

        return 1;
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

        while(1) {
            memset(&regAtual, 0, sizeof(Registro));
            //Inicializar campos variáveis com '$'
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

        Header cabecalho;
        readHeader(&cabecalho, file);
        
        // Confere se o arquivo nao esta corrompido
        if (cabecalho.status == '0') {
            printf("Falha no processamento do arquivo.\n");
            fclose(file);
            return;
        }

        for(int i = 0; i < nroBuscas; i++){
            int nroCampos;
            scanf("%d", &nroCampos);
            
            CriteriosBusca criterios; 
            memset(&criterios, 0, sizeof(CriteriosBusca));

            // Leitura de cada par campo/valor
            for (int j = 0; j < nroCampos; j++){
                char campo[20];
                char conteudo[21];
                scanf("%s", campo);
                ScanQuoteString(conteudo);
                
                // Trata o caso do NULO ou string vazia para campos numericos pra nao quebrar no atoi
                if (strcmp(conteudo, "NULO") == 0 || strlen(conteudo) == 0) {
                    if (strcmp(campo, "codEstacao") == 0 || strcmp(campo, "codLinha") == 0 ||
                        strcmp(campo, "codProxEstacao") == 0 || strcmp(campo, "distProxEstacao") == 0 ||
                        strcmp(campo, "codLinhaIntegra") == 0 || strcmp(campo, "codEstIntegra") == 0) {
                        strcpy(conteudo, "-1");
                    }
                }

                preencherCriteriosBusca(&criterios, campo, conteudo);
            }

            // Quebra de linha entre as diferentes buscas
            if (i > 0) {
                printf("\n");
            }

            // Reseta o ponteiro pra comecar a ler os registros
            rewind(file);
            fseek(file, 17, SEEK_SET);

            Registro regAtual;
            int registrosEncontrados = 0; 
            
            while(1) {
                // Limpa o struct pra evitar lixo de memoria da iteracao anterior
                memset(&regAtual, 0, sizeof(Registro));
                
                if (!readRegistros(&regAtual, file))
                    break;
                    
                // Se nao ta removido logicamente e atende aos criterios, imprime
                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)){
                    printRegistros(&regAtual);
                    registrosEncontrados++;
                }
            }
            
            // Tratamento pra quando a busca nao retorna nada
            if (registrosEncontrados == 0) {
                printf("Registro inexistente.\n");
            }
        }
        
        // Quebra de linha final exigida na formatacao
        printf("\n");
        
        fclose(file);
    }

    void Delete(char *FileName, int nroRemocoes) {
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

        // Marca como inconsistente durante a execucao 
        cabecalho.status = '0';
        writeHeader(&cabecalho, arquivoBinario);

        for (int i = 0; i < nroRemocoes; i++) {
            int m;
            scanf("%d", &m);
            
            CriteriosBusca criterios;
            // Zerando as flags manualmente para garantir que lixo de memoria nao interfira
            criterios.flag_codEstacao = 0;
            criterios.flag_nomeEstacao = 0;
            criterios.flag_codLinha = 0;
            criterios.flag_nomeLinha = 0;
            criterios.flag_codProxEstacao = 0;
            criterios.flag_distProxEstacao = 0;
            criterios.flag_codLinhaIntegra = 0;
            criterios.flag_codEstIntegra = 0;

            for (int j = 0; j < m; j++) {
                char nomeCampo[50];
                char valorCampo[100];
                scanf("%s", nomeCampo);
                ScanQuoteString(valorCampo); 
                preencherCriteriosBusca(&criterios, nomeCampo, valorCampo);
            }

            Registro regAtual;
            int rrn = 0;
            
            // Pula o cabecalho de 17 bytes para comecar a ler os registros
            fseek(arquivoBinario, 17, SEEK_SET);

            while (readRegistros(&regAtual, arquivoBinario)) {
                // Se nao esta removido e bate com a busca 
                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    
                    // Atualiza flags de remocao 
                    regAtual.removido = '1';
                    regAtual.proximo = cabecalho.topo; // aponta pro antigo topo da pilha 
                    cabecalho.topo = rrn; // atualiza o topo do cabecalho com o RRN atual 

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

        // Retorna status para consistente e salva cabecalho 
        cabecalho.status = '1';
        writeHeader(&cabecalho, arquivoBinario);
        fclose(arquivoBinario);
    }

    void Insert(char *FileName, int nroInsercoes) {
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

        // Marca como inconsistente
        cabecalho.status = '0';
        writeHeader(&cabecalho, arquivoBinario);

        for (int i = 0; i < nroInsercoes; i++) {
            Registro novoReg;
            char bufferAux[100];

            // Lendo os campos na ordem especificada
            scanf("%d", &novoReg.codEstacao);

            scanf("%s", bufferAux);
            ScanQuoteString(bufferAux);
            novoReg.tamNomeEstacao = strlen(bufferAux);
            strcpy(novoReg.nomeEstacao, bufferAux);

            scanf("%s", bufferAux);
            if (strcmp(bufferAux, "NULO") == 0) {
                novoReg.codLinha = -1;
            } else {
                novoReg.codLinha = atoi(bufferAux);
            }

            scanf("%s", bufferAux);
            if (strcmp(bufferAux, "NULO") == 0) {
                novoReg.tamNomeLinha = 0;
            } else {
                ScanQuoteString(bufferAux);
                novoReg.tamNomeLinha = strlen(bufferAux);
                strcpy(novoReg.nomeLinha, bufferAux);
            }

            // Leitura dos ultimos 4 campos inteiros
            int* camposInt[] = {&novoReg.codProxEstacao, &novoReg.distProxEstacao, &novoReg.codLinhaIntegra, &novoReg.codEstIntegra};
            for(int k = 0; k < 4; k++) {
                scanf("%s", bufferAux);
                if (strcmp(bufferAux, "NULO") == 0) {
                    *camposInt[k] = -1;
                } else {
                    *camposInt[k] = atoi(bufferAux);
                }
            }

            // Setando campos de controle para o novo registro
            novoReg.removido = '0';
            novoReg.proximo = -1;

            // Lógica de reaproveitamento de espaço 
            long posicaoEscrita;

            if (cabecalho.topo == -1) {
                // Escreve no final do arquivo
                posicaoEscrita = 17 + (cabecalho.proxRRN * 80);
                cabecalho.proxRRN++;
            } else {
                // Reaproveita o espaco do topo da pilha
                int rrnReaproveitado = cabecalho.topo;
                posicaoEscrita = 17 + (rrnReaproveitado * 80);
                
                // Le o proximo da pilha antes de sobrescrever
                fseek(arquivoBinario, posicaoEscrita + 1, SEEK_SET); // Pula 1 byte do 'removido'
                int proximoDaPilha;
                fread(&proximoDaPilha, sizeof(int), 1, arquivoBinario);
                
                cabecalho.topo = proximoDaPilha; // Atualiza o topo do cabecalho
            }

            fseek(arquivoBinario, posicaoEscrita, SEEK_SET);

            // Escreve campo a campo garantindo a ordem 
            fwrite(&novoReg.removido, sizeof(char), 1, arquivoBinario);
            fwrite(&novoReg.proximo, sizeof(int), 1, arquivoBinario);
            fwrite(&novoReg.codEstacao, sizeof(int), 1, arquivoBinario);
            fwrite(&novoReg.codLinha, sizeof(int), 1, arquivoBinario);
            fwrite(&novoReg.codProxEstacao, sizeof(int), 1, arquivoBinario);
            fwrite(&novoReg.distProxEstacao, sizeof(int), 1, arquivoBinario);
            fwrite(&novoReg.codLinhaIntegra, sizeof(int), 1, arquivoBinario);
            fwrite(&novoReg.codEstIntegra, sizeof(int), 1, arquivoBinario);
            
            fwrite(&novoReg.tamNomeEstacao, sizeof(int), 1, arquivoBinario);
            if (novoReg.tamNomeEstacao > 0) {
                fwrite(novoReg.nomeEstacao, sizeof(char), novoReg.tamNomeEstacao, arquivoBinario);
            }
            
            fwrite(&novoReg.tamNomeLinha, sizeof(int), 1, arquivoBinario);
            if (novoReg.tamNomeLinha > 0) {
                fwrite(novoReg.nomeLinha, sizeof(char), novoReg.tamNomeLinha, arquivoBinario);
            }

            // Calcula lixo e preenche com '$' 
            int bytesEscritos = 1 + 4 + (6 * 4) + 4 + novoReg.tamNomeEstacao + 4 + novoReg.tamNomeLinha;
            int bytesSobrando = 80 - bytesEscritos;
            char lixo = '$';
            for (int k = 0; k < bytesSobrando; k++) {
                fwrite(&lixo, sizeof(char), 1, arquivoBinario);
            }
        }

        cabecalho.status = '1';
        writeHeader(&cabecalho, arquivoBinario);
        fclose(arquivoBinario);
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
        // Diz ser inconsistente durante a execucao
        cabecalho.status = '0';
        writeHeader(&cabecalho, arquivoBinario);

        for (int i = 0; i < nroAtualizacoes; i++) {
            int m;
            scanf("%d", &m);
            CriteriosBusca criterios;
            
            // Zera as flags de busca
            criterios.flag_codEstacao = 0;
            criterios.flag_nomeEstacao = 0;
            criterios.flag_codLinha = 0;
            criterios.flag_nomeLinha = 0;
            criterios.flag_codProxEstacao = 0;
            criterios.flag_distProxEstacao = 0;
            criterios.flag_codLinhaIntegra = 0;
            criterios.flag_codEstIntegra = 0;

            // Le as condicoes de busca
            for (int j = 0; j < m; j++) {
                char nomeCampo[50];
                char valorCampo[100];
                scanf("%s", nomeCampo);
                scanf("%s", valorCampo);
                ScanQuoteString(valorCampo);
                preencherCriteriosBusca(&criterios, nomeCampo, valorCampo);
            }

            int p;
            scanf("%d", &p);
            char camposUpdate[10][50];
            char valoresUpdate[10][100];
            
            // Le os campos a serem atualizados
            for (int k = 0; k < p; k++) {
                scanf("%s", camposUpdate[k]);
                scanf("%s", valoresUpdate[k]);
                ScanQuoteString(valoresUpdate[k]);
            }

            Registro regAtual;
            int rrn = 0;
            fseek(arquivoBinario, 17, SEEK_SET);

            // Busca sequencial no arquivo
            while (readRegistros(&regAtual, arquivoBinario)) {
                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    
                    // Aplica atualizacoes no registro em memoria
                    for (int k = 0; k < p; k++) {
                        if (strcmp(camposUpdate[k], "codEstacao") == 0) {
                            regAtual.codEstacao = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
                        } else if (strcmp(camposUpdate[k], "codLinha") == 0) {
                            regAtual.codLinha = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
                        } else if (strcmp(camposUpdate[k], "codProxEstacao") == 0) {
                            regAtual.codProxEstacao = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
                        } else if (strcmp(camposUpdate[k], "distProxEstacao") == 0) {
                            regAtual.distProxEstacao = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
                        } else if (strcmp(camposUpdate[k], "codLinhaIntegra") == 0) {
                            regAtual.codLinhaIntegra = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
                        } else if (strcmp(camposUpdate[k], "codEstIntegra") == 0) {
                            regAtual.codEstIntegra = (strcmp(valoresUpdate[k], "NULO") == 0) ? -1 : atoi(valoresUpdate[k]);
                        } else if (strcmp(camposUpdate[k], "nomeEstacao") == 0) {
                            if (strcmp(valoresUpdate[k], "NULO") == 0) {
                                regAtual.tamNomeEstacao = 0;
                            } else {
                                strcpy(regAtual.nomeEstacao, valoresUpdate[k]);
                                regAtual.tamNomeEstacao = strlen(valoresUpdate[k]);
                            }
                        } else if (strcmp(camposUpdate[k], "nomeLinha") == 0) {
                            if (strcmp(valoresUpdate[k], "NULO") == 0) {
                                regAtual.tamNomeLinha = 0;
                            } else {
                                strcpy(regAtual.nomeLinha, valoresUpdate[k]);
                                regAtual.tamNomeLinha = strlen(valoresUpdate[k]);
                            }
                        }
                    }

                    // Volta para o inicio do registro para sobrescrever
                    long posicaoRegistro = 17 + (rrn * 80);
                    fseek(arquivoBinario, posicaoRegistro, SEEK_SET);

                    // Escreve os campos atualizados no disco
                    fwrite(&regAtual.removido, sizeof(char), 1, arquivoBinario);
                    fwrite(&regAtual.proximo, sizeof(int), 1, arquivoBinario);
                    fwrite(&regAtual.codEstacao, sizeof(int), 1, arquivoBinario);
                    fwrite(&regAtual.codLinha, sizeof(int), 1, arquivoBinario);
                    fwrite(&regAtual.codProxEstacao, sizeof(int), 1, arquivoBinario);
                    fwrite(&regAtual.distProxEstacao, sizeof(int), 1, arquivoBinario);
                    fwrite(&regAtual.codLinhaIntegra, sizeof(int), 1, arquivoBinario);
                    fwrite(&regAtual.codEstIntegra, sizeof(int), 1, arquivoBinario);
                    
                    fwrite(&regAtual.tamNomeEstacao, sizeof(int), 1, arquivoBinario);
                    if (regAtual.tamNomeEstacao > 0) {
                        fwrite(regAtual.nomeEstacao, sizeof(char), regAtual.tamNomeEstacao, arquivoBinario);
                    }
                    
                    fwrite(&regAtual.tamNomeLinha, sizeof(int), 1, arquivoBinario);
                    if (regAtual.tamNomeLinha > 0) {
                        fwrite(regAtual.nomeLinha, sizeof(char), regAtual.tamNomeLinha, arquivoBinario);
                    }

                    // Preenche o lixo restante com '$'
                    int bytesEscritos = 1 + 4 + (6 * 4) + 4 + regAtual.tamNomeEstacao + 4 + regAtual.tamNomeLinha;
                    int bytesSobrando = 80 - bytesEscritos;
                    char lixo = '$';
                    for (int k = 0; k < bytesSobrando; k++) {
                        fwrite(&lixo, sizeof(char), 1, arquivoBinario);
                    }

                    // Pula para o final deste registro para continuar o while
                    fseek(arquivoBinario, posicaoRegistro + 80, SEEK_SET);
                }
                rrn++;
            }
            // Rewind para a proxima iteracao de atualizacao
            fseek(arquivoBinario, 17, SEEK_SET);
        }

        cabecalho.status = '1';
        writeHeader(&cabecalho, arquivoBinario);
        fclose(arquivoBinario);
        
        BinarioNaTela(FileName);
    }
