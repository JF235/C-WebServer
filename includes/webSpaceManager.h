#ifndef WEBSPACEMANAGER_H
#define WEBSPACEMANAGER_H

#include "essentials.h"

#define MAX_PATH_SIZE 1024

#define WEBSPACE_REL_PATH "/web/meu-webspace"

/*
webResource:
- `resourcePath` caminho do recurso resolvido.
- `httpCode` código de status HTTP da resposta.
- `bytes` número de bytes enviados em resposta.
*/
typedef struct webResource
{
    char resourcePath[MAX_PATH_SIZE + 16];
    char htaccessPath[MAX_PATH_SIZE + 16];
    http_request httpCode;
    ssize_t bytes;
} webResource;

/*
Processa uma requisição HTTP `reqText` (GET, HEAD, OPTIONS, TRACE)
Associada ao espaço web com caminho `webPath` e caminho de recurso `resource`.
O resultado será encaminhado para stdout por padrão.
*/
webResource httpRequest(char *response, char *resource, char *reqText, char *auth);

/*
Imprime a resposta HTTP associada ao recurso `resourceInfo` e requisição `req`.

A resposta é impressa usando `printf()` e encaminhada na saída padrão, `FILE *stdout`.
Para imprimir em outro lugar, basta redirecionar a saída de `stdout` antes de fazer a chamada para essa função.

- Caso `resourceInfo.httpCode` represente um erro. Imprime somente o cabeçalho.
- Caso `resourceInfo.httpCode` represente um OK. Imprime cabeçalho e conteúdo.
*/
void httpRespond(char *response, webResource resourceInfo, http_request req);

/*
Função que faz a busca pelo recurso endereçado pelo espaço web `webPath` e caminho relativo `resource`.

Retorna `webResource` com código HTTP e caminho do recurso, caso esteja acessível.
*/
webResource checkWebResource(const char *resource, bool authenticated);

/* 
Imprime o header de uma resposta associado ao código `code`, recurso `resourcePath` e requisição do tipo `req`.
*/
void printHeader(char *response, const char *resourcePath, http_request req);

/* 
Imprime o header de um erro associado ao código `code`.
*/
void printErrorHeader(char *response, http_code code);

/* 
Faz a leitura do conteúdo presente no recurso indicado por `resourcePath` e envia para a saída padrão.
*/
void printResource(char *response, char *resourcePath);

/* 
Verifica se o arquivo de caminho `filePath` não contém referências
relativas de diretório com `.` ou `..`
*/
int isValid(const char *filePath);

/*
Seta a variável global `webspacePath` que contém o caminho absoluto 
do webspace.
*/
void config_webspace();

/*
Busca o arquivo htaccess e retorna o seu caminho caso
tenha sido encontrado.
Retorna `NULL` caso não tenha sido encontrado.

A string que contém o caminho é alocada dinamicamente,
portano precisa ser liberada após uso com `free()`.
*/
char *findHtaccess(char *resourcePath);

/*
Verifica se o caminho atual faz menção a um recurso especial
que está fora do webspace.

Arquivs especiais:
- forms.html
*/
void checkSpecialResource(char *resourcePath, char *specialPath);

#endif