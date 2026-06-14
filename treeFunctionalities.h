#ifndef TREEFUNCTIONALITIES_H
#define TREEFUNCTIONALITIES_H
#include "binaryTree.h"
#include "treeUtils.h"
#include "utils.h"

/** Implementa a Funcionalidade [7] (CREATE INDEX): Gera um índice Árvore-B a partir do arquivo de dados.
 *
 * Realiza uma varredura sequencial no arquivo binário de dados (criado na Funcionalidade 1)
 * para construir um índice Árvore-B de ordem m=4. O campo indexado como chave primária é 
 * o 'codEstacao'. O algoritmo calcula dinamicamente o byte offset de cada registro 
 * para gerar o campo de referência de dados correspondente. Registros marcados como 
 * logicamente removidos são estritamente ignorados e não geram chaves de busca no índice.
 * Recebe como parâmetros binFileName (Nome do arquivo binário de dados) e indexFileName 
 * (nome do arquivo binário de índice Árvore-B).
 */
void createIndex(char *binFileName, char *indexFileName);


/** Implementa a Funcionalidade [8] (SELECT WHERE COM ÍNDICE): Busca otimizada de registros.
 * 
 * Realiza consultas dinâmicas no arquivo de dados utilizando uma arquitetura de roteamento 
 * de busca. Se o critério de seleção envolver a chave primária indexada ('codEstacao'), 
 * a consulta é delegada à Árvore-B, acessando fisicamente apenas a página de índice e o 
 * registro de dados exato. Se a busca não envolver a chave  indexada, o algoritmo realiza
 * a varredura sequencial completa do arquivo de dados a partir do byte 17.
 * Recebe como parâmetros binFileName (nome do arquivo binário de dados), indexFileName 
 * (nome do arquivo binário de índice Árvore-B) e nroBuscas (quantidade de buscas a serem realizadas)
 */
void searchWithIndex(char *binFileName, char *indexFileName, int nroBuscas);


/** Implementa a Funcionalidade [9] (INSERT INTO COM ÍNDICE): Inserção dupla em Dados e Árvore-B.
 * Adiciona novos registros ao arquivo binário de dados utilizando a abordagem dinâmica 
 * de reaproveitamento de espaços de registros logicamente removidos. 
 * Simultaneamente, as chaves de busca e os byte offsets referentes aos novos 
 * registros são inseridos no arquivo de índice da Árvore-B gerado na Funcionalidade [7]. 
 * Recebe como parâmetros binFileName (nome do arquivo binário de dados onde ocorrerá a inserção) 
 * indexFileName (nome do arquivo binário de índice Árvore-B que será atualizado) e nroInsercoes
 * (quantidade de registros que serão lidos da entrada padrão e inseridos)
 */
void insertWithIndex(char *binFileName, char *indexFileName, int nroInsercoes);


/**
 * Implementa a Funcionalidade [10] (DELETE COM ÍNDICE): Remoção lógica síncrona em Dados e Árvore-B.
 *
 * Realiza múltiplas remoções lógicas baseadas em critérios de busca (WHERE) fornecidos pelo usuário.
 * Utiliza uma lógica de busca dupla: se a chave primária ('codEstacao') for um critério ativo, 
 * localiza o registro via Árvore-B. Caso contrário, percorre sequencialmente.
 * Para cada registro removido, a função realiza de forma síncrona:
 * Rece como parâmetros binFileName (Nome do arquivo binário de dados), indexFileName 
 * (nome do arquivo binário de índice Árvore-B) e nroRemocoes (quantidade de remoções)
**/
void deleteWithIndex(char *binFileName, char *indexFileName, int nroRemocoes);

#endif