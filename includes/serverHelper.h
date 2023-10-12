#ifndef SERVERHELPER_H
#define SERVERHELPER_H

#include "essentials.h"

#define MAX_BUFFER_SIZE 2048

#define MAX_NUMBER_CHLD 2
#define SERVER_READ_TIMEOUT_MS 3000 // [penultimo digito do RA] + 3

/*
Processa uma conexão estabelecida no novo socket `newSock`.
- Lê a requisição enviada no socket com `readRequest`
- Faz o parse da requisição com `parseRequest`
- Responde a requisição com `respondeRequest`
- Por fim, exibe uma mensagem de confirmação.

Retorna: Número do status
0 pode significar que foi lido um EOF.
*/
int processConnection(int newSock);

/*
Cria um soquete e atribui o endereço de IP local com a porta
passada como argumento. 

Retorna: Número do socket.
*/
int createAndBind(unsigned short port);

/*
Le a requisição no socket de numero `newSock` e grava o conteúdo no buffer `request`

Retorna: Número de bytes recebidos.
0 significa um EOF.
*/
ssize_t readRequest(int newSock, char *request);

/*
Faz o parse do buffer `request`
*/
void parseRequest(char *request);

/* 
Responde a requisição presente na variável `cmdList` e
envia a resposta para o socket `newSock`.

Retorna a `struct webResource` com os campos:
- resourcePath: caminho do recurso
- httpCode: codigo da resposta
- bytes: numero de bytes da resposta
*/
webResource respondRequest(int newSock);

/*
Tratador do sinal SIGCHLD.
*/
void sigchld_handler(int signo);

/*
Configura o tratador para o sinal SIGCHLD e seta
a flag SA_RESTART que reinicia chamadas de sistema bloqueantas
quando a sua execução é interrompida por um sinal.
*/
void config_signals(void);

#endif