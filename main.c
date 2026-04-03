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
            break;
        
        case 5:
            break;
        
        case 6:
            break;
    }



    return 0;
}