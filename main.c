#include "functionalities.h"
#include "treeFunctionalities.h"
#include "register.h"
#include "header.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>

/*  TRABALHO 1 - INDEXAÇÃO DE REGISTROS EM ESTRUTURA DE ÁRVORE BINÁRIA A PARTIR DE UM ARQUIVO DE DADOS
    Guilherme Pego dos Santos - 15575570
    Pablo Henrique Almeida Vieira - 16895429
    NOME DA EQUIPE: G13
    Turma: Terça-Feira
    Link do vídeo: https://drive.google.com/file/d/1S1IE_zOj3QLgdCrpAMNY8VF0IukK4IWh/view?usp=sharing
*/

int main(void) {
    int funcionalidade = 0;
    scanf("%d", &funcionalidade);

    char nomeArquivoBin[100];
    char nomeArquivoIndice[100];

    switch (funcionalidade) {
        case 1:
            // Cria o arquivo binário a partir do CSV
            char nomeArquivoCSV[100];
            scanf("%s", nomeArquivoCSV);
            scanf("%s", nomeArquivoBin);
            CreateTable(nomeArquivoCSV, nomeArquivoBin);
            break;

        case 2:
            // Imprime todos os registros do arquivo binário
            scanf("%s", nomeArquivoBin);
            Select(nomeArquivoBin);
            break;

        case 3:
            // Busca registros com critérios definidos pelo usuário
            int nroBuscas;
            scanf("%s", nomeArquivoBin);
            scanf("%d", &nroBuscas);
            Where(nomeArquivoBin, nroBuscas);
            break;

        case 4:
            // Remove logicamente registros que atendem aos critérios
            int nroRemocoes;
            scanf("%s", nomeArquivoBin);
            scanf("%d", &nroRemocoes);
            Delete(nomeArquivoBin, nroRemocoes);
            break;

        case 5:
            // Insere novos registros no arquivo binário
            int nroInsercoes;
            scanf("%s", nomeArquivoBin);
            scanf("%d", &nroInsercoes);
            Insert(nomeArquivoBin, nroInsercoes);
            break;

        case 6:
            // Atualiza registros que atendem aos critérios
            int nroAtualizacoes;
            scanf("%s", nomeArquivoBin);
            scanf("%d", &nroAtualizacoes);
            Update(nomeArquivoBin, nroAtualizacoes);
            break;

        case 7:
            // Cria o arquivo de índice árvore-B a partir do arquivo binário
            scanf("%s", nomeArquivoBin);
            scanf("%s", nomeArquivoIndice);
            createIndex(nomeArquivoBin, nomeArquivoIndice);
            break;

        case 8:
            // Busca registros usando o índice árvore-B quando possível
            int nroBuscas8;
            scanf("%s", nomeArquivoBin);
            scanf("%s", nomeArquivoIndice);
            scanf("%d", &nroBuscas8);
            searchWithIndex(nomeArquivoBin, nomeArquivoIndice, nroBuscas8);
            break;

        case 9:
            // Insere registros e atualiza o índice árvore-B
            int nroInsercoes10;
            scanf("%s", nomeArquivoBin);
            scanf("%s", nomeArquivoIndice);
            scanf("%d", &nroInsercoes10);
            insertWithIndex(nomeArquivoBin, nomeArquivoIndice, nroInsercoes10);
            break;

        case 10:
        // Remove registros e atualiza o índice árvore-B
            int nroRemocoes9;
            scanf("%s", nomeArquivoBin);
            scanf("%s", nomeArquivoIndice);
            scanf("%d", &nroRemocoes9);
            deleteWithIndex(nomeArquivoBin, nomeArquivoIndice, nroRemocoes9);
            break;

        case 11:
        //
            break;

        case 12:
        //
            break;

        case 13:
        // Ordena os registros do arquivo binário conforme um campo definido pelo usuário. Cria um novo arquivo ordenado.
            scanf("%s", nomeArquivoBin);
            char nomeNovoArquivo[100];
            scanf("%s", nomeNovoArquivo);
            char campoCriterio[50];
            scanf("%s", campoCriterio);
            sortBinary(nomeArquivoBin, campoCriterio, nomeNovoArquivo);
            break;
            
        case 14:
        //
            break;

    }
    return 0;
}