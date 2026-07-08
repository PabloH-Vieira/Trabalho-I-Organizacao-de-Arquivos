#ifndef FUNCTIONALITIES_H
#define FUNCTIONALITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "register.h"
#include "utils.h"

/** Implementa a Funcionalidade [1] (CREATE TABLE): Gera um arquivo binário a partir de um CSV.
 *
 * Realiza a leitura sequencial e a conversão de dados brutos de um arquivo de texto (CSV) 
 * para um formato estruturado binário de tamanho fixo (80 bytes por registro). Gerencia a inicialização
 * do cabeçalho do arquivo e aplica rigorosamente as regras de tratamento de valores nulos 
 * (-1 para numéricos) e preenchimento de lixo ('$') no espaço residual dos registros.
 * Recebe como parâmetros InputFile (nome do arquivo de dados CSV de entrada) e OutPutFile 
 * (nome do arquivo binário de saída que será gerado).
 */
void CreateTable(char* InputFile, char* OutPutFile);


/** Implementa a Funcionalidade [2] (SELECT ALL): Exibe todos os registros ativos do banco de dados.
 * * Realiza uma varredura sequencial completa (Table Scan) no arquivo binário de dados a partir
 * do byte 17. Recupera, formata e imprime na saída padrão todos os registros armazenados. 
 * Registros que possuem o byte de status marcado como logicamente removidos ('1') são 
 * estritamente ignorados e saltados durante a exibição.
 * Recebe como parâmetro FileName (nome do arquivo binário de dados a ser lido).
 */
void Select(char *FileName);


/** Implementa a Funcionalidade [3] (SELECT WHERE): Busca sequencial com filtragem de campos.
 * * Processa consultas dinâmicas realizando uma varredura sequencial (Table Scan) no arquivo 
 * binário para comparar os registros contra um ou mais campos fornecidos pelo usuário. 
 * Ignora registros logicamente removidos e exibe apenas aqueles que satisfazem 
 * simultaneamente todos os critérios de busca estipulados na entrada.
 * Recebe como parâmetros FileName (nome do arquivo binário de dados) e nroBuscas 
 * (quantidade de operações de consulta a serem realizadas).
 */
void Where(char *FileName, int nroBuscas);


/** Implementa a Funcionalidade [4] (DELETE): Remoção lógica de registros do arquivo de dados.
 *
 * Localiza registros que atendem aos critérios de busca (WHERE) fornecidos e realiza a 
 * desativação lógica alterando o byte de status do registro ('removido' = '1'). Gerencia o 
 * reaproveitamento de espaço inserindo o RRN do registro deletado no topo da pilha de 
 * removidos (LIFO), atualizando o encadeamento físico no arquivo de dados e no cabeçalho.
 * Recebe como parâmetros FileName (nome do arquivo binário de dados onde ocorrerão as exclusões) 
 * e nroRemocoes (quantidade de blocos de remoção a serem processados).
 */
void Delete(char *FileName, int nroRemocoes);


/** Implementa a Funcionalidade [5] (INSERT INTO): Inserção de novos registros no arquivo de dados.
 * * Adiciona novos dados utilizando uma abordagem dinâmica de reaproveitamento de espaços 
 * logicamente removidos. Verifica o campo 'topo' do cabeçalho: se houver espaço na pilha 
 * (topo != -1), o algoritmo sobrescreve o registro removido correspondente (Pop). Caso contrário, 
 * anexa o novo registro fisicamente no final do arquivo, expandindo-o.
 * Recebe como parâmetros FileName (nome do arquivo binário de dados que será atualizado) 
 * e nroInsercoes (quantidade de novos registros lidos da entrada padrão).
 */
void Insert(char *FileName, int nroInsercoes);


/** Implementa a Funcionalidade [6] (UPDATE): Atualização in-place de campos em registros existentes.
 * * Realiza a varredura sequencial para encontrar registros que correspondam aos critérios de 
 * filtragem e modifica fisicamente (in-place) apenas os campos solicitados pelo usuário. 
 * Respeita a limitação do bloco físico de 80 bytes, garantindo que alterações em strings de 
 * tamanho variável tenham o espaço residual devidamente sobrescrito com o caractere de lixo '$'.
 * Recebe como parâmetros FileName (nome do arquivo binário de dados a ser alterado) e 
 * nroAtualizacoes (quantidade de operações de modificação a serem aplicadas).
 */
void Update(char *FileName, int nroAtualizacoes);


/** Implementa a Funcionalidade [13] (ORDER BY): Ordenação interna e desfragmentação estrutural.
 * * Carrega todos os registros ativos (não removidos) do arquivo de dados para a memória RAM, 
 * ordena-os utilizando o algoritmo nativo Quicksort em tempo O(n log n) com base no campo 
 * escolhido como critério, e os persiste sequencialmente em um novo arquivo binário. 
 * Atua como uma operação de 'Vacuum', expurgando permanentemente os registros removidos.
 * Recebe como parâmetros inputFileName (nome do arquivo binário original desordenado), 
 * campoOrdenacao (string com o nome da coluna de critério) e outputFileName (nome do arquivo ordenado).
 */
void sortBinary(char *inputFileName, char *campoOrdenacao, char *outputFileName);

#endif