#ifndef TREEFUNCTIONALITIES_H
#define TREEFUNCTIONALITIES_H
#include "binaryTree.h"
#include "treeUtils.h"
#include "utils.h"

/**
 * @brief Implementa a Funcionalidade [7] (CREATE INDEX): Gera um índice Árvore-B a partir do arquivo de dados.
 *
 * Realiza uma varredura sequencial no arquivo binário de dados (criado na Funcionalidade 1)
 * para construir um índice Árvore-B de ordem m=4. O campo indexado como chave primária é 
 * o 'codEstacao'. O algoritmo calcula dinamicamente o byte offset de cada registro 
 * para gerar o campo de referência de dados correspondente. Registros marcados como 
 * logicamente removidos são estritamente ignorados e não geram chaves de busca no índice[cite: 672].
 *
 * @param binFileName Nome ou caminho do arquivo binário de dados (origem).
 * @param indexFileName Nome ou caminho do arquivo binário de índice Árvore-B (destino).
 */
void createIndex(char *binFileName, char *indexFileName);


/**
 * @brief Implementa a Funcionalidade [8] (SELECT WHERE COM ÍNDICE): Busca otimizada de registros.
 *
 * Realiza consultas dinâmicas no arquivo de dados utilizando uma arquitetura de roteamento 
 * de busca. Se o critério de seleção envolver a chave primária indexada ('codEstacao'), 
 * a consulta é delegada à Árvore-B, acessando fisicamente apenas a página de índice e o 
 * registro de dados exato. Se a busca não envolver a chave  indexada, o algoritmo realiza
 * a varredura sequencial completa do arquivo de dados a partir do byte 17.
 *
 * @param binFileName Nome ou caminho do arquivo binário de dados (origem).
 * @param indexFileName Nome ou caminho do arquivo binário de índice Árvore-B (auxiliar).
 * @param nroBuscas Quantidade de pesquisas distintas que serão executadas em sequência.
 */
void searchWithIndex(char *binFileName, char *indexFileName, int nroBuscas);


/**
 * @brief Implementa a Funcionalidade [9] (INSERT INTO COM ÍNDICE): Inserção dupla em Dados e Árvore-B.
 *
 * Adiciona novos registros ao arquivo binário de dados utilizando a abordagem dinâmica 
 * de reaproveitamento de espaços de registros logicamente removidos. 
 * Simultaneamente, as chaves de busca (codEstacao) e os *byte offsets* referentes aos novos 
 * registros são inseridos no arquivo de índice da Árvore-B gerado na Funcionalidade [7]. 
 * A função gerencia a concorrência entre os dois arquivos, garantindo que ambos permaneçam 
 * sincronizados ou sejam marcados como inconsistentes em caso de falha.
 *
 * @param binFileName Nome do arquivo binário de dados onde ocorrerá a inserção.
 * @param indexFileName Nome do arquivo binário de índice Árvore-B que será atualizado.
 * @param nroInsercoes Quantidade de registros que serão lidos da entrada padrão e inseridos.
 */
void insertWithIndex(char *binFileName, char *indexFileName, int nroInsercoes);


/**
 * @brief Implementa a Funcionalidade [10] (DELETE COM ÍNDICE): Remoção lógica síncrona em Dados e Árvore-B.
 *
 * Realiza múltiplas remoções lógicas baseadas em critérios de busca (WHERE) fornecidos pelo usuário.
 * Utiliza uma lógica de busca dupla: se a chave primária ('codEstacao') for um critério ativo, 
 * localiza o registro via Árvore-B em tempo O(log n); caso contrário, percorre sequencialmente.
 * Para cada registro removido, a função realiza de forma síncrona:
 * 1. A delegação da remoção da chave primária no arquivo de índice através da função removeKey.
 * 2. A atualização da pilha de registros removidos no arquivo de dados (reescrevendo 5 bytes).
 * 3. O recálculo das estatísticas globais do cabeçalho de dados (número de estações e pares únicos).
 *
 * @param binFileName Nome ou caminho do arquivo binário de dados (.bin).
 * @param indexFileName Nome ou caminho do arquivo binário de índice Árvore-B (.bin).
 * @param nroRemocoes Quantidade de ciclos independentes de remoção que serão executados.
 */
void deleteWithIndex(char *binFileName, char *indexFileName, int nroRemocoes);

#endif