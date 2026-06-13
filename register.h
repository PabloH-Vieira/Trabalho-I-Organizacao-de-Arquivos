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

/**
 * @brief Função auxiliar de roteamento: Converte e preenche os campos do Registro.
 *
 * Atua como um controlador durante o parsing do arquivo CSV. Recebe uma string bruta (buffer)
 * representando uma coluna específica (indicada pelo fieldIndex) e converte o dado para 
 * o tipo apropriado (inteiro ou string), alocando-o no campo correspondente da struct Registro.
 * Implementa rigorosamente as regras de tratamento de valores nulos da especificação: 
 * campos numéricos ausentes recebem o valor -1 e campos de texto ausentes resultam 
 * em tamanho 0.
 *
 * @param buffer Vetor de caracteres contendo o valor extraído da coluna atual do CSV.
 * @param fieldIndex Índice numérico (0 a 7) que identifica a coluna atual do CSV.
 * @param regAtual Ponteiro para o registro em memória RAM que está sendo populado.
 */
void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual);


/**
 * @brief Função auxiliar de serialização: Grava um registro estruturado no arquivo binário.
 *
 * Transforma a struct de dados em RAM em um bloco de exatos 80 bytes 
 * para persistência em disco. Utiliza uma técnica de serialização em buffer para evitar 
 * desalinhamento de memória e otimizar o tempo de I/O.
 * Todo o espaço residual do bloco de 80 bytes que não for ocupado por dados válidos é 
 * rigorosamente preenchido com o caractere '$'.
 *
 *
 * @param registro Ponteiro para a struct contendo os dados extraídos ou atualizados.
 * @param saida Ponteiro do tipo FILE aberto em modo de escrita binária ("wb+" ou "rb+").
 * @param cabecalho Ponteiro para o cabeçalho do arquivo (opcional dependendo do uso interno).
 */
void writeRegistros(Registro *registro, FILE* saida, Header *cabecalho);


/**
 * @brief Função principal de I/O (Leitura): Desempacota um registro de dados do disco e implementa para a RAM.
 *
 * Responsável por ler fisicamente um registro do arquivo binário e mapeá-lo para a 
 * struct interna. O tamanho do registro de dados é estritamente de 80 bytes. 
 * A função lida com o salto de registros logicamente removidos  ('removido' == '1'), 
 * otimizando o reposicionamento do cursor. Os dados extraídos garantem a integridade
 * das strings em memória ao adicionar dinamicamente o '\0', respeitando a regra de 
 * que esse caractere não deve ser persistido no disco.
 *
 * @param registro Ponteiro para a struct em RAM que receberá os dados deserializados.
 * @param file Ponteiro do tipo FILE correspondente ao arquivo de dados binário.
 * @return 1 em caso de leitura bem-sucedida, 2 se o registro foi pulado (removido), 
 * ou 0 em caso de fim de arquivo (EOF) ou erro de I/O.
 */
int readRegistros(Registro *registro, FILE* file);

/**
 * @brief Função de formatação e saída (Output): Imprime um registro formatado na saída padrão.
 *
 * Responsável por traduzir a representação interna de um registro em memória RAM 
 * para o output exigido. Garante que registros marcados como logicamente removidos
 * sejam ignorados e não exibidos. Transforma os valores sentinela de nulidade 
 * (-1 para campos inteiros e tamanho 0 para campos de tamanho variável) na palavra-chave
 * "NULO". A impressão obedece estritamente à formatação sequencial, exibindo os dados em 
 * linha única e separados por espaços, e segue rigorosamente a ordem específica dos campos.
 *
 * @param registro Ponteiro para a struct contendo os dados extraídos do arquivo que serão exibidos.
 */
void printRegistros(Registro *registro);

/**
 * @brief Função auxiliar de entrada: Preenche um novo registro a partir da entrada padrão (stdin).
 *
 * Utilizada pela Funcionalidade [5] (INSERT) para ler a sequência exata de campos digitada 
 * pelo usuário e mapeá-la para a struct interna Registro. Implementa o tratamento da 
 * palavra-chave "NULO", convertendo-a fisicamente para o inteiro sentinela -1.
 * Para os campos de texto de tamanho variável, utiliza a função auxiliar ScanQuoteString 
 * para remover aspas duplas e processar espaços em branco com segurança.
 *
 * @param novoReg Ponteiro para o registro em memória RAM que será populado.
 */
void preencherNovoRegistro(Registro *novoReg);

/**
 * @brief Função auxiliar de atualização: Aplica os novos valores a um registro carregado em memória.
 *
 * Processa as solicitações de atualização mapeadas nos vetores paralelos de 
 * campos e valores. Identifica a qual coluna o novo dado pertence e altera a struct interna 
 * do registro correspondente. Implementa rigorosamente as regras de tratamento de campos nulos
 * da Funcionalidade [6]: atualizações com a string "NULO" forçam campos de tamanho fixo 
 * a assumirem o valor sentinela -1, enquanto campos de tamanho variável têm seu 
 * indicador de tamanho reduzido a 0.
 *
 * @param registro Ponteiro para a struct em RAM alvo da atualização.
 * @param camposUpdate Matriz de strings contendo os nomes das colunas a serem atualizadas.
 * @param valoresUpdate Matriz de strings contendo os novos valores (ou a string "NULO").
 * @param p Quantidade total de campos que sofrerão atualização neste registro.
 */
void updateRegistro(Registro *registro, char camposUpdate[][50], char valoresUpdate[][100], int p);

#endif