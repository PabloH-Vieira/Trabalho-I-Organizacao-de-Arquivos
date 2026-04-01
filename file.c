#include "file.h"
#include <stdio.h>

    void CreateTable(char *inputFileName, char *outputFileName) {
        FILE* entrada = fopen(inputFileName, "r");
        FILE* saida = fopen(outputFileName, "wb");
        char buffer[256];
        int posBuffer = 0; // Índice para rastrear a posição atual no buffer
        char c; // Char que armazena o caractere lido do arquivo de entrada
        int fieldIndex = 0; // Índice para rastrear qual campo do registro está sendo preenchido
        Registro regAtual;

        //Pula a primeira linha do arquivo de entrada (cabeçalho)
        while(fread(&c, sizeof(char), 1, entrada) == 1 && c != '\n');

        //loop de leitura dos campos do arquivo de entrada (caractere a caractere)
        while(fread(&c, sizeof(char), 1, entrada) == 1){
            if(c == ',' || c == '\n'){
                //Encontramos o final de um campo ou de um registro
                buffer[posBuffer] = '\0'; //Sinaliza o final da string no buffer

                //Função que preenche o campo correspondente no registro atual com base no índice do campo
                preencherCampos(buffer, fieldIndex, regAtual);

                posBuffer = 0; // Reseta o índice do buffer para sinalizar que o próximo campo começará a ser lido do início do buffer
                
                //Verifica se o registro atual foi completamente preenchido (quebra de linha). Se não, preenche o próximo campo
                if (c == '\n') {
                    fwrite(&regAtual, sizeof(Registro), 1, saida);
                    fieldIndex = 0; // Reseta o índice do campo para o próximo registro
                } else {
                    fieldIndex++; //Sinaliza que a leitura de um campo foi concluída, passando para o próximo
                }
            }
            
            //A leitura do campo ainda não foi concluída, continua lendo os caracteres
            else{
                //Adiciona o caractere ao buffer
                buffer[posBuffer] = c;
                posBuffer++; // Incrementa o índice do buffer para a próxima posição
            }
        }
        fclose(entrada);
        fclose(saida);
    }

    void preencherCampos(char buffer[256], int fieldIndex, Registro regAtual){
                switch(fieldIndex){
                    case 0:
                        //Verifica se o campo tem valor nulo
                        if (buffer[0] == '\0')
                            regAtual.codEstacao = -1;
                        else
                            regAtual.codEstacao = atoi(buffer);
                        break;
                    case 1:
                        strcpy(regAtual.nomeEstacao, buffer);
                        //Passa o tamanho do nome da estação
                        regAtual.tamNomeEstacao = strlen(regAtual.nomeEstacao);
                        break;
                    case 2:
                        if (buffer[0] == '\0')
                            regAtual.codLinha = -1;
                        else
                            regAtual.codLinha = atoi(buffer);
                        break;
                    case 3:
                        strcpy(regAtual.nomeLinha, buffer);
                        //Passa o tamanho do nome da linha
                        regAtual.tamNomeLinha = strlen(regAtual.nomeLinha);
                        break;
                    case 4:
                        if (buffer[0] == '\0')
                            regAtual.codProxEstacao = -1;
                        else
                            regAtual.codProxEstacao = atoi(buffer);
                        break;
                    case 5:
                        if (buffer[0] == '\0')
                            regAtual.distProxEstacao = -1;
                        else
                            regAtual.distProxEstacao = atoi(buffer);
                        break;
                    case 6:
                        if (buffer[0] == '\0')
                            regAtual.codLinhaIntegra = -1;
                        else
                        regAtual.codLinhaIntegra = atoi(buffer);
                        break;
                    case 7:
                        if (buffer[0] == '\0')
                            regAtual.codEstIntegra = -1;
                        else
                            regAtual.codEstIntegra = atoi(buffer);
                        break;
                }
    }