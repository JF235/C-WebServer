#ifndef HTTPHELPER_H
#define HTTPHELPER_H

#include "essentials.h"

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
- HTTP_OK: 200
- HTTP_FORBIDDEN: 403
- HTTP_NOT_FOUND: 404
- HTTP_INTERNAL_SERVER_ERROR: 500
*/
typedef int http_code;

/* 
Associação http_request:
- HTTP_GET: 0
- HTTP_HEAD: 1
- HTTP_OPTIONS: 2
- HTTP_TRACE: 3
*/
typedef int http_request;

/*
Transforma um código de resposta http (`http_code`) em uma string com:
"<código> <texto>"

Por exemplo, o código 404 retorna
"404 Not Found"
*/
char *getHttpStatusText(http_code code);

/*
Converte a string com o nome da requisição para o identificador
`http_request` associado.


- "GET": `HTTP_GET`
- "HEAD": `HTTP_HEAD`
- "OPTIONS": `HTTP_OPTIONS`
- "TRACE": `HTTP_TRACE`
*/
http_request httpReqText2Number(const char *reqText);

#endif