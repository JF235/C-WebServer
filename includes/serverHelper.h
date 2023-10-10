#ifndef SERVERHELPER_H
#define SERVERHELPER_H

#include "essentials.h"

#define MAX_BUFFER_SIZE 1024

/*
Cria um soquete e atribui o endereço de IP local com a porta
passada como argumento. 
*/
int createAndBind(unsigned short porta);


int processConnection(int newSock);

#endif