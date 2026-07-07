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
    int encontrou = 0;

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
                encontrou = 1;
                break; // codEstacao é chave primária, só existe um match
            }
        }
    }

    if (!encontrou)
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

/*
 * [14] Junção por ordenação-intercalação (Sort-Merge Join).
 * Ordena ambos os arquivos por suas respectivas chaves de junção e realiza
 * uma varredura sincronizada para juntar os registros que satisfaçam
 * a condição de igualdade entre os campos.
 */
void juncaoOrdenacao(char *arq1, char *campo1, char *arq2, char *campo2) {
    // Utiliza a funcionalidade [13] para criar arquivos temporários com os registros ordenados
    char tempArq1[] = "temp_sorted1.bin";
    char tempArq2[] = "temp_sorted2.bin";
    
    sortBinary(arq1, campo1, tempArq1); // Ordena arquivo 1
    sortBinary(arq2, campo2, tempArq2); // Ordena arquivo 2

    // Abertura dos arquivos temporários ordenados
    FILE *f1 = fopen(tempArq1, "rb");
    FILE *f2 = fopen(tempArq2, "rb");

    if (f1 == NULL || f2 == NULL) {
        printf("Falha no processamento do arquivo.\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }

    // Validação dos cabeçalhos
    Header h1, h2;
    readHeader(&h1, f1);
    readHeader(&h2, f2);

    if (h1.status == '0' || h2.status == '0') {
        printf("Falha no processamento do arquivo.\n");
        fclose(f1);
        fclose(f2);
        return;
    }

    Registro r1, r2;
    
    // Posiciona ponteiros após o cabeçalho para iniciar varredura
    fseek(f1, 17, SEEK_SET);
    fseek(f2, 17, SEEK_SET);

    int existeR1 = readRegistros(&r1, f1);
    int existeR2 = readRegistros(&r2, f2);
    int encontrou = 0;

    // Enquanto não atingir o final de nenhum dos arquivos
    while (existeR1 && existeR2) {
        // Ignora valores NULOS
        if (r1.codProxEstacao == -1){
            existeR1 = readRegistros(&r1, f1); 
            continue;
        }
        if (r2.codEstacao == -1){
            existeR2 = readRegistros(&r2, f2);
            continue;
        }

        // COMPARAÇÃO DAS CHAVES
        if (r1.codProxEstacao == r2.codEstacao) {
            // Match das chaves
            encontrou = 1;
            printRegistroJuncao(&r1, &r2);

            // Como a codEstacao (r2) é Chave Primária (única), múltiplas estações (r1) 
            // podem apontar para ela (Relação N:1). Apenas R1 avança.
            existeR1 = readRegistros(&r1, f1);
            
        } else if (r1.codProxEstacao < r2.codEstacao) {
            // Se a chave do R1 ficou para trás, avança R1 para tentar alcançar R2
            existeR1 = readRegistros(&r1, f1);
            
        } else {
            // Se a chave do R2 ficou para trás, avança R2 para tentar alcançar R1
            existeR2 = readRegistros(&r2, f2);
        }
    }

    if (!encontrou) {
        printf("Registro inexistente.\n");
    }

    fclose(f1);
    fclose(f2);

    // Apaga os arquivos temporários gravados no disco
    remove(tempArq1);
    remove(tempArq2);
}