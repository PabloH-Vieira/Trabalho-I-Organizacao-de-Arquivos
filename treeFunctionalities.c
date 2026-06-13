#include "treeFunctionalities.h"
#include "register.h"
#include "header.h"

/*
 * Percorre o arquivo de dados e insere a chave (codEstacao) de cada
 * registro não removido na árvore-B, um a um.
 */
void createIndex(char *binFileName, char *indexFileName) {
    FILE *arquivoBinario = fopen(binFileName, "rb");
    if (arquivoBinario == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    binaryHeader header;
    createBinaryHeader(&header);

    FILE *arquivoIndice = fopen(indexFileName, "wb+");
    if (arquivoIndice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }

    // escreve o cabeçalho inicial antes de começar a inserir
    writeBinaryHeader(&header, arquivoIndice);

    Registro regAtual;
    int rrn = 0;
    fseek(arquivoBinario, 17, SEEK_SET);

    while (1) {
        int statusLeitura = readRegistros(&regAtual, arquivoBinario);
        if (statusLeitura == 0)
            break; // Fim do arquivo
        // Insere apenas registros válidos (não removidos) na árvore de índice
        if (statusLeitura == 1 && regAtual.removido == '0'){
            int byteOffset = 17 + (rrn * 80);
            insertKey(arquivoIndice, byteOffset, regAtual.codEstacao, &header);
        }
        // Incrementa o RRN para sincronizar com os registros no arquivo binário
        rrn++;
    }

    header.status = '1';
    writeBinaryHeader(&header, arquivoIndice);
    fclose(arquivoBinario);
    fclose(arquivoIndice);
    BinarioNaTela(indexFileName);
}

/*
 * Busca registros no arquivo de dados usando critérios definidos pelo usuário.
 * Quando o critério for codEstacao, usa o índice árvore-B para ir direto
 * ao registro. Para outros campos, faz varredura sequencial.
 */
void searchWithIndex(char *binFileName, char *indexFileName, int nroBuscas) {
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

    // verifica consistência do arquivo de dados
    Header cabecalho;
    readHeader(&cabecalho, arquivoBinario);
    if (cabecalho.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    // verifica consistência do índice
    binaryHeader headerIndice;
    readBinaryHeader(&headerIndice, arquivoIndice);
    if (headerIndice.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        fclose(arquivoIndice);
        return;
    }

    for (int i = 0; i < nroBuscas; i++) {
        int nroCampos;
        scanf("%d", &nroCampos);

        CriteriosBusca criterios = {0};
        lerCriteriosUsuario(&criterios, nroCampos);

        int registrosEncontrados = 0;
        Registro regAtual;

        if (criterios.flag_codEstacao == 1) {
            // busca pelo índice: vai direto ao RRN do registro
            int byteOffset = searchKey(arquivoIndice, criterios.regBusca.codEstacao, &headerIndice);

            if (byteOffset != -1) {
                fseek(arquivoBinario, byteOffset, SEEK_SET);
                readRegistros(&regAtual, arquivoBinario);

                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    printRegistros(&regAtual);
                    registrosEncontrados++;
                }
            }
        } else {
            // sem índice disponível — varredura sequencial igual ao Where
            fseek(arquivoBinario, 17, SEEK_SET);

            while (1) {
                if (!readRegistros(&regAtual, arquivoBinario))
                    break;
                if (checagemCriteriosBusca(&criterios, &regAtual)) {
                    printRegistros(&regAtual);
                    registrosEncontrados++;
                }
            }
        }

        if (!registrosEncontrados)
            printf("Registro inexistente.\n");

        printf("\n");
    }

    fclose(arquivoBinario);
    fclose(arquivoIndice);
}

/*
 * Remove logicamente registros do arquivo de dados que atendem aos
 * criterios de busca, e remove as chaves correspondentes do indice.
 * Utiliza o indice Arvore-B caso o criterio de busca envolva codEstacao.
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

    // marca ambos os arquivos como inconsistentes durante a operacao
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

        // se a busca for pela chave primaria (codEstacao), usa o indice para ir direto
        if (criterios.flag_codEstacao == 1) {
            int byteOffset = searchKey(arquivoIndice, criterios.regBusca.codEstacao, &headerIndice);

            if (byteOffset != -1) {
                fseek(arquivoBinario, byteOffset, SEEK_SET);
                readRegistros(&regAtual, arquivoBinario);

                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    removeKey(arquivoIndice, regAtual.codEstacao, &headerIndice);

                    // decrementa o nro de pares se a estacao tiver uma proxima ligada a ela
                    if (regAtual.codProxEstacao != -1) {
                        cabecalho.nroParesEstacao--;
                    }

                    // verifica se o nome da estacao vai sumir do arquivo pra decrementar no cabecalho
                    if (regAtual.tamNomeEstacao > 0) {
                        int contagem = 0;
                        long posAtual = ftell(arquivoBinario); // salva a posicao atual

                        fseek(arquivoBinario, 17, SEEK_SET); // pula o cabecalho pra iniciar a varredura
                        Registro regBusca;
                        
                        while (readRegistros(&regBusca, arquivoBinario)) {
                            if (regBusca.removido == '0' && regBusca.tamNomeEstacao == regAtual.tamNomeEstacao) {
                                if (strncmp(regBusca.nomeEstacao, regAtual.nomeEstacao, regAtual.tamNomeEstacao) == 0) {
                                    contagem++;
                                }
                            }
                        }

                        // se achar so 1, eh pq eh o proprio registro que ta sendo removido agora
                        if (contagem == 1) {
                            cabecalho.nroEstacoes--;
                        }

                        fseek(arquivoBinario, posAtual, SEEK_SET); // devolve o ponteiro
                    }

                    // calcula o RRN baseado no byteOffset (descontando o cabecalho de 17 bytes)
                    int rrn = (byteOffset - 17) / 80;

                    regAtual.removido = '1';
                    regAtual.proximo  = cabecalho.topo;
                    cabecalho.topo    = rrn;

                    fseek(arquivoBinario, byteOffset, SEEK_SET);
                    fwrite(&regAtual.removido, sizeof(char), 1, arquivoBinario);
                    fwrite(&regAtual.proximo, sizeof(int), 1, arquivoBinario);
                }
            }
        } else {
            // sem chave primaria na busca, faz varredura sequencial
            int rrn = 0;
            fseek(arquivoBinario, 17, SEEK_SET);

            while (readRegistros(&regAtual, arquivoBinario)) {
                if (regAtual.removido == '0' && checagemCriteriosBusca(&criterios, &regAtual)) {
                    removeKey(arquivoIndice, regAtual.codEstacao, &headerIndice);

                    // decrementa o nro de pares se a estacao tiver uma proxima ligada a ela
                    if (regAtual.codProxEstacao != -1) {
                        cabecalho.nroParesEstacao--;
                    }

                    // verifica se o nome da estacao vai sumir do arquivo pra decrementar no cabecalho
                    if (regAtual.tamNomeEstacao > 0) {
                        int contagem = 0;
                        long posAtual = ftell(arquivoBinario); // salva a posicao atual

                        fseek(arquivoBinario, 17, SEEK_SET); // pula o cabecalho pra iniciar a varredura
                        Registro regBusca;
                        
                        while (readRegistros(&regBusca, arquivoBinario)) {
                            if (regBusca.removido == '0' && regBusca.tamNomeEstacao == regAtual.tamNomeEstacao) {
                                if (strncmp(regBusca.nomeEstacao, regAtual.nomeEstacao, regAtual.tamNomeEstacao) == 0) {
                                    contagem++;
                                }
                            }
                        }

                        // se achar so 1, eh pq eh o proprio registro que ta sendo removido agora
                        if (contagem == 1) {
                            cabecalho.nroEstacoes--;
                        }

                        fseek(arquivoBinario, posAtual, SEEK_SET); // devolve o ponteiro
                    }

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
        }
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

/*
 * Insere novos registros no arquivo de dados e adiciona as chaves
 * correspondentes no índice árvore-B.
 */
/*
void insertWithIndex(char *binFileName, char *indexFileName, int nroInsercoes) {
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

    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);
    headerIndice.status = '0';
    writeBinaryHeader(&headerIndice, arquivoIndice);

    for (int i = 0; i < nroInsercoes; i++) {
        Registro novoReg;
        preencherNovoRegistro(&novoReg);

        long posicaoEscrita;
        int rrnNovoRegistro;

        if (cabecalho.topo == -1) {
            // sem espaço reaproveitável — escreve no fim
            rrnNovoRegistro = cabecalho.proxRRN;
            posicaoEscrita  = 17 + (rrnNovoRegistro * 80);
            cabecalho.proxRRN++;
        } else {
            // reaproveta o topo da pilha de removidos
            rrnNovoRegistro = cabecalho.topo;
            posicaoEscrita  = 17 + (rrnNovoRegistro * 80);

            fseek(arquivoBinario, posicaoEscrita + 1, SEEK_SET);
            int proximoDaPilha;
            fread(&proximoDaPilha, sizeof(int), 1, arquivoBinario);
            cabecalho.topo = proximoDaPilha;
        }

        fseek(arquivoBinario, posicaoEscrita, SEEK_SET);
        writeRegistros(&novoReg, arquivoBinario, &cabecalho);

        // insere a chave do novo registro no índice
        insertKey(arquivoIndice, posicaoEscrita, novoReg.codEstacao, &headerIndice);
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
    */
void insertWithIndex(char *binFileName, char *indexFileName, int nroInsercoes) {
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

    cabecalho.status = '0';
    writeHeader(&cabecalho, arquivoBinario);
    headerIndice.status = '0';
    writeBinaryHeader(&headerIndice, arquivoIndice);
    for (int i = 0; i < nroInsercoes; i++) {
        Registro novoReg;
        preencherNovoRegistro(&novoReg);
        
        // 1. ÁRVORE-B: Verifica se a chave primária já existe
        int offsetExistente = searchKey(arquivoIndice, novoReg.codEstacao, &headerIndice);
        if (offsetExistente != -1) {
            continue; 
        }

        int nomeInedito = 0;
        if (novoReg.tamNomeEstacao > 0) {
            nomeInedito = 1; // Assume que é inédito
            long posAtual = ftell(arquivoBinario);
            
            fseek(arquivoBinario, 17, SEEK_SET); // Pula o cabeçalho
            Registro regBusca;
            int statusLeitura;
            
            // Varre o arquivo buscando o nome
            while ((statusLeitura = readRegistros(&regBusca, arquivoBinario)) != 0) {
                if (regBusca.tamNomeEstacao == novoReg.tamNomeEstacao) {
                    if (strncmp(regBusca.nomeEstacao, novoReg.nomeEstacao, novoReg.tamNomeEstacao) == 0) {
                        nomeInedito = 0; // O nome já existia!
                        break;
                    }
                }
            }
            fseek(arquivoBinario, posAtual, SEEK_SET); // Restaura o ponteiro
        }

        long posicaoEscrita;
        int rrnNovoRegistro;

        if (cabecalho.topo == -1) {
            rrnNovoRegistro = cabecalho.proxRRN;
            posicaoEscrita  = 17 + (rrnNovoRegistro * 80);
            cabecalho.proxRRN++;
        } else {
            rrnNovoRegistro = cabecalho.topo;
            posicaoEscrita  = 17 + (rrnNovoRegistro * 80);

            fseek(arquivoBinario, posicaoEscrita + 1, SEEK_SET);
            int proximoDaPilha;
            fread(&proximoDaPilha, sizeof(int), 1, arquivoBinario);
            cabecalho.topo = proximoDaPilha;
        }

        // 4. ESCRITA FÍSICA E ÍNDICE
        fseek(arquivoBinario, posicaoEscrita, SEEK_SET);
        writeRegistros(&novoReg, arquivoBinario, &cabecalho);
        
        insertKey(arquivoIndice, posicaoEscrita, novoReg.codEstacao, &headerIndice);

        // 5. ATUALIZAÇÃO SEGURA DO CABEÇALHO
        if (novoReg.codProxEstacao != -1) {
            cabecalho.nroParesEstacao++;
        }
        
        // Agora sim, a variável nomeInedito traz a verdade!
        if (nomeInedito == 1) {
            cabecalho.nroEstacoes++;
        }
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