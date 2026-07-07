#include "functionalities.h"

// VARIÁVEL DE ESTADO PARA O QSORT
char g_campo_ordenacao[50];

// Implementação da função que realiza a conversão do CSV para binário (Funcionalidade 1)
void CreateTable(char *inputFileName, char *outputFileName){
    // Abertura estrita exigida: modo de leitura textual para o CSV e 
    // modo binário ("wb+") para o arquivo de saída.
    FILE *entrada = fopen(inputFileName, "r");
    FILE *saida = fopen(outputFileName, "wb+");

    if (entrada == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // INICIALIZAÇÃO DO CABEÇALHO
    // Cria o cabeçalho no estado inconsistente (status = '0') e reserva 
    // os primeiros 17 bytes no disco. O cabeçalho será reescrito ao final.
    Header cabecalho;
    newHeader(&cabecalho);
    writeHeader(&cabecalho, saida);

    // VARIÁVEIS PARA EXECUÇÃO DO PARSER DO CSV
    char buffer[256];
    int posBuffer = 0;
    char c;
    int fieldIndex = 0; // Mapeia a coluna atual do CSV para a struct Registro
    
    Registro regAtual;
    Estacoes nomesEPares = {0}; // Estrutura auxiliar em RAM para contabilizar nomes e pares de estações únicos

    // Salto do cabeçalho CSV: percorre a primeira linha até o '\n'
    while(fread(&c, sizeof(char), 1, entrada) == 1 && c != '\n');

    // LEITURA BYTE A BYTE
    // Varredura contínua, com atenção para lidar com colunas vazias consecutivas.
    while(fread(&c, sizeof(char), 1, entrada) == 1){
        if (c == '\r') // Ignora o carriage return do padrão Windows (CRLF)
            continue;
        
        // Fim de um campo (',') ou fim de um registro inteiro ('\n')
        if (c == ',' || c == '\n'){
            buffer[posBuffer] = '\0'; // Sinaliza o final da string no buffer

            // Delega o preenchimento da struct com base no índice da coluna
            writeCampos(buffer, fieldIndex, &regAtual);
            posBuffer = 0; // Reset lógico do buffer para a próxima extração

            if (c == '\n'){
                // PERSISTÊNCIA E PREVENÇÃO DE LIXO
                // Verifica a exclusividade do nome e par da estação antes de gravar no disco
                isEstacaoUnica(&nomesEPares, regAtual.nomeEstacao);
                isParUnico(&nomesEPares, regAtual.codEstacao, regAtual.codProxEstacao);

                // Grava os exatos 80 bytes do registro estruturado
                writeRegistros(&regAtual, saida, &cabecalho);
                cabecalho.proxRRN++;
                fieldIndex = 0; // Reinicia a contagem de colunas para a próxima linha

                // Limpeza e Padding: Zera a struct e injeta obrigatoriamente
                // o caractere '$' nos campos de tamanho variável para evitar lixo de memória.
                memset(&regAtual, 0, sizeof(Registro));
                memset(regAtual.nomeEstacao, '$', sizeof(regAtual.nomeEstacao));
                memset(regAtual.nomeLinha, '$', sizeof(regAtual.nomeLinha));
            }
            else
                fieldIndex++; /// Avança o mapeamento para o próximo campo da linha
        }
        else{
            // Acúmulo de estado: constrói o valor do campo atual
            buffer[posBuffer] = c;
            posBuffer++; // Incrementa o índice do buffer para a próxima posição
        }
    }

    // TRATAMENTO DE EOF (FIM DE ARQUIVO) ABRUPTO
    // Garante que o último registro seja persistido caso o CSV termine 
    // sem uma quebra de linha ('\n') no final da última string.
    if (fieldIndex > 0 || posBuffer > 0){
        buffer[posBuffer] = '\0';
        writeCampos(buffer, fieldIndex, &regAtual);
        isEstacaoUnica(&nomesEPares, regAtual.nomeEstacao);
        isParUnico(&nomesEPares, regAtual.codEstacao, regAtual.codProxEstacao);
        writeRegistros(&regAtual, saida, &cabecalho);
        cabecalho.proxRRN++;
    }

    // CONSOLIDAÇÃO DO ARQUIVO BINÁRIO
    // Escreve os valores dos campos acumulados na RAM no cabeçalho.
    cabecalho.nroEstacoes = nomesEPares.numEstacoes;
    cabecalho.nroParesEstacao = nomesEPares.numParesEstacao;

    // Altera o status para consistente e retrocede o cursor para o byte 0.
    cabecalho.status = '1';
    fseek(saida, 0, SEEK_SET);
    writeHeader(&cabecalho, saida);

    fclose(entrada);
    fclose(saida);
    BinarioNaTela(outputFileName);
}

// Implementação da função que imprime os registros do arquivo binário
void Select(char *FileName){
    // ABERTURA DO ARQUIVO
    // Modo "rb" (read binary) obrigatório, pois a operação é estritamente de leitura.
    FILE *file = fopen(FileName, "rb");
    
    // Trava de segurança para evitar SegFault caso o arquivo não exista no diretório.
    if (file == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // ACESSO AO CABEÇALHO
    // Extrai os primeiros 17 bytes para avaliar o status e campos no cabeçalho do arquivo.
    Header cabecalho;
    readHeader(&cabecalho, file);

    // VERIFICAÇÃO DE INTEGRIDADE
    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return;
    }

    // VERIFICAÇÃO DE CONTEÚDO
    if (cabecalho.nroEstacoes == 0){
        printf("Registro inexistente.\n");
        fclose(file);
        return;
    }

    Registro regAtual;
    int registrosEncontrados = 0; // Rastreador de busca

    // VARREDURA SEQUENCIAL
    // Percorre o disco registro a registro de forma linear, do início ao fim.
    while (1){
        // A função readRegistros consome exatos 80 bytes por iteração.
        int statusLeitura = readRegistros(&regAtual, file);
        
        if (statusLeitura == 0)
            break; // EOF (Fim de arquivo) alcançado
            
        if (statusLeitura == 2)
            continue; // Registro marcado como removido ('1'). Salto de 79 bytes já foi feito no parser.

        // IMPRESSÃO E CONTABILIZAÇÃO
        // O registro é válido e ativo. Chama função de formatação de saída e do tratamento de nulos.
        printRegistros(&regAtual);
        registrosEncontrados++;
    }
    
    // VALIDAÇÃO FINAL E ENCERRAMENTO
    // Resultado para o caso extremo onde o arquivo possui registros físicos, 
    // mas absolutamente todos estão logicamente removidos.
    if (!registrosEncontrados)
        printf("Registro inexistente.\n");
        
    fclose(file);
}

// Implementação da função que imprime registros com base em um conjunto de critérios
void Where(char *FileName, int nroBuscas){
    // ABERTURA DO ARQUIVO
    // Modo de leitura estrita ("rb"). Os dados não são alterados nessa funcionalidade.
    FILE *file = fopen(FileName, "rb");

    if (file == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // ACESSO E VALIDAÇÃO DO CABEÇALHO
    // Extrai os 17 bytes do cabeçalho para garantir que o arquivo não está corrompido.
    Header cabecalho;
    readHeader(&cabecalho, file);
    
    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(file);
        return;
    }

    int nroCampos; 

    // PROCESSAMENTO DAS MÚLTIPLAS BUSCAS
    // Executa o bloco de pesquisa para a quantidade exata de buscas exigida pela entrada.
    for (int i = 0; i < nroBuscas; i++){
        
        // LEITURA DE CRITÉRIOS DE BUSCA
        // Obtém a quantidade de filtros lógicos que serão aplicados a esta busca
        scanf("%d", &nroCampos);

        CriteriosBusca criterios = {0}; 
        lerCriteriosUsuario(&criterios, nroCampos); // Centraliza e processa os dados do input

        // PREPARAÇÃO DO CURSOR
        // Posiciona o ponteiro do arquivo exatamente no byte 17 (início do primeiro registro de dados).
        // Isso é obrigatório para garantir que buscas subsequentes leiam o arquivo desde o começo, 
        // evitando a sobrecarga de sistema causada por múltiplos fclose/fopen.
        fseek(file, 17, SEEK_SET);

        Registro regAtual;
        int registrosEncontrados = 0; 

        // VARREDURA SEQUENCIAL
        // Navega fisicamente pelos registros até atingir o fim do arquivo (EOF).
        while (1){
            // A leitura realiza o salto de registros logicamente removidos e consome os 80 bytes.
            if (!readRegistros(&regAtual, file))
                break;

            // AVALIAÇÃO DE CONDIÇÕES E IMPRESSÃO
            // A função auxiliar aplica o conjunto de critérios (múltiplas combinações de colunas)
            if (checagemCriteriosBusca(&criterios, &regAtual)){
                printRegistros(&regAtual);
                registrosEncontrados++;
                
                // OTIMIZAÇÃO DE BUSCA
                // Se o critério contém 'codEstacao' (chave primária, garantidamente única 
                // pela Funcionalidade 1), a varredura é abortada assim que a primeira e única 
                // correspondência é encontrada, economizando tempo de I/O em disco.
                if (criterios.flag_codEstacao == 1)
                    break;
            }
        }

        // RESULTADO DE AUSÊNCIA
        // Exibido unicamente se a varredura alcançou o EOF sem que o contador sofresse incremento.
        if (!registrosEncontrados)
            printf("Registro inexistente.\n");
        
        printf("\n");
    }
    
    fclose(file);
}

// Implementação da função que marca registros que atendam a um conjunto de critérios como removidos.
void Delete(char *FileName, int nroRemocoes){
    // ABERTURA DO ARQUIVO
    // Modo "rb+" obrigatório: permite percorrer o arquivo lendo registros e, simultaneamente, 
    // reescrever bytes específicos no meio do arquivo
    FILE *arquivoBinario = fopen(FileName, "rb+");
    if (arquivoBinario == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // EXTRAÇÃO E VALIDAÇÃO DO CABEÇALHO
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);

    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // INCONSISTÊNCIA DE SEGURANÇA
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);

    // PROCESSAMENTO DE MÚLTIPLAS DELEÇÕES
    // Loop que controla quantas instruções de exclusão independentes foram requisitadas.
    for (int i = 0; i < nroRemocoes; i++){
        int m;
        scanf("%d", &m);

        // Prevenção de lixo
        CriteriosBusca criterios = {0}; 
        lerCriteriosUsuario(&criterios, m); // Realiza o parse da entrada padrão

        Registro regAtual;
        int rrn = 0;

        // PREPARAÇÃO DO CURSOR
        // Avança 17 bytes para pular o cabeçalho e alinhar o ponteiro do disco no RRN 0.
        fseek(arquivoBinario, 17, SEEK_SET);

        // VARREDURA DE LOCALIZAÇÃO E MUTABILIDADE
        while (readRegistros(&regAtual, arquivoBinario)){
            // Garante que um registro já removido não sofra nova remoção,
            // o que causaria ciclos infinitos ou perda de encadeamento na pilha de reaproveitamento.
            if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)){
                
                // GERENCIAMENTO DA PILHA DE REMOVIDOS
                regAtual.removido = '1';
                regAtual.proximo = cabecalho.topo; 
                cabecalho.topo = rrn;              
                
                // ATUALIZAÇÃO DA LOCALIZAÇÃO DO CURSOR
                // A função readRegistros consome 80 bytes. O cursor do disco parou no início 
                // do próximo registro. O fseek negativo recua 80 bytes, posicionado o cursor 
                // no byte 0 do registro recém-analisado para permitir a sobrescrita.
                fseek(arquivoBinario, -80, SEEK_CUR);

                // Escreve estritamente os 5 bytes dos campos do cabeçalho referentes à pilha (1 de char + 4 de inteiro)
                fwrite(&regAtual.removido, sizeof(char), 1, arquivoBinario);
                fwrite(&regAtual.proximo, sizeof(int), 1, arquivoBinario);

                // RESTAURAÇÃO DO CURSOR PARA LEITURA
                // Como 5 bytes já foram percorridos, avança 75 bytes diretos sem ler dados desnecessários,
                // alinhando o cursor no byte 0 do próximo registro lógico.
                fseek(arquivoBinario, 75, SEEK_CUR);
            }
            rrn++; // Mantém o sincronismo entre a posição lógica e a física
        }
    }

    // CONSOLIDAÇÃO E VALIDAÇÃO
    // Restaura o status indicando que todas as atualizações e encadeamentos da pilha 
    // ocorreram com sucesso. O cabeçalho é reescrito no offset 0 contendo o novo 'topo'.
    cabecalho.status = '1';
    writeHeader(&cabecalho, arquivoBinario);
    
    fclose(arquivoBinario);
    
    BinarioNaTela(FileName);
}

// Implementação da função que insere novos registros, reaproveitando a pilha de registros logicamente removidos
void Insert(char *FileName, int nroInsercoes){
    // ABERTURA DO ARQUIVO
    // Modo "rb+" exigido para permitir tanto a escrita no fim do arquivo quanto a 
    // sobrescrita em espaços reaproveitados no meio do arquivo.
    FILE *arquivoBinario = fopen(FileName, "rb+");
    if (arquivoBinario == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // EXTRAÇÃO E VALIDAÇÃO DO CABEÇALHO
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);

    if (cabecalho.status == '0'){
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // INCONSISTÊNCIA DE SEGURANÇA
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);

    // PROCESSAMENTO DE MÚLTIPLAS INSERÇÕES
    for (int i = 0; i < nroInsercoes; i++){
        Registro novoReg;
        
        // Função responsável pela leitura e a formatação da entrada padrão
        preencherNovoRegistro(&novoReg);
    
        // CÁLCULO DE OFFSET E GERENCIAMENTO DA PILHA
        long posicaoEscrita;

        if (cabecalho.topo == -1){
            // CASO 1: PILHA VAZIA (Sem registros removidos)
            // A inserção ocorre no final lógico do arquivo. 
            // Cálculo: 17 bytes do cabeçalho + (RRN atual * 80 bytes por registro).
            posicaoEscrita = 17 + (cabecalho.proxRRN * 80);
            cabecalho.proxRRN++;
        }
        else{
            // CASO 2: REAPROVEITAMENTO DE ESPAÇO
            // Existe pelo menos um espaço logicamente removido disponível.
            int rrnReaproveitado = cabecalho.topo;
            posicaoEscrita = 17 + (rrnReaproveitado * 80);

            // ATUALIZAÇÃO DO ENCADEAMENTO DA PILHA
            // Posiciona o cursor fisicamente no RRN a ser reaproveitado, pulando 1 byte 
            // (que corresponde ao campo char 'removido') para acessar diretamente 
            // o inteiro 'proximo', que contém o RRN do próximo espaço livre.
            fseek(arquivoBinario, posicaoEscrita + 1, SEEK_SET); 
            
            int proximoDaPilha;
            fread(&proximoDaPilha, sizeof(int), 1, arquivoBinario);

            // O cabeçalho herda o ponteiro, desempilhando o nó atual
            cabecalho.topo = proximoDaPilha; 
        }
        
        // POSICIONAMENTO FINAL E PERSISTÊNCIA
        // Crava o cursor no offset exato
        fseek(arquivoBinario, posicaoEscrita, SEEK_SET);

        // A função writeRegistros implementa a injeção de lixo ('$') e garante a escrita 
        // estrita dos 80 bytes do novo registro por cima do espaço alocado.
        writeRegistros(&novoReg, arquivoBinario, &cabecalho);
    }

    // CONSOLIDAÇÃO E VALIDAÇÃO
    // Operações de disco concluídas. O status retorna para consistente e o cabeçalho 
    // é persistido com os novos valores de 'proxRRN' e/ou 'topo'.
    cabecalho.status = '1';
    writeHeader(&cabecalho, arquivoBinario);
    
    fclose(arquivoBinario);
    
    BinarioNaTela(FileName);
}

// Implementação da função que atualiza os campos de registros determinados por critérios de busca fornecidos pelo usuário
void Update(char *FileName, int nroAtualizacoes){
    // ABERTURA DO ARQUIVO
    // Modo "rb+" estritamente necessário para percorrer o disco em busca dos registros
    // e sobrescrevê-los fisicamente nas mesmas posições originais.
    FILE *arquivoBinario = fopen(FileName, "rb+");
    if (arquivoBinario == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // EXTRAÇÃO DO CABEÇALHO E VERIFICAÇÃO DE INTEGRIDADE
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);

    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // INCONSISTÊNCIA DE SEGURANÇA
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);

    // PROCESSAMENTO DE MÚLTIPLAS ATUALIZAÇÕES
    for (int i = 0; i < nroAtualizacoes; i++) {
        
        // LEITURA DOS CRITÉRIOS DE BUSCA
        int m;
        scanf("%d", &m);
        CriteriosBusca criterios = {0}; // Zera as flags para evitar lixo
        lerCriteriosUsuario(&criterios, m);

        // LEITURA DOS CAMPOS A SEREM ATUALIZADOS
        int p;
        scanf("%d", &p);
        
        // Matrizes para armazenar o par (Chave-Valor) das atualizações solicitadas
        char camposUpdate[10][50];
        char valoresUpdate[10][100];
        
        for (int k = 0; k < p; k++) {
            scanf("%s", camposUpdate[k]);
            
            // TRATAMENTO DE STRINGS (Aspas Duplas)
            // Se o campo for variável, aciona o ScanQuoteString para processar as 
            // aspas duplas, englobando valores compostos (ex: "Parada Inglesa").
            if (strcmp(camposUpdate[k], "nomeEstacao") == 0 || strcmp(camposUpdate[k], "nomeLinha") == 0)
                ScanQuoteString(valoresUpdate[k]);
            else
                scanf("%s", valoresUpdate[k]);
        }

        Registro regAtual;
        int rrn = 0;

        // ALINHAMENTO DO CURSOR PARA VARREDURA
        // Retorna ao primeiro byte do registro lógico inicial, pulando os 17 bytes de cabeçalho
        fseek(arquivoBinario, 17, SEEK_SET);

        // BUSCA SEQUENCIAL E ALTERAÇÕES
        while (readRegistros(&regAtual, arquivoBinario)) {
            
            // Identifica registros que satisfazem os critérios da cláusula WHERE
            if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                
                // ALTERAÇÃO DOS CAMPOS DO REGISTRO
                // Substitui os campos antigos pelos novos armazenados nas matrizes de update
                updateRegistro(&regAtual, camposUpdate, valoresUpdate, p);
                
                // CÁLCULO E REPOSICIONAMENTO DO CURSOR FÍSICO
                // Como readRegistros avança 80 bytes, calcula o offset absoluto de onde esse
                // registro deveria estar e reposiciona o cursor antes de realizar a gravação.
                long posicaoRegistro = 17 + (rrn * 80);
                fseek(arquivoBinario, posicaoRegistro, SEEK_SET);
                
                // PERSISTÊNCIA E PADRONIZAÇÃO DE LIXO
                // Grava os 80 bytes modificados. A função writeRegistros garante a injeção
                // de caracteres '$' nos espaços vazios deixados por possíveis mudanças no tamanho de strings.
                writeRegistros(&regAtual, arquivoBinario, &cabecalho);

                // REPOSICIONAMENTO PARA LEITURA
                // Restaura o cursor para o final do registro modificado, permitindo
                // que o laço while(readRegistros...) continue a leitura perfeitamente.
                fseek(arquivoBinario, posicaoRegistro + 80, SEEK_SET);
            }
            rrn++; // Mantém sincronia entre os registros lidos e a posição física
        }
        
        // RESET PARA PRÓXIMA ALTERAÇÃO
        // Volta o cursor para o início dos dados para a próxima rodada de atualizações.
        fseek(arquivoBinario, 17, SEEK_SET);
    }

    // CONSOLIDAÇÃO E VALIDAÇÃO FINAL
    cabecalho.status = '1';
    writeHeader(&cabecalho, arquivoBinario);
    fclose(arquivoBinario);
    
    BinarioNaTela(FileName);
}


/**
 * Função comparadora para o Quicksort. Retorna < 0 se a < b, > 0 se a > b, e 0 se forem iguais.
 * Garante que registros com valores nulos sejam considerados "maiores" que qualquer valor válido, 
 * forçando-os para o final da ordenação.
 */
int compareRegistros(const void *a, const void *b) {
    Registro *r1 = (Registro *)a;
    Registro *r2 = (Registro *)b;

    // AVALIAÇÃO DE CAMPOS NUMÉRICOS
    if (strcmp(g_campo_ordenacao, "codEstacao") == 0) {
        if (r1->codEstacao == -1 && r2->codEstacao != -1) return 1;  // r1 nulo vai pro fim
        if (r2->codEstacao == -1 && r1->codEstacao != -1) return -1; // r2 nulo vai pro fim
        return r1->codEstacao - r2->codEstacao;
    }
        
    else if (strcmp(g_campo_ordenacao, "codLinha") == 0) {
        if (r1->codLinha == -1 && r2->codLinha != -1) return 1;
        if (r2->codLinha == -1 && r1->codLinha != -1) return -1;
        return r1->codLinha - r2->codLinha;
    }
        
    else if (strcmp(g_campo_ordenacao, "codProxEstacao") == 0) {
        if (r1->codProxEstacao == -1 && r2->codProxEstacao != -1) return 1;
        if (r2->codProxEstacao == -1 && r1->codProxEstacao != -1) return -1;
        return r1->codProxEstacao - r2->codProxEstacao;
    }
        
    else if (strcmp(g_campo_ordenacao, "distProxEstacao") == 0) {
        if (r1->distProxEstacao == -1 && r2->distProxEstacao != -1) return 1;
        if (r2->distProxEstacao == -1 && r1->distProxEstacao != -1) return -1;
        return r1->distProxEstacao - r2->distProxEstacao;
    }
        
    else if (strcmp(g_campo_ordenacao, "codLinhaIntegra") == 0) {
        if (r1->codLinhaIntegra == -1 && r2->codLinhaIntegra != -1) return 1;
        if (r2->codLinhaIntegra == -1 && r1->codLinhaIntegra != -1) return -1;
        return r1->codLinhaIntegra - r2->codLinhaIntegra;
    }
        
    else if (strcmp(g_campo_ordenacao, "codEstIntegra") == 0) {
        if (r1->codEstIntegra == -1 && r2->codEstIntegra != -1) return 1;
        if (r2->codEstIntegra == -1 && r1->codEstIntegra != -1) return -1;
        return r1->codEstIntegra - r2->codEstIntegra;
    }

    // AVALIAÇÃO DE CAMPOS DE STRINGS
    // Se o tamanho for nulo então é considerado nulo
    else if (strcmp(g_campo_ordenacao, "nomeEstacao") == 0) {
        if (r1->tamNomeEstacao == 0 && r2->tamNomeEstacao > 0) return 1;
        if (r2->tamNomeEstacao == 0 && r1->tamNomeEstacao > 0) return -1;
        if (r1->tamNomeEstacao == 0 && r2->tamNomeEstacao == 0) return 0;
        return strcmp(r1->nomeEstacao, r2->nomeEstacao);
    }
        
    else if (strcmp(g_campo_ordenacao, "nomeLinha") == 0) {
        if (r1->tamNomeLinha == 0 && r2->tamNomeLinha > 0) return 1;
        if (r2->tamNomeLinha == 0 && r1->tamNomeLinha > 0) return -1;
        if (r1->tamNomeLinha == 0 && r2->tamNomeLinha == 0) return 0;
        return strcmp(r1->nomeLinha, r2->nomeLinha);
    }

    return 0;
}


void sortBinary(char *inputFileName, char *campoOrdenacao, char *outputFileName) {
    // ABERTURA E VERIFICAÇÃO DO ARQUIVO DE ORIGEM
    FILE *arquivoEntrada = fopen(inputFileName, "rb");
    if (arquivoEntrada == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    Header cabecalhoAntigo;
    readHeader(&cabecalhoAntigo, arquivoEntrada);
    if (cabecalhoAntigo.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoEntrada);
        return;
    }

    int capacidade = 50;
    int nroRegistrosAtivos = 0;
    Registro *vetorRAM = malloc(capacidade * sizeof(Registro));
    if (vetorRAM == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoEntrada);
        return;
    }

    fseek(arquivoEntrada, 17, SEEK_SET);
    Registro regAtual;

    // LEITURA SEQUENCIAL COM FILTRO DE REMOÇÃO
    while (readRegistros(&regAtual, arquivoEntrada)) {
        // Ignora qualquer registro removido logicamente
        if (regAtual.removido == '0') {
            
            // Aumenta capacidade caso necessário
            if (nroRegistrosAtivos == capacidade) {
                capacidade *= 2;
                vetorRAM = realloc(vetorRAM, capacidade * sizeof(Registro));
            }
            
            // Copia o registro lido do disco para o bloco contíguo na RAM
            vetorRAM[nroRegistrosAtivos] = regAtual;
            nroRegistrosAtivos++;
        }
    }
    fclose(arquivoEntrada);

    // Passa o campo utilizado como critério de ordenação para a variável global
    strcpy(g_campo_ordenacao, campoOrdenacao);
    
    // Chamada do QuickSort da biblioteca padrão
    qsort(vetorRAM, nroRegistrosAtivos, sizeof(Registro), compareRegistros);

    // 5. CRIAÇÃO E ABERTURA DO ARQUIVO DE DESTINO
    FILE *arquivoSaida = fopen(outputFileName, "wb+");
    if (arquivoSaida == NULL) {
        printf("Falha no processamento do arquivo.\n");
        free(vetorRAM);
        return;
    }

    // ESCRITA DO CABEÇALHO DO NOVO ARQUIVO
    Header cabecalhoNovo = cabecalhoAntigo;
    cabecalhoNovo.status = '0'; // Marca como inconsistente durante a gravação
    
    // Como o novo arquivo ignorou os deletados, a pilha é reiniciada
    cabecalhoNovo.topo = -1; 
    cabecalhoNovo.proxRRN = nroRegistrosAtivos; 
    
    writeHeader(&cabecalhoNovo, arquivoSaida);

    // O ponteiro proximo de todos os registros é igual a -1,
    // pois neste novo arquivo nenhum deles está na pilha de removidos.
    for (int i = 0; i < nroRegistrosAtivos; i++) {
        vetorRAM[i].proximo = -1;
        writeRegistros(&vetorRAM[i], arquivoSaida, &cabecalhoNovo);
    }

    cabecalhoNovo.status = '1';
    writeHeader(&cabecalhoNovo, arquivoSaida);

    free(vetorRAM);
    fclose(arquivoSaida);
    BinarioNaTela(outputFileName);
}