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
        // Se o registro foi lido com sucesso, insere na árvore de índice
        if (statusLeitura == 1){
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

/*
 * Insere novos registros no arquivo de dados e adiciona as chaves
 * correspondentes no índice árvore-B.
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
