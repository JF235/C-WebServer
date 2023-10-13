#ifndef CLIENT_H
#define CLIENT_H

#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include "../includes/ioHelper.h"
#include <netinet/tcp.h>

#define REQS_PATH "/home/jf/C-WebServer/client_sim/reqs"

/*
Separa o cabecalho e o conteudo da resposta HTTP.

Para isso, dentro da função são alocados espaços de memória para as strings de `cabecalho` e `conteudo`.

Importante: Os endereços precisarão ser desalocados pela função que fez a chamada.
*/
void separarCabecalhoEConteudo(const char *respostaHTTP, char **cabecalho, char **conteudo);

/*
Cria o socket e realiza a conexão ao servidor com ip `serverIp` e porta `port`.
O valor do socket é armazenado no endereço `sock`.
*/
int connectToServer(char *serverIp, char *port);

/*
Envia a requisição `request` pelo socket `serverSock`.
*/
void sendRequest(int serverSock, char *request);

/*
Recebe a resposta pelo socket `serverSock`.
Guarda o conteúdo em um arquivo e também imprime na saída.
*/
int receiveResponse(int serverSock);

#endif