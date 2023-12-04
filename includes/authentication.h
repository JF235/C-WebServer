#ifndef AUTHENTICATION_H
#define AUTHENTICATION_H

#include "essentials.h"

#define SALT "EA"

/*
Retorna o caminho do arquivo passwd apontado pelo arquivo `htaccessPath`.

Retorna `NULL`, caso o arquivo não seja encontrado.
*/
char *findPasswdPath(char *htaccessPath);

/*
Dados `authB64` codificados em base64 são: primeiro decodificados e em seguida comparados com as senhas presentes no arquivo de senha apontado
por `resourceInfo.htaccessPath`.
*/
bool authenticateB64(webResource resourceInfo, char *authB64);

/*
Dados `auth` (`usuario:senha`) são comparados com as senhas presentes no arquivo de senha apontado por `resourceInfo.htaccessPath`.

Para isso, dentro da função a senha é encriptada com salt fixo.
*/
bool authenticate(webResource resourceInfo, char *auth);

/*
Encripta os dados de autenticação e grava no buffer passado.


`auth` (user:password) -> `cryptedAuth` (user:crypted(password))
*/
void cryptPassword(char *auth, char *cryptedAuth);

/*
Encripta os dados de autenticação em base64 e grava no buffer passado.

`B64auth` -- decoded --> `auth`
`auth` (user:password) -> `cryptedAuth` (user:crypted(password))
*/
void cryptPasswordB64(char *authB64, char *decodedAuth);

/*
Atualiza a senha no `.htpasswd` referenciado pelo arquivo em
`resourceInfo.htaccessPath`. Para isso, recebe os dados de
`oldAuth` para comparação e `newAuth` para sobrescrita.

Retorna:
- 0, caso a senha tenha sido alterada com sucesso
- -1, as senhas não conferem
- -2, o arquivo passwd não foi encontrado
- -3, problema na manipulação dos arquivos
*/
int updatePassword(webResource resourceInfo, char *oldAuth, char *newAuth);

#endif