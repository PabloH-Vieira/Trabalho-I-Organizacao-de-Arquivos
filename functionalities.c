#include "functionalities.h"

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
    //memset(&regAtual, 0, sizeof(Registro));  // Inicializa o registro atual com zeros
    // Preenche os campos variáveis com '$'
    //memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
    //memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));

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

    // Verifica se há dados pendentes no buffer ou campo incompleto no fim do arquivo
    // por exemplo quando o último campo é ...
    if (fieldIndex > 0 || posBuffer > 0){
        // Sinaliza o final da string no buffer para o último campo
        buffer[posBuffer] = '\0';
        writeCampos(buffer, fieldIndex, &regAtual);
        // Verificar se a estação atual é única
        isEstacaoUnica(&nomesEPares, regAtual.nomeEstacao);
        // Verificar se o par de estações atual é único
        isParUnico(&nomesEPares, regAtual.codEstacao, regAtual.codProxEstacao);
        // Escreve o último registro no arquivo binário de saída
        writeRegistros(&regAtual, saida, &cabecalho);
        // Incrementa o próximo RRN disponível no cabeçalho para o próximo registro a ser escrito
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

    Registro regAtual;
    int registrosEncontrados = 0; // Variável para contar o número de registros encontrados para imprimir a mensagem de "Registro inexistente" caso nenhum registro seja encontrado

    // Loop para ler os registros
    while (1){
        int statusLeitura = readRegistros(&regAtual, file);
        if (statusLeitura == 0)
            break; // Fim do arquivo
        if (statusLeitura == 2)
            continue; // Registro removido, pula para o próximo registro

        printRegistros(&regAtual);
        registrosEncontrados++;
    }
    if (!registrosEncontrados)
        printf("Registro inexistente.\n");
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

        CriteriosBusca criterios = {0}; 
        lerCriteriosUsuario(&criterios, nroCampos); // Reaproveitamento total de código!

        // Posicionar o ponteiro do arquivo no início dos registros para a execução de múltiplas buscas
        fseek(file, 17, SEEK_SET);

        // Acessar os registros e fazer as verificações para imprimir os registros que atendem às condições de busca
        Registro regAtual;
        int registrosEncontrados = 0; // Variável para contar o número de registros encontrados que atendem aos critérios de busca

        while (1){
            //memset(&regAtual, 0, sizeof(Registro));
            // Inicializar campos variáveis com '$' para evitar lixo
            //memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
            //memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));

            // Loop para preencher registros e verificar se está no final do arquivo
            if (!readRegistros(&regAtual, file))
                break;

            // Comparar registro atual com os critérios de busca
            if (checagemCriteriosBusca(&criterios, &regAtual)){
                printRegistros(&regAtual);
                registrosEncontrados++;
                
                // Se o critério de busca for o código da estação, que é um valor único, para a busca quando encontrar a correspondência
                if (criterios.flag_codEstacao == 1)
                    break;
            }
        }

        if (!registrosEncontrados)
            printf("Registro inexistente.\n");
        
        printf("\n");
    }
    fclose(file);
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
        lerCriteriosUsuario(&criterios, m); // Reaproveitamento total de código!

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
                
                // Volta o ponteiro do arquivo para o início do registro atual para escrever a marcação de remoção e o próximo da pilha
                fseek(arquivoBinario, -80, SEEK_CUR);

                // Escreve os 5 bytes modificados (1 do removido + 4 do proximo RRN)
                fwrite(&regAtual.removido, sizeof(char), 1, arquivoBinario);
                fwrite(&regAtual.proximo, sizeof(int), 1, arquivoBinario);

                // Pula os 75 bytes restantes para deixar o cursor pronto para o próximo registro
                fseek(arquivoBinario, 75, SEEK_CUR);
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

    // Ler o cabeçalho do arquivo e guardar na struct de cabeçalho
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
        //memset(&novoReg, 0, sizeof(Registro));
        // Inicializar campos variáveis com '$' para evitar lixo
        //memset(novoReg.nomeEstacao, '$', sizeof(novoReg.nomeEstacao));
        //memset(novoReg.nomeLinha, '$', sizeof(novoReg.nomeLinha));
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

        // Ler o cabeçalho do arquivo e guardar na struct de cabeçalho
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
            lerCriteriosUsuario(&criterios, m);

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