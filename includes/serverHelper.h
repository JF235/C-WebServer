#ifndef SERVERHELPER_H
#define SERVERHELPER_H

#include "essentials.h"

#define MAX_BUFFER_SIZE 1024

void processConnection(int newSock);

/*
Cria um soquete e atribui o endereço de IP local com a porta
passada como argumento. 
*/
int createAndBind(unsigned short port);

ssize_t readRequest(int newSock, char *request);

void parseRequest(char *request);

webResource respondRequest(int newSock);

#endif