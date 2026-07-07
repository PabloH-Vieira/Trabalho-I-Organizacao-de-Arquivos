#ifndef FUNCTIONALITIES_H
#define FUNCTIONALITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "register.h"
#include "utils.h"

//Funções das funcionalidades principais

/**
 * @brief Implementa a Funcionalidade [1] (CREATE TABLE): Converte um arquivo de entrada CSV em um arquivo binário de dados.
 *
 * Realiza a varredura e o processamento de um arquivo .csv, gerando registros de dados
 * com tamanho fixo de exatos 80 bytes no arquivo de saída. A operação é realizada
 * estritamente em modo binário. A função cria e gerencia um registro de cabeçalho 
 * de 17 bytes, cujo campo status é iniciado como '0' (inconsistente) durante a escrita 
 * e atualizado para '1' (consistente) ao final do processo.
 * * Regras de formatação aplicadas conforme a documentação:
 * - Campos de tamanho fixo com valores nulos são preenchidos com o inteiro -1.
 * - Campos de tamanho variável com valores nulos registram tamanho zero no campo que conta os caracteres.
 * - O espaço não utilizado de cada registro é obrigatoriamente preenchido com o caractere '$'.
 * * Antes de encerrar a execução e logo após o fechamento do arquivo, a função binarioNaTela 
 * é acionada para exibir a saída do arquivo binário gerado.
 *
 * @param inputFileName Nome do arquivo .csv de origem que contém os valores a serem armazenados.
 * @param outputFileName Nome do arquivo binário de destino (.bin) que será gerado.
 */
void CreateTable(char* InputFile, char* OutPutFile);


/**
 * @brief Implementa a Funcionalidade [2] (SELECT ALL): Recupera e exibe todos os registros do arquivo binário.
 *
 * Realiza uma varredura sequencial no arquivo de dados. Verifica a integridade 
 * do arquivo (status) antes de iniciar o processo. Registros marcados como logicamente removidos 
 * são ignorados e não exibidos. A função de impressão formata os campos, substituindo 
 * valores nulos mapeados (-1 ou tamanho 0) pela string "NULO", conforme exigido pela especificação.
 *
 * @pre O arquivo binário de destino deve existir e possuir status consistente ('1').
 * @post O arquivo é lido de forma não destrutiva. Os dados formatados são enviados para a saída padrão.
 *
 * @param FileName Nome ou caminho do arquivo binário (.bin) a ser consultado.
*/
void Select(char *FileName);


/**
 * @brief Implementa a Funcionalidade [3] (SELECT WHERE): Realiza buscas parametrizadas no arquivo de dados.
 *
 * Executa múltiplas varreduras sequenciais no arquivo binário para encontrar registros
 * que satisfaçam um conjunto de critérios especificados pelo usuário. Otimiza o tempo de 
 * execução interrompendo a busca prematuramente caso o critério envolva o campo 
 * de chave primária única (codEstacao). Ignora logicamente registros marcados como removidos.
 *
 * @param FileName Nome ou caminho do arquivo binário (.bin) onde a busca será realizada.
 * @param nroBuscas Quantidade de pesquisas distintas que serão executadas em sequência.
 */
void Where(char *FileName, int nroBuscas);


/**
 * @brief Implementa a Funcionalidade [4] (DELETE): Remoção lógica de registros baseada em critérios.
 *
 * Realiza múltiplas varreduras no arquivo de dados para localizar registros que satisfaçam
 * condições de busca específicas. Os registros encontrados não são apagados fisicamente, mas 
 * sim marcados como logicamente removidos (removido = '1'). A função gerencia internamente 
 * uma pilha de espaços disponíveis, encadeando os registros deletados através do 
 * campo 'proximo' e atualizando o 'topo' no cabeçalho para garantir abordagens dinâmicas 
 * de reaproveitamento de espaço em inserções futuras.
 *
 * @param FileName Nome ou caminho do arquivo binário (.bin) onde ocorrerão as deleções.
 * @param nroRemocoes Quantidade de ciclos de deleção parametrizada que serão executados.
 */
void Delete(char *FileName, int nroRemocoes);


/**
 * @brief Implementa a Funcionalidade [5] (INSERT INTO): Inserção de registros com reaproveitamento de espaço.
 *
 * Adiciona novos registros ao arquivo binário de dados utilizando a abordagem dinâmica 
 * de reaproveitamento de espaços. A função intercepta a pilha de registros logicamente 
 * removidos gerenciada pelo cabeçalho. Se a pilha estiver vazia (topo = -1), o registro 
 * é apensado no final do arquivo (utilizando o proxRRN). Se houver espaço disponível, 
 * o registro sobrescreve o nó no topo da pilha, e o cabeçalho é atualizado para apontar 
 * para o próximo espaço livre.
 *
 * @param FileName Nome ou caminho do arquivo binário (.bin) onde ocorrerão as inserções.
 * @param nroInsercoes Quantidade de registros que serão lidos da entrada padrão e inseridos no arquivo.
 */
void Insert(char *FileName, int nroInsercoes);

/**
 * @brief Implementa a Funcionalidade [6] (UPDATE): Atualização de registros.
 *
 * Realiza buscas no arquivo de dados baseadas em múltiplos critérios (WHERE). Quando os registros 
 * são encontrados e não estão marcados como removidos, seus dados sofrem modificações 
 * de acordo com novos valores fornecidos (SET). Como os registros possuem um 
 * tamanho estático e invariável de 80 bytes, a atualização ocorre fisicamente no mesmo
 * espaço. Quaisquer fragmentos de memória não utilizados após a atualização de uma string
 * de tamanho variável são preenchidos com lixo ('$').
 * 
 * @param FileName Nome ou caminho do arquivo binário (.bin) alvo das atualizações.
 * @param nroAtualizacoes Número de lotes de atualizações a serem processados.
 */
void Update(char *FileName, int nroAtualizacoes);

#endif