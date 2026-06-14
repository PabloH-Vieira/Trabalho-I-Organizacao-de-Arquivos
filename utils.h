#ifndef UTILS_H
#define UTILS_H

#include "header.h"
#include "register.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/**
 * @struct CriteriosBusca
 * @brief Estrutura rastreadora para a cláusula WHERE (Busca Multicritério).
 *
 * permite buscas combinadas usando a lógica de flags e valores 
 * sobre um ou mais campos. Cada coluna do registro possui 
 * uma flag inteira (0 para ignorar na busca, 1 para avaliar). Os dados alvo da 
 * pesquisa são armazenados na struct interna regBusca. Isso otimiza o processamento, 
 * garantindo que a varredura compare apenas os bytes solicitados pelo usuário.
 */
typedef struct{
    // FLAGS DE ATIVAÇÃO
    int flag_codEstacao;
    int flag_nomeEstacao;
    int flag_codLinha;
    int flag_nomeLinha;
    int flag_codProxEstacao;
    int flag_distProxEstacao;
    int flag_codLinhaIntegra;
    int flag_codEstIntegra;

    // CARGA DOS PARAMETROS DE BUSCA
    // Reaproveita a estrutura Registro para armazenar os dados que serão comparados.
    Registro regBusca;
} CriteriosBusca;


//Struct para guardar o nome das estações e os pares das estações
typedef struct{
    char nomesEstacoes[200][29];    // Matriz para armazenar os nomes
    int numEstacoes;          // Variável para contar o número de estações únicas
    int paresEstacoes[500][2];   // Variável para contar o número de pares de estações únicas
    int numParesEstacao;    // Variável para contar o número de pares de estações únicas
}Estacoes;

/** Função que mapeia e ativa um critério individual de busca.
 *
 * Analisa o nome de um campo fornecido via input do usuário e o seu respectivo valor alvo 
 * em formato de texto. Ativa a flag correspondente na struct CriteriosBusca e converte 
 * o texto para o tipo numérico adequado quando necessário. Implementa a restrição sobre valor 
 * 'NULO': buscas pelo termo resultam no armazenamento do valor -1 para os campos de tamanho fixo.
 * Recebe como parâmetros: criterios (ponteiro para a struct que acumula os parâmetros da busca atual),
 * campo (string contendo o nome exato da coluna) e conteudo (string contendo o valor alvo da busca).
 */
void preencherCriteriosBusca(CriteriosBusca *criterios, char* campo, char* conteudo);

/** Função que erifica se um registro atende aos filtros da cláusula WHERE.
 *
 * Funciona como um avaliador de igualdade lógica para pesquisas com múltiplos critérios. Recebe os parâmetros 
 * ativados pelo usuário (na struct CriteriosBusca) e os compara com os dados reais de um registro. 
 * Implementa uma lógica de comparação, onde o registro  só é aprovado se satisfizer simultaneamente todos os 
 * campos solicitados na busca, já que a pesquisa deve ser feita considerando um ou mais campos. 
 * Possui uma trava de segurança estrutural de nível zero: registros marcados como 
 * logicamente removidos são reprovados instantaneamente, não devendo ser exibidos 
 * ou contabilizados.
 * Recebe como parâmetros: criterios (ponteiro para a struct contendo os parâmetros da busca) e
 * regAtual (ponteiro para o registro que será testado.
 * Retorna 1 se o registro atende a TODOS os critérios ativos; 0 caso contrário.
 */
int checagemCriteriosBusca(CriteriosBusca *criterios, Registro *regAtual);

// Função que lê os critérios de busca fornecidos pelo usuário
void lerCriteriosUsuario(CriteriosBusca *criterios, int quantidade);

// Função que verifica se uma estação é única e atualiza a struct Estacoes
int isEstacaoUnica(Estacoes *estacoes, char *nomeEstAtual);

// Função que verifica se um par de estações é único e atualiza a struct Estacoes
int isParUnico(Estacoes *estacoes, int EstA, int estB);

// Função que verifica se uma estação é única e atualiza a struct Estacoes
void recalcularEstacoesPares(FILE *arquivoBinario, Header *cabecalho);

// Funções fornecidas
void BinarioNaTela(char *arquivo);
void ScanQuoteString(char *str);

#endif