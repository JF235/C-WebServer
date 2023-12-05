#ifndef WEBSPACEMANAGER_H
#define WEBSPACEMANAGER_H

#include "essentials.h"

#define PATH_SIZE_SMALL 512   // Tamanho do caminho de um recurso
#define PATH_SIZE_MEDIUM 1024 // Tamanho do caminho do webspace
#define PATH_SIZE_BIG 2048    // Tamanho de um caminho webspace + recurso

#define WEBSPACE_REL_PATH "/web/meu-webspace"

/*
webResource:
- `resourcePath` caminho do recurso resolvido.
- `htacessPath` caminho do arquivo de proteção.
- `httpCode` código de status HTTP da resposta.
- `bytes` número de bytes enviados em resposta.
*/
typedef struct webResource
{
    char resourcePath[PATH_SIZE_BIG];
    char htaccessPath[PATH_SIZE_BIG];
    http_request httpCode;
    ssize_t bytes;
} webResource;

/*
Seta a variável global `webspacePath` que contém o caminho absoluto
do webspace.
*/
void configWebspace(char *webspaceName);

/*
Processa uma requisição HTTP `reqText` (GET, HEAD, OPTIONS, TRACE, POST)
Associada ao espaço web com caminho `webPath` e caminho de recurso `resource`.
O resultado será escrito em `response`.
*/
webResource httpRequest(char *response, char *resource, char *reqText, char *auth, char *newCredentials);

/*
Imprime a resposta HTTP associada ao recurso `resourceInfo` e requisição `req`.

A resposta é impressa usando `printf()` e encaminhada na saída padrão, `FILE *stdout`.
Para imprimir em outro lugar, basta redirecionar a saída de `stdout` antes de fazer a chamada para essa função.

- Caso `resourceInfo.httpCode` represente um erro. Imprime somente o cabeçalho.
- Caso `resourceInfo.httpCode` represente um OK. Imprime cabeçalho e conteúdo.

Retorna o tamanho em bytes da resposta.
*/
int httpRespond(char *response, webResource resourceInfo, http_request req);

/*
Função que faz a busca pelo recurso endereçado pelo espaço web `webPath` e caminho relativo `resource`.

Retorna `webResource` com código HTTP e caminho do recurso, caso esteja acessível.
*/
webResource checkWebResource(const char *resource, bool authenticated);

/*
Imprime o header de uma resposta associado ao código `code`, recurso `resourcePath` e requisição do tipo `req`.
*/
void printHeader(char *response, char *resourcePath, http_request req);

/*
Imprime o header de um erro associado ao código `code`.
*/
void printErrorHeader(char *response, http_code code, char *errorHtmlFile);

/*
Faz a leitura do conteúdo presente no recurso indicado pelo caminho `resourcePath` e escreve no buffer response.

Retorna o tamanho em bytes do recurso
*/
int printResource(char *response, char *resourcePath);

/*
Verifica se o arquivo de caminho `filePath` não contém referências
relativas de diretório com `.` ou `..`
*/
int isValid(const char *filePath);

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

Arquivos especiais:
- forms.html

Escreve o caminho do recurso especial no buffer `specialPath`
*/
void checkSpecialResource(char *resourcePath, char *specialPath);

/*
Define o formato do arquivo enviado, baseando-se na extensão do arquivo.
.html => text/html
.jpeg || .jpg => image/jpeg
*/
void setContentType(char *resourcePath, char *contentType);

#endif