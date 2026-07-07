#ifndef REGISTER_H
#define REGISTER_H

#include "header.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
    char removido;
    int proximo;
    int codEstacao;
    int codLinha;
    int codProxEstacao;
    int distProxEstacao;
    int codLinhaIntegra;
    int codEstIntegra;
    int tamNomeEstacao;
    char nomeEstacao[100];
    int tamNomeLinha;
    char nomeLinha[100];
}Registro;

/** Função auxiliar de conversão: Processa e preenche os campos do registro a partir de texto.
 *
 * Atua na leitura de arquivos de texto (como tabelas CSV) para processar os dados brutos de cada coluna. 
 * Identifica a coluna correspondente por meio de um índice numérico, converte o texto para o tipo de dado 
 * adequado (inteiro ou string) e salva o resultado no campo correspondente da estrutura do registro.
 * Implementa as regras de tratamento de valores nulos: campos numéricos ausentes recebem o valor sentinela -1 
 * e campos de texto ausentes definem o tamanho como 0.
 * Recebe como parâmetros buffer (vetor contendo o texto da coluna atual), fieldIndex (posição da coluna 
 * de 0 a 7) e regAtual (ponteiro para o registro em memória RAM que está sendo preenchido).
 */
void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual);


/** Função de escrita em disco: Grava as informações de um registro no arquivo binário de dados.
 *
 * Transforma os dados armazenados na estrutura em memória RAM em um bloco com tamanho fixo de exatos 
 * 80 bytes para salvamento no arquivo físico de dados. Realiza a cópia sequencial de cada 
 * um dos campos estruturados para um vetor auxiliar antes da escrita, evitando desalinhamentos no arquivo. 
 * Preenche obrigatoriamente todo o espaço restante do registro que não foi ocupado por strings válidas 
 * com o caractere '$' para indicar lixo.
 * Recebe como parâmetros registro (ponteiro para as informações em memória), saida (ponteiro do arquivo 
 * de dados aberto em modo de escrita binária) e cabecalho (ponteiro para a estrutura de cabeçalho do arquivo).
 */
void writeRegistros(Registro *registro, FILE* saida, Header *cabecalho);


/** Função principal de leitura: Transfere um registro do arquivo binário para a memória RAM.
 *
 * Lê fisicamente um bloco de 80 bytes do arquivo binário e mapeia os campos sequencialmente para a 
 * estrutura em memória RAM. Identifica registros removidos logicamente por meio do primeiro byte: 
 * se estiver marcado como deletado ('1'), avança o ponteiro do arquivo com fseek para o início do 
 * próximo registro, otimizando o tempo de execução.
 * Recebe como parâmetros registro (ponteiro para a estrutura que armazenará os dados lidos) e file 
 * (ponteiro do arquivo binário aberto para leitura). Retorna 1 para leitura bem-sucedida, 2 se o registro 
 * estava removido e foi pulado, ou 0 para fim de arquivo (EOF) ou erro de leitura.
 */
int readRegistros(Registro *registro, FILE* file);


/** Função de exibição: Imprime os campos de um registro formatados na tela.
 * 
 * Ignora e não exibe registros que estejam marcados com o status de remoção lógica. Converte os 
 * valores de campos sem informação (inteiros iguais a -1 ou tamanhos de string iguais a 0) 
 * na palavra-chave textual "NULO".
 * Recebe como parâmetro registro (ponteiro para a estrutura com os dados que serão mostrados).
 */
void printRegistros(Registro *registro);


/** Função de entrada de dados: Preenche um registro com valores digitados pelo usuário.
 *
 * Lê sequencialmente os dados inseridos no input para carregar os atributos de um novo 
 * registro em memória. Realiza a ocorrência e tratamento do termo "NULO" digitado pelo usuário, 
 * convertendo-o para o valor numérico -1 nos campos inteiros.
 * Recebe como parâmetro novoReg (ponteiro para a estrutura em memória que receberá as informações digitadas).
 */
void preencherNovoRegistro(Registro *novoReg);


/** Função de modificação: Atualiza os dados de um registro com base em novos valores.
 *
 * Compara as colunas solicitadas pelo usuário com os atributos do registro carregado em memória RAM. 
 * Realiza as alterações e atualizações de valores de forma condicional apenas nos campos indicados por 
 * parâmetros. Atualizações contendo o texto "NULO" fazem os campos numéricos assumirem o valor -1, e
 * fazem os campos de texto terem seu indicador de tamanho definido como 0. Recebe como parâmetros registro
 * (ponteiro para a estrutura alvo das alterações), camposUpdate (matriz com os nomes das colunas que devem
 * ser mudadas), valoresUpdate (matriz contendo as strings dos novos valores para cada coluna) e p
 * (quantidade de campos que serão alterados neste ciclo).
 */
void updateRegistro(Registro *registro, char camposUpdate[][50], char valoresUpdate[][100], int p);

#endif