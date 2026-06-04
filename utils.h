#include "register.h"
#include "header.h"

typedef struct{
    //Flags para indicar quais campos são critérios de busca
    int flag_codEstacao;
    int flag_nomeEstacao;
    int flag_codLinha;
    int flag_nomeLinha;
    int flag_codProxEstacao;
    int flag_distProxEstacao;
    int flag_codLinhaIntegra;
    int flag_codEstIntegra;

    //Valores dos campos a serem buscados, caso o campo seja um critério de busca
    Registro regBusca;
}CriteriosBusca;

//Struct para guardar o nome das estações e os pares das estações
typedef struct{
    char nomesEstacoes[200][29];    // Matriz para armazenar os nomes
    int numEstacoes;          // Variável para contar o número de estações únicas
    int paresEstacoes[500][2];   // Variável para contar o número de pares de estações únicas
    int numParesEstacao;    // Variável para contar o número de pares de estações únicas
}Estacoes;

// Função que preenche a struct CriteriosBusca com os critérios de busca fornecidos pelo usuário
void preencherCriteriosBusca(CriteriosBusca *criterios, char* campo, char* conteudo);

// Função que verifica se um registro atende aos critérios de busca fornecidos pelo usuário
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