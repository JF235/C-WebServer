#ifndef ERRORHANDLING_H
#define ERRORHANDLING_H

#include "essentials.h"

// ## Verifica se houve erro
// Verifica se a chamada retorna um erro (valor de retorno < 0).
// Em caso positivo, imprime os detalhes do erro e sai do programa.
// Os detalhes do erro são:
// - Nome do arquivo
// - O numero da linha
// - A string que representa a chamada
// - O valor de retorno
// - A string associada ao errno
#define TRY_ERR(x) do { \
    int retval = (x); \
    if (retval < 0) \
    { \
        fprintf(stderr, "\033[1;1m%s:%d:\033[m Runtime error at\n%s\nreturned %d \033[31m(%s)\n", __FILE__, __LINE__, #x, retval, strerror(errno)); \
        exit(EXIT_FAILURE); \
    } \
} while (0);

// ## Verifica se houve erro
// Verifica se a chamada retorna um ponteiro NULL.
// Em caso positivo, imprime os detalhes do erro e sai do programa.
// Os detalhes do erro são:
// - Nome do arquivo
// - O numero da linha
// - A string que representa a chamada
// - A string associada ao errno
#define TRY_NULL(x) do { \
    void *retval = (void *)(x); \
    if (retval == NULL) \
    { \
        fprintf(stderr, "%s:%d Runtime error at\n%s\nreturned NULL (%s)\n", __FILE__, __LINE__, #x, strerror(errno)); \
        exit(EXIT_FAILURE); \
    } \
} while (0);

#endif