#ifndef IOHELPER_H
#define IOHELPER_H

#include "essentials.h"

/*
Imprime o conteúdo do arquivo de nome `filename` na saída padrão.
*/
void fileName2stdout(char *filename);

/*
Imprime o conteúdo do arquivo aberto em stream `file` na saída padrão.
*/
void file2stdout(FILE *file);

/*
Escreve conteúdo do arquivo aberto em stream `file` no buffer `buff`.

Adiciona `\0` no final.
*/
void file2buffer(FILE *file, char *buff);

/*
Escreve conteúdo do arquivo de nome `filename` no buffer `buff`.

Adiciona `\0` no final.
*/
void fileName2buffer(char *filename, char *buff);


/*
Escreve os `bufferSize` bytes do `buffer` no arquivo de nome `filename`.
*/
void buffer2fileName(char *buffer, char *filename, size_t bufferSize);


#endif