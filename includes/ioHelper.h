#ifndef IOHELPER_H
#define IOHELPER_H

#include "essentials.h"

/*
Imprime o conteúdo do arquivo de nome `filename` na saída padrão.
*/
void fileName2stdout(char *filename);

/*
Imprime o conteúdo do arquivo aberto em strem `file` na saída padrão.
*/
void file2stdout(FILE *file);

/*
Escreve conteúdo do arquivo aberto em stream `file` no buffer `buff`.
*/
void file2buffer(FILE *file, char *buff);

/*
Escreve conteúdo do arquivo de nome `filename` no buffer `buff`.
*/
void fileName2buffer(char *filename, char *buff);

#endif