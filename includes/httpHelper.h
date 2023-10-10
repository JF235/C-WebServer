#ifndef HTTPHELPER_H
#define HTTPHELPER_H

#include "essentials.h"

/*
Transforma um código de resposta http em uma string com:
"<código> <texto>"

Por exemplo, o código 404 retorna
"404 Not Found"
*/
char *getHttpStatusText(http_code code);

/*
Converte a string com o nome da requisição para o identificador
http_request associado.

```c
#define HTTP_GET     0
#define HTTP_HEAD    1
#define HTTP_OPTIONS 2
#define HTTP_TRACE   3
```
*/
http_request httpReqText2Number(const char *reqText);

#endif