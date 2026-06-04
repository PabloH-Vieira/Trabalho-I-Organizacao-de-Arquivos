#ifndef FUNCTIONALITIES_H
#define FUNCTIONALITIES_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "header.h"
#include "register.h"
#include "utils.h"

//Funções das funcionalidades principais
void CreateTable(char* InputFile, char* OutPutFile);
void Select(char *FileName);
void Where(char *FileName, int nroBuscas);
void Delete(char *FileName, int nroRemocoes);
void Insert(char *FileName, int nroInsercoes);
void Update(char *FileName, int nroAtualizacoes);

#endif