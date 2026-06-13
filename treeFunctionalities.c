#include "treeFunctionalities.h"
#include "register.h"
#include "header.h"

/*
 * Percorre o arquivo de dados e insere a chave (codEstacao) de cada
 * registro não removido na árvore-B, um a um.
 */
void createIndex(char *binFileName, char *indexFileName) {
    // ABERTURA E CONTROLE DE FLUXOS
    // O arquivo de dados é aberto de forma não-destrutiva ("rb").
    FILE *arquivoBinario = fopen(binFileName, "rb");
    if (arquivoBinario == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // INICIALIZAÇÃO DA ÁRVORE-B
    // Instancia o cabeçalho do índice na RAM. A função createBinaryHeader deve garantir
    // que status = '0' (inconsistente), noRaiz = -1 e proxRRN = 0.
    binaryHeader header;
    createBinaryHeader(&header);

    // O arquivo de índice é criado em modo de escrita binária ("wb+").
    FILE *arquivoIndice = fopen(indexFileName, "wb+");
    if (arquivoIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // Persiste o cabeçalho no estado inconsistente ocupando os primeiros 17 bytes do disco
    writeBinaryHeader(&header, arquivoIndice);

    Registro regAtual;
    int rrn = 0; // Rastreador lógico da posição do registro no arquivo de dados
    
    // PREPARAÇÃO DO CURSOR DE LEITURA
    // Pula os 17 bytes do cabeçalho do arquivo de dados para alinhar o cursor no RRN 0
    fseek(arquivoBinario, 17, SEEK_SET);

    // VARREDURA E INDEXAÇÃO
    while (1) {
        // A função readRegistros implementa o salto de bytes e retorna códigos de estado
        int statusLeitura = readRegistros(&regAtual, arquivoBinario);
        
        if (statusLeitura == 0)
            break; // EOF (Fim de arquivo de dados)
            
        // FILTRO PARA IGNORAR REMOVIDOS
        // A especificação proíbe terminantemente a inserção de chaves referentes a 
        // registros marcados como logicamente removidos no arquivo de índice.
        if (statusLeitura == 1 && regAtual.removido == '0'){
            
            // CÁLCULO DO BYTE OFFSET
            // Converte a posição lógica (RRN) para a posição física no disco.
            // 17 bytes (cabeçalho) + (RRN * 80 bytes por registro).
            int byteOffset = 17 + (rrn * 80);
            
            // Dispara o algoritmo de inserção na Árvore-B passando a chave (codEstacao) 
            // e o ponteiro de dados (byteOffset). O cabeçalho é passado por referência 
            // para controle do proxRRN e noRaiz durante eventuais Splits.
            insertKey(arquivoIndice, byteOffset, regAtual.codEstacao, &header);
        }
        
        // INCREMENTO DE SINCRONIZAÇÃO
        // O RRN lógico avança independentemente do registro ser ativo ou removido,
        // garantindo que o cálculo matemático do byteOffset permaneça exato.
        rrn++;
    }

    // CONSOLIDAÇÃO DO ARQUIVO DE ÍNDICE
    // Retorna o status para '1' (consistente), indicando que a árvore-B foi 
    // completamente formada e o programa não sofreu interrupções.
    header.status = '1';
    writeBinaryHeader(&header, arquivoIndice);
    
    fclose(arquivoBinario);
    fclose(arquivoIndice);
    
    // Acionamento exigido pela plataforma de testes
    BinarioNaTela(indexFileName);
}

/*
 * Busca registros no arquivo de dados usando critérios definidos pelo usuário.
 * Quando o critério for codEstacao, usa o índice árvore-B para ir direto
 * ao registro. Para outros campos, faz varredura sequencial.
 */
void searchWithIndex(char *binFileName, char *indexFileName, int nroBuscas) {
    // ABERTURA E CONTROLE DE FLUXOS
    // Ambos os arquivos são abertos em modo restrito de leitura binária ("rb").
    // Previne qualquer alteração acidental durante as buscas.
    FILE *arquivoBinario = fopen(binFileName, "rb");
    if (arquivoBinario == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arquivoIndice = fopen(indexFileName, "rb");
    if (arquivoIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // EXTRAÇÃO E VALIDAÇÃO DOS CAMPOS DO CABEÇALHO
    // A especificação exige que a operação aborte caso qualquer um dos arquivos 
    // tenha sofrido corrupção estrutural (status == '0').
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    binaryHeader headerIndice;
    readBinaryHeader(&headerIndice, arquivoIndice);
    if (headerIndice.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    // PROCESSAMENTO DE MÚLTIPLAS BUSCAS
    for (int i = 0; i < nroBuscas; i++) {
        int nroCampos;
        scanf("%d", &nroCampos);

        // Prevenção de lixo de memória na struct de flags
        CriteriosBusca criterios = {0};
        lerCriteriosUsuario(&criterios, nroCampos);

        int registrosEncontrados = 0;
        Registro regAtual;

        // ALTERNATIVAS DE BUSCA
        
        if (criterios.flag_codEstacao == 1) {
            // BUSCA INDEXADA
            // Como a chave primária foi requisitada, aciona a Árvore-B. Qualquer busca
            // que utilize este campo deve obrigatoriamente ser feita com o auxílio do índice.
            int byteOffset = searchKey(arquivoIndice, criterios.regBusca.codEstacao, &headerIndice);

            // A chave existe na árvore
            if (byteOffset != -1) {
                // Acesso Direto: Posiciona o cursor do disco de dados exatamente
                // no offset físico retornado pelo índice.
                fseek(arquivoBinario, byteOffset, SEEK_SET);
                readRegistros(&regAtual, arquivoBinario);

                // CHECAGEM DOS CRITÉRIOS
                // Mesmo encontrando via índice, é necessário aplicar o checagemCriteriosBusca
                // caso o usuário tenha fornecido múltiplos filtros (ex: WHERE codEstacao = 1 AND nomeLinha = "Azul").
                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    printRegistros(&regAtual);
                    registrosEncontrados++;
                }
            }
        } else {
            // VARREDURA SEQUENCIAL
            // Para buscas que não utilizam a chave primária, a especificação exige 
            // seguir a lógica da funcionalidade [3].
            
            // Pula os 17 bytes do cabeçalho de dados para o reinício da varredura
            fseek(arquivoBinario, 17, SEEK_SET);

            while (1) {
                // Lê fisicamente até o EOF
                if (!readRegistros(&regAtual, arquivoBinario))
                    break;
                    
                // O motor lógico interno garante a validação da flag removido == '0' 
                // junto com os demais critérios de busca.
                if (checagemCriteriosBusca(&criterios, &regAtual)) {
                    printRegistros(&regAtual);
                    registrosEncontrados++;
                }
            }
        }

        // OUTPUT DE AUSÊNCIA DE DADOS ---
        // Exibido unicamente se a busca (seja via Índice ou Varredura) retornar vazia.
        if (!registrosEncontrados)
            printf("Registro inexistente.\n");

        printf("\n"); // Separação visual exigida entre buscas sequenciais
    }

    // ENCERRAMENTO SEGURO
    fclose(arquivoBinario);
    fclose(arquivoIndice);
}

/*
 * Remove logicamente registros do arquivo de dados que atendem aos
 * critérios de busca, e remove as chaves correspondentes do índice.
 */
void deleteWithIndex(char *binFileName, char *indexFileName, int nroRemocoes) {
    FILE *arquivoBinario = fopen(binFileName, "rb+");
    if (arquivoBinario == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arquivoIndice = fopen(indexFileName, "rb+");
    if (arquivoIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    binaryHeader headerIndice;
    readBinaryHeader(&headerIndice, arquivoIndice);
    if (headerIndice.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    // marca ambos os arquivos como inconsistentes durante a operação
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);
    headerIndice.status = '0';
    writeBinaryHeader(&headerIndice, arquivoIndice);

    for (int i = 0; i < nroRemocoes; i++) {
        int m;
        scanf("%d", &m);

        CriteriosBusca criterios = {0};
        lerCriteriosUsuario(&criterios, m);

        Registro regAtual;
        int rrn = 0;
        fseek(arquivoBinario, 17, SEEK_SET);

        while (readRegistros(&regAtual, arquivoBinario)) {
            if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                // remove a chave do índice antes de marcar o registro
                removeKey(arquivoIndice, regAtual.codEstacao, &headerIndice);

                // marca o registro como removido no arquivo de dados
                regAtual.removido = '1';
                regAtual.proximo  = cabecalho.topo;
                cabecalho.topo    = rrn;

                fseek(arquivoBinario, -80, SEEK_CUR);
                fwrite(&regAtual.removido, sizeof(char), 1, arquivoBinario);
                fwrite(&regAtual.proximo, sizeof(int), 1, arquivoBinario);
                fseek(arquivoBinario, 75, SEEK_CUR);
            }
            rrn++;
        }
        fseek(arquivoBinario, 17, SEEK_SET);
    }

    cabecalho.status = '1';
    writeHeader(&cabecalho, arquivoBinario);
    headerIndice.status = '1';
    writeBinaryHeader(&headerIndice, arquivoIndice);

    fclose(arquivoBinario);
    fclose(arquivoIndice);

    BinarioNaTela(binFileName);
    BinarioNaTela(indexFileName);
}


void insertWithIndex(char *binFileName, char *indexFileName, int nroInsercoes) {
    // ABERTURA E CONTROLE DE REGISTROS DUPLOS
    // Ambos os arquivos precisam ser abertos em "rb+" para permitir a leitura 
    // das estruturas de navegação (pilha e nós da árvore) e a escrita do novo registro.
    FILE *arquivoBinario = fopen(binFileName, "rb+");
    if (arquivoBinario == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *arquivoIndice = fopen(indexFileName, "rb+");
    if (arquivoIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // EXTRAÇÃO E VALIDAÇÃO DE CAMPOS DO CABEÇALHO
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    binaryHeader headerIndice;
    readBinaryHeader(&headerIndice, arquivoIndice);
    if (headerIndice.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    // INCONSISTÊNCIA DE SEGURANÇA
    // Trava simultaneamente os dois arquivos. Como a inserção agora é um processo
    // de duas vias (grava no dado E no índice), uma queda de energia no meio do processo 
    // invalidará as duas estruturas para leituras futuras.
    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);
    headerIndice.status = '0';
    writeBinaryHeader(&headerIndice, arquivoIndice);

    // PROCESSAMENTO DE MÚLTIPLAS INSERÇÕES
    for (int i = 0; i < nroInsercoes; i++) {
        Registro novoReg;
        
        // Função que implementa a extração dos 8 campos da entrada padrão e converte nulos.
        preencherNovoRegistro(&novoReg);
        
        // TRAVA DE CHAVE PRIMÁRIA (ÍNDICE)
        // Antes de tentar inserir no disco de dados, consulta a Árvore-B
        // para garantir que a chave 'codEstacao' não está duplicada.
        int offsetExistente = searchKey(arquivoIndice, novoReg.codEstacao, &headerIndice);
        if (offsetExistente != -1) {
            continue; // Aborta esta inserção se a chave já existe
        }

        // VALIDAÇÃO DE CAMPOS DO CABEÇALHO
        // Para manter o 'nroEstacoes' do cabeçalho correto, é necessário verificar 
        // se o nome da estação que está sendo inserida é inédito no arquivo de dados.
        int nomeInedito = 0;
        if (novoReg.tamNomeEstacao > 0) {
            nomeInedito = 1; // Hipótese inicial
            
            // Salva o estado do cursor para não perder a posição de inserção futura
            long posAtual = ftell(arquivoBinario);
            fseek(arquivoBinario, 17, SEEK_SET); 
            
            Registro regBusca;
            int statusLeitura;
            
            // Varredura sequencial no arquivo para validação de nome de estação inédito
            while ((statusLeitura = readRegistros(&regBusca, arquivoBinario)) != 0) {
                if (regBusca.tamNomeEstacao == novoReg.tamNomeEstacao) {
                    if (strncmp(regBusca.nomeEstacao, novoReg.nomeEstacao, novoReg.tamNomeEstacao) == 0) {
                        nomeInedito = 0; // Correspondência encontrada
                        break;
                    }
                }
            }
            fseek(arquivoBinario, posAtual, SEEK_SET); // Restaura o cursor para a alteração
        }

        // GERENCIAMENTO DA PILHA DE REMOVIDOS (REAPROVEITAMENTO DE ESPAÇO)
        long posicaoEscrita;
        int rrnNovoRegistro;

        if (cabecalho.topo == -1) {
            // Caso Base: Arquivo sem espaços disponíveis. Insere no final.
            rrnNovoRegistro = cabecalho.proxRRN;
            posicaoEscrita  = 17 + (rrnNovoRegistro * 80);
            cabecalho.proxRRN++;
        } else {
            // Reaproveitamento: Sobrescreve o nó no topo da pilha de removidos.
            rrnNovoRegistro = cabecalho.topo;
            posicaoEscrita  = 17 + (rrnNovoRegistro * 80);

            // Desempilha atualizando o cabeçalho com o RRN do próximo nó livre
            fseek(arquivoBinario, posicaoEscrita + 1, SEEK_SET);
            int proximoDaPilha;
            fread(&proximoDaPilha, sizeof(int), 1, arquivoBinario);
            cabecalho.topo = proximoDaPilha;
        }

        // DUPLA PERSISTÊNCIA
        
        // Arquivo de Dados: Grava o registro de 80 bytes na posição calculada
        fseek(arquivoBinario, posicaoEscrita, SEEK_SET);
        writeRegistros(&novoReg, arquivoBinario, &cabecalho);
        
        // B. Arquivo de Índice: Insere a Chave e o Byte Offset na Árvore-B.
        insertKey(arquivoIndice, posicaoEscrita, novoReg.codEstacao, &headerIndice);

        // ATUALIZAÇÃO SEGURA DO CABEÇALHO
        // O incremento do campo que contabiliza pares de estações únicos ocorre 
        // apenas se a dupla persistência foi concluída com sucesso.
        if (novoReg.codProxEstacao != -1) {
            cabecalho.nroParesEstacao++;
        }
        
        if (nomeInedito == 1) {
            cabecalho.nroEstacoes++;
        }
    }

    // CONSOLIDAÇÃO DOS ARQUIVOS
    // Retorna os dois arquivos para o estado consistente e sobrescreve as configurações atualizadas.
    cabecalho.status = '1';
    writeHeader(&cabecalho, arquivoBinario);
    
    headerIndice.status = '1';
    writeBinaryHeader(&headerIndice, arquivoIndice);

    fclose(arquivoBinario);
    fclose(arquivoIndice);

    BinarioNaTela(binFileName);
    BinarioNaTela(indexFileName);
}