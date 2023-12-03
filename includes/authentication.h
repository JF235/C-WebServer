#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "essentials.h"

/*
Devolve true se os dados de auth (base64) batem com o elemento guardado em .htpasswd apontado pelo arquivo .htaccess (o caminho desse arquivo está em `resourceInfo.htacces`)
*/
bool authenticate(webResource resourceInfo, char *auth);

/*
Encripta a senha contida na autenticacao de auth (em base64)
e devolve a versão decodificada em decodedAuth com
user:cryptedpasswd
*/
void cryptPassword(char *auth, char *decodedAuth);

#endif