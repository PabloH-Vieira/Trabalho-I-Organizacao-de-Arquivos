#include "file.h"
#include <stdio.h>
#include <stdlib.h>

int main (void){
    //Ler a funcionalidade a ser executada
    int funcionalidade;
    scanf("%d", &funcionalidade);

    switch(funcionalidade){
        case 1:
            //Criar o arquivo binário a partir do arquivo csv
            char nomeArquivoCSV[100];
            char nomeArquivoBin[100];
            scanf("%s", nomeArquivoCSV);
            scanf("%s", nomeArquivoBin);
            CreateTable(nomeArquivoCSV, nomeArquivoBin);
            break;

        case 2:
            //Ler o arquivo binário e imprimir os registros
            char nomeArquivoBin2[100];
            scanf("%s", nomeArquivoBin2);
            Select(nomeArquivoBin2);
            break;

        case 3:
            //Ler o arquivo binário e os critérios de busca, imprimir os registros que atendem aos critérios de busca
            char nomeArquivoBin3[100];
            int nroBuscas;
            scanf("%s", nomeArquivoBin3);
            scanf("%d", &nroBuscas);
            Where(nomeArquivoBin3, nroBuscas);
            break;

        case 4:
            //Ler o arquivo binário e os critérios de busca, marcar os registros que atendem aos critérios de busca como removidos
            char nomeArquivoBin4[100];
            int nroRemocoes;
            scanf("%s", nomeArquivoBin4);
            scanf("%d", &nroRemocoes);
            Delete(nomeArquivoBin4, nroRemocoes);
            break;
            
        case 5:
            char nomeArquivoBin5[100];
            int nroInsercoes;
            scanf("%s", nomeArquivoBin5);
            scanf("%d", &nroInsercoes);
            Insert(nomeArquivoBin5, nroInsercoes);
            break;
        case 6:
            char nomeArquivoBin6[100];
            int nroAtualizacoes;
            scanf("%s", nomeArquivoBin6);
            scanf("%d", &nroAtualizacoes);
            Update(nomeArquivoBin6, nroAtualizacoes);
            break;
    }



    return 0;
}