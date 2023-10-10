#ifndef WEBSPACEMANAGER_H
#define WEBSPACEMANAGER_H

#include "essentials.h"

#define MAX_PATH_SIZE 512

#define WEBSPACE_PATH "/home/jf/C-WebServer/web/meu-webspace"

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
webResource httpRequest(char *response, char *resource, char *reqText);

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
webResource checkWebResource(const char *resource);

/* 
Imprime o header de uma resposta associado ao código `code`, recurso `resourcePath` e requisição do tipo `req`.
*/
void printHeader(char *response, http_code code, const char *resourcePath, http_request req);

/* 
Imprime o header de um erro associado ao código `code`.
*/
void printErrorHeader(char *response, http_code code);

/* 
Faz a leitura do conteúdo presente no recurso indicado por `resourcePath` e envia para a saída padrão.
*/
void printResource(char *response, char *resourcePath);

/* 
Verifica se o arquivo de caminho `filePath` é um subarquivo contido na pasta de caminho `folderPath`.
*/
int isSubfile(const char *filePath, const char *folderPath) ;

#endif