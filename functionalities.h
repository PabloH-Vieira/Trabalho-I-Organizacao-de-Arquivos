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

/**
 * @brief Funcionalidade de Ordenação Interna (Sort and Rebuild): Ordena os registros e recria o arquivo.
 *
 * Realiza a leitura integral de todos os registros ativos de um arquivo de dados para a memória RAM.
 * Utiliza o algoritmo nativo Quicksort (O(n log n)) para ordenar o vetor dinâmico de registros 
 * com base em um critério (campo) escolhido pelo usuário. Em seguida, serializa os dados ordenados 
 * em um novo arquivo binário. Registros logicamente removidos são descartados durante a leitura, 
 * promovendo a desfragmentação (Vacuum) do novo arquivo.
 *
 * @pre O arquivo de entrada deve estar íntegro (status == '1'). A memória RAM deve ser 
 * suficiente para comportar o vetor de Registro.
 * @post Um novo arquivo binário é gerado com os registros ordenados. O cabeçalho é recalculado, 
 * redefinindo a pilha de removidos (topo = -1) e atualizando o proxRRN para corresponder 
 * exatamente à quantidade de registros consolidados.
 *
 * @param inputFileName Nome do arquivo de dados original (desordenado).
 * @param outputFileName Nome do arquivo de dados destino (ordenado).
 * @param campoOrdenacao String indicando o nome da coluna que servirá como critério de ordenação.
 */
void sortBinary(char *inputFileName, char *campoOrdenacao, char *outputFileName);

/**
 * @brief Implementa a Funcionalidade [14] (Sort-Merge Join): Junção de dois arquivos de dados.
 *
 * Realiza o cruzamento de dados (JOIN) entre dois arquivos baseando-se na condição de igualdade
 * entre estacao1.codProxEstacao e estacao2.codEstacao.
 * O algoritmo executa a preparação prévia ordenando ambos os arquivos (via Funcionalidade [13]) 
 * e gravando arquivos temporários. Em seguida, lê os arquivos ordenados simultaneamente 
 * em tempo O(N + M). Após o cruzamento e exibição dos dados, os arquivos temporários são deletados.
 *
 * @pre A função sortDataFile e readRegistros devem estar implementadas. 
 * @post Os arquivos temporários ordenados são criados no disco e removidos após o uso.
 *
 * @param arq1 Nome do primeiro arquivo de dados (ex: estacao1.bin).
 * @param campo1 Nome do campo de junção do arq1 ("codProxEstacao").
 * @param arq2 Nome do segundo arquivo de dados (ex: estacao2.bin).
 * @param campo2 Nome do campo de junção do arq2 ("codEstacao").
 */
void juncaoOrdenacao(char *arq1, char *campo1, char *arq2, char *campo2);

#endif