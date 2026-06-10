#include "treeFunctionalities.h"
#include "register.h"
#include "header.h"

void createIndex(char *binFileName, char *indexFileName){
    // Percorre o arquivo de dados registro a registro, indexando os registros não removidos pelo campo de codEstacao e escrevendo as chaves e os RRNs correspondentes no arquivo de índice
    FILE *arquivoBinario = fopen(binFileName, "rb");
    if (arquivoBinario == NULL){
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // Cria um cabeçalho para o arquivo de índice e escreve no arquivo
    binaryHeader header;
    createBinaryHeader(&header);
    FILE *arquivoIndice = fopen(indexFileName, "wb+");
    if (arquivoIndice == NULL){
        printf("Falha no processamento do arquivo.\n");
        fclose(arquivoBinario);
        return;
    }
    writeBinaryHeader(&header, arquivoIndice);
    
    // Loop para ler os registros do arquivo binário e escrever as chaves e RRNs correspondentes no arquivo de índice
    Registro regAtual;
    int rrn = 0;
    fseek(arquivoBinario, 17, SEEK_SET); // Pula o cabeçalho
    while (1){
        int statusLeitura = readRegistros(&regAtual, arquivoBinario);
        if (statusLeitura == 0)
            break; // Fim do arquivo
        // Se o registro não estiver removido, insere o nó correspondente na árvore de índice
        if (statusLeitura == 1 && regAtual.removido == '0')
            insertKey(arquivoIndice, rrn, regAtual.codEstacao, &header);
        // Escreve a chave (codEstacao) e o RRN correspondente no arquivo de índice
        rrn++;
    }
    // Atualiza o cabeçalho do arquivo de índice com o número de nós inseridos e marca como consistente
    header.status = '1';
    writeBinaryHeader(&header, arquivoIndice);
    fclose(arquivoBinario);
    fclose(arquivoIndice);
    BinarioNaTela(indexFileName);
}