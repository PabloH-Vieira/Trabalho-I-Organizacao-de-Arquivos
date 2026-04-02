#include "file.h"
#include <stdio.h>

    void newHeader(Header *header){
        header->status = '0';
        header->topo = -1;
        header->proxRRN = 0;
        header->nroEstacoes = 0;
        header->nroParesEstacao = 0;
    }

    void writeHeader(Header *header, FILE* file){
        fseek(file, 0, SEEK_SET);
        //Escreve o cabeçalho no arquivo campo a campo para evitar o padding
        fwrite(&header->status, sizeof(char), 1, file);
        fwrite(&header->topo, sizeof(int), 1, file);
        fwrite(&header->proxRRN, sizeof(int), 1, file);
        fwrite(&header->nroEstacoes, sizeof(int), 1, file);
        fwrite(&header->nroParesEstacao, sizeof(int), 1, file);
    }

    void preencherCampos(char buffer[256], int fieldIndex, Registro *regAtual){
        switch(fieldIndex){
            case 0:
                //Verifica se o campo tem valor nulo
                regAtual->codEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 1:
                strcpy(regAtual->nomeEstacao, buffer);
                //Passa o tamanho do nome da estação
                regAtual->tamNomeEstacao = strlen(regAtual->nomeEstacao);
                break;
            case 2:
                regAtual->codLinha = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 3:
                strcpy(regAtual->nomeLinha, buffer);
                //Passa o tamanho do nome da linha
                regAtual->tamNomeLinha = strlen(regAtual->nomeLinha);
                break;
            case 4:
                regAtual->codProxEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 5:
                regAtual->distProxEstacao = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 6:
                regAtual->codLinhaIntegra = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
            case 7:
                regAtual->codEstIntegra = (buffer[0] == '\0') ? -1 : atoi(buffer);
                break;
        }
    }

    void preencherRegistros(Registro *registro, FILE* saida, Header *cabecalho){
        //Escreve o registro no arquivo de saída
        fwrite(registro, sizeof(Registro), 1, saida);
        cabecalho->proxRRN++; //Incrementa o RRN
        cabecalho->nroEstacoes++; //Incrementa o número de estações

        //Preenche campos não presentes no CSV
        registro->removido = '0';
        registro->proximo = -1;
    }

    FILE* CreateTable(char *inputFileName, char *outputFileName) {
        FILE* entrada = fopen(inputFileName, "r");

        //Justificar o wb+
        FILE* saida = fopen(outputFileName, "wb+");

        //Inicializa o cabeçalho do arquivo de saída
        Header cabecalho;
        newHeader(&cabecalho);

        //Escreve o cabeçalho no arquivo de saída
        writeHeader(&cabecalho, saida);

        //Variáveis auxiliares para a leitura do CSV
        char buffer[256];   //Buffer para armazenar os caracteres lidos no CSV
        int posBuffer = 0; //Índice para rastrear a posição atual no buffer
        char c; //Char que armazena o caractere lido do arquivo de entrada
        int fieldIndex = 0; //Índice para rastrear qual campo do registro está sendo preenchido
        Registro regAtual;

        //Pula a primeira linha do arquivo de entrada (cabeçalho)
        while(fread(&c, sizeof(char), 1, entrada) == 1 && c != '\n');

        //Loop de leitura dos campos do arquivo de entrada (caractere a caractere)
        while(fread(&c, sizeof(char), 1, entrada) == 1){
            if(c == ',' || c == '\n'){ //Verifica a condição de final de um campo ou de um registro
                buffer[posBuffer] = '\0'; //Sinaliza o final da string no buffer

                //Função que preenche o campo correspondente no registro atual com base no índice do campo
                preencherCampos(buffer, fieldIndex, &regAtual);
                posBuffer = 0; //Sinaliza que o buffer está vazio para a leitura do próximo campo
                
                //Se o registro foi completamente preenchido (quebra de linha), escreve no binário. Se não, preenche o próximo campo
                if (c == '\n') {
                    preencherRegistros(&regAtual, saida, &cabecalho);
                    fieldIndex = 0; //Os campos foram todos preenchidos, o próximo campo será o primeiro do próximo registro
                } else {
                    fieldIndex++; //Sinaliza que a leitura de um campo foi concluída, passando para o próximo
                }
            }
            
            //A leitura do campo ainda não foi concluída, continua lendo os caracteres
            else{
                //Adiciona o caractere ao buffer
                buffer[posBuffer] = c;
                posBuffer++; //Incrementa o índice do buffer para a próxima posição
            }
        }

        cabecalho.status = '1'; //Sinaliza que o arquivo foi criado corretamente
        fseek(saida, 0, SEEK_SET); //Posiciona o ponteiro do arquivo no início para atualizar o cabeçalho
        writeHeader(&cabecalho, saida); //Atualiza os valores do cabeçalho no arquivo de saída
        fclose(entrada);
        return saida;
    }

