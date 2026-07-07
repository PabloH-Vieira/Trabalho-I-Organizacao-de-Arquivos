#ifndef JOIN_H
#define JOIN_H

#include "register.h"
#include "binaryTree.h"
#include "treeUtils.h"

/*
 * [11] Junção por loop aninhado entre dois arquivos de dados.
 * Condição de junção: arq1.codProxEstacao = arq2.codEstacao
 * Não usa índice — força bruta com dois loops aninhados.
 */
void juncaoLoopAninhado(char *arq1, char *campo1, char *arq2, char *campo2);

/*
 * [12] Junção por loop único entre dois arquivos de dados.
 * Usa o índice árvore-B do arquivo 2 para buscar diretamente
 * o registro correspondente ao codProxEstacao do arquivo 1.
 */
void juncaoLoopUnico(char *arq1, char *campo1, char *arq2, char *campo2, char *indiceArq2);

#endif