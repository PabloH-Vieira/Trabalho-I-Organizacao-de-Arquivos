#ifndef FILE_H
#define FILE_H
#include <stdio.h>

typedef struct fileHeader{
    char status;
    int topo;
    int proxRRN;
    int nroEstacoes;
    int nroParesEstacao;
}Header;

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

//Funções auxiliares
// Função que preenche os campos de um novo registro com valores nulos
void newHeader(Header *header);

// Função que escreve a struct header no arquivo binário
void writeHeader(Header *header, FILE* file);

// Função que lê o cabeçalho do arquivo binário e preenche a struct Header
void readHeader(Header *header, FILE* file);

// Função que preenche os campos da struct Registro com os valores lidos do arquivo CSV
void writeCampos(char buffer[256], int fieldIndex, Registro *regAtual);

// Função que escreve a struct Registro no arquivo binário
void writeRegistros(Registro *registro, FILE* saida, Header *cabecalho);

// Função que lê os campos de um registro do arquivo binário e preenche a struct Registro
int readRegistros(Registro *registro, FILE* file);

// Função que imprime os campos de um registro na tela
void printRegistros(Registro *registro);

// Função que preenche a struct CriteriosBusca com os critérios de busca fornecidos pelo usuário
void preencherCriteriosBusca(CriteriosBusca *criterios, char* campo, char* conteudo);

// Função que verifica se um registro atende aos critérios de busca fornecidos pelo usuário
int checagemCriteriosBusca(CriteriosBusca *criterios, Registro *regAtual);

// Função que preenche os campos de um registro com os valores fornecidos pelo usuário para inserção
void preencherNovoRegistro(Registro *novoReg);

// Função que atualiza os campos de um registro com os valores fornecidos pelo usuário para atualização
void updateRegistro(Registro *registro, char camposUpdate[][50], char valoresUpdate[][100], int p);

// Funções fornecidas
void BinarioNaTela(char *arquivo);
void ScanQuoteString(char *str);

//Funções das funcionalidades principais
void CreateTable(char* InputFile, char* OutPutFile);
void Select(char *FileName);
void Where(char *FileName, int nroBuscas);
void Delete(char *FileName, int nroRemocoes);
void Insert(char *FileName, int nroInsercoes);
void Update(char *FileName, int nroAtualizacoes);

#endif