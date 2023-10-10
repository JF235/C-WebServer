#ifndef WEBSPACEMANAGER_H
#define WEBSPACEMANAGER_H

#include "essentials.h"

#define MAX_PATH_SIZE 512
#define HTTP_OK 200
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_INTERNAL_SERVER_ERROR 500
#define HTTP_NOT_IMPLEMENTED 501

#define HTTP_GET 0
#define HTTP_HEAD 1
#define HTTP_OPTIONS 2
#define HTTP_TRACE 3

/* 
Associação http_code:
- MAX_PATH_SIZE 512
- HTTP_OK 200
- HTTP_FORBIDDEN 403
- HTTP_NOT_FOUND 404
- HTTP_INTERNAL_SERVER_ERROR 500
*/
typedef int http_code;

/* 
Associação http_request:
- HTTP_GET 0
- HTTP_HEAD 1
- HTTP_OPTIONS 2
- HTTP_TRACE 3
*/
typedef int http_request;

/*
webResource:
- `resourcePath` caminho do recurso resolvido.
- `httpCode` código de status HTTP da resposta.
- `bytes` número de bytes enviados em resposta.
*/
typedef struct webResource
{
    char resourcePath[MAX_PATH_SIZE + 16];
    http_request httpCode;
    ssize_t bytes;
} webResource;

/*
Processa uma requisição HTTP `reqText` (GET, HEAD, OPTIONS, TRACE)
Associada ao espaço web com caminho `webPath` e caminho de recurso `resource`.
O resultado será encaminhado para stdout por padrão.
*/
webResource httpRequest(char *webPath, char *resource, char *reqText);

/*
Imprime a resposta HTTP associada ao recurso `resourceInfo` e requisição `req`.

A resposta é impressa usando `printf()` e encaminhada na saída padrão, `FILE *stdout`.
Para imprimir em outro lugar, basta redirecionar a saída de `stdout` antes de fazer a chamada para essa função.

- Caso `resourceInfo.httpCode` represente um erro. Imprime somente o cabeçalho.
- Caso `resourceInfo.httpCode` represente um OK. Imprime cabeçalho e conteúdo.
*/
void httpRespond(webResource resourceInfo, http_request req);

/*
Função que faz a busca pelo recurso endereçado pelo espaço web `webPath` e caminho relativo `resource`.

Retorna `webResource` com código HTTP e caminho do recurso, caso esteja acessível.
*/
webResource checkWebResource(const char *webPath, const char *resource);

/* 
Imprime o header de uma resposta associado ao código `code`, recurso `resourcePath` e requisição do tipo `req`.
*/
void printHeader(http_code code, const char *resourcePath, http_request req);

/* 
Imprime o header de um erro associado ao código `code`.
*/
void printErrorHeader(http_code code);

/* 
Faz a leitura do conteúdo presente no recurso indicado por `resourcePath` e envia para a saída padrão.
*/
void printResource(const char *resourcePath);

/* 
Verifica se o arquivo de caminho `filePath` é um subarquivo contido na pasta de caminho `folderPath`.
*/
int isSubfile(const char *filePath, const char *folderPath) ;

#endif