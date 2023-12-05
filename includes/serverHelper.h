#ifndef SERVERHELPER_H
#define SERVERHELPER_H

#include "essentials.h"

#define MAX_BUFFER_SIZE 8192
#define BUFFER_SIZE_BIG 102400 // Tamanho de um arquivo HTML
//#define BUFFER_SIZE_HUGE 

#define MAX_THREADS 2
#define SERVER_READ_TIMEOUT_MS 7 * 1000 // [penultimo digito do RA] + 3

/*
Cria um socket e atribui o endereço de IP local com a porta
passada como argumento.

Retorna: Número do socket.
*/
int createAndBind(unsigned short port);

/*
Processa uma conexão estabelecida no novo socket `newSock`.
- Lê a requisição enviada no socket com `readRequest()`
- Faz o parse da requisição com `parseRequest()`
- Responde a requisição com `respondRequest()`
- Por fim, exibe uma mensagem de confirmação.

Retorna: Número do status
0 pode significar que foi lido um EOF.

Pode alterar o estado da conexão, `keepalive`.
*/
int processConnection(int newSock, bool *keepalive);



/*
Le a requisição no socket de numero `newSock` e grava o conteúdo no buffer `request`

Retorna: Número de bytes recebidos.
0 significa fim de conexão, EOF ou TIMEOUT.
*/
ssize_t readRequest(int newSock, char *request);

/*
Faz o parse do buffer `request`.

Retorna um ponteiro para a lista de comandos gerada.
*/
CommandList *parseRequest(char *request);

/*
Responde a requisição presente na variável `cmdList` e
envia a resposta para o socket `newSock`.

Retorna a `struct webResource` com os campos:
- resourcePath: caminho do recurso
- httpCode: codigo da resposta
- bytes: numero de bytes da resposta
*/
webResource respondRequest(int newSock, CommandList *cmdList);

/*
Envia uma resposta HTTP com uma página HTML mínima indicando que
o servidor está sobrecarregado.
*/
int send_response_overload(int sock);

/*
Lida com uma requisição post que deseja alterar as credenciais de um usuário.
*/
void postHandler(webResource *resourceInfo, char *newCredentials);

#endif