#include "join.h"
#include "header.h"
#include "utils.h"
#include "functionalities.h"
#include <stdio.h>
#include <stdlib.h>

/*
 * [11] Junção por loop aninhado.
 * Para cada registro válido do arquivo 1, percorre todos os registros
 * válidos do arquivo 2 procurando o match codProxEstacao = codEstacao.
 */
void juncaoLoopAninhado(char *arq1, char *campo1, char *arq2, char *campo2) {
    FILE *f1 = fopen(arq1, "rb");
    if (f1 == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *f2 = fopen(arq2, "rb");
    if (f2 == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        return;
    }

    // verifica consistência dos dois arquivos
    Header cab1, cab2;
    readHeader(&cab1, f1);
    readHeader(&cab2, f2);

    if (cab1.status == '0' || cab2.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        return;
    }

    Registro r1, r2;
    int encontrouMatch = 0;

    // loop externo — percorre arquivo 1
    fseek(f1, 17, SEEK_SET);
    while (readRegistros(&r1, f1)) {
        if (r1.removido == '1' || r1.codProxEstacao == -1)
            continue;

        // loop interno — percorre arquivo 2 do início para cada r1
        fseek(f2, 17, SEEK_SET);
        while (readRegistros(&r2, f2)) {
            if (r2.removido == '1' || r2.codEstacao == -1)
                continue;

            if (r1.codProxEstacao == r2.codEstacao) {
                printRegistroJuncao(&r1, &r2);
                encontrouMatch = 1;
                break; // codEstacao é chave primária, só existe um match
            }
        }
    }

    if (!encontrouMatch)
        printf("Registro inexistente.\n");

    fclose(f1);
    fclose(f2);
}

/*
 * [12] Junção por loop único com índice árvore-B.
 * Para cada registro válido do arquivo 1, usa o índice do arquivo 2
 * para ir direto ao registro com codEstacao = codProxEstacao.
 */
void juncaoLoopUnico(char *arq1, char *campo1, char *arq2, char *campo2, char *indiceArq2) {
    FILE *f1 = fopen(arq1, "rb");
    if (f1 == NULL) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *f2 = fopen(arq2, "rb");
    if (f2 == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        return;
    }

    FILE *indice = fopen(indiceArq2, "rb");
    if (indice == NULL) {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        return;
    }

    // verifica consistência dos arquivos
    Header cab1, cab2;
    readHeader(&cab1, f1);
    readHeader(&cab2, f2);

    binaryHeader headerIndice;
    readBinaryHeader(&headerIndice, indice);

    if (cab1.status == '0' || cab2.status == '0' || headerIndice.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        fclose(indice);
        return;
    }

    Registro r1, r2;
    int encontrouMatch = 0;

    // loop único — percorre arquivo 1 e usa índice para buscar no arquivo 2
    fseek(f1, 17, SEEK_SET);
    while (readRegistros(&r1, f1)) {
        if (r1.removido == '1' || r1.codProxEstacao == -1)
            continue;

        // busca pelo índice: retorna o byte offset do registro no arquivo 2
        int byteOffset = searchKey(indice, r1.codProxEstacao, &headerIndice);

        if (byteOffset == -1)
            continue; // não existe correspondência no arquivo 2

        // vai direto ao registro no arquivo 2
        fseek(f2, byteOffset, SEEK_SET);
        if (!readRegistros(&r2, f2))
            continue;

        if (r2.removido == '1')
            continue;

        printRegistroJuncao(&r1, &r2);
        encontrouMatch = 1;
    }

    if (!encontrouMatch)
        printf("Registro inexistente.\n");

    fclose(f1);
    fclose(f2);
    fclose(indice);
}