#include "../includes/essentials.h"

bool authenticate(webResource resourceInfo, char *auth)
{
    bool authenticated = false;
    char decodedAuth[512];

    cryptPassword(auth, decodedAuth);
    if (!strcmp(decodedAuth, "NULL")){
        // Faltou usuário e/ou senha
        return false;
    }

    // Obtém o caminho do passwd
    FILE *htaccessFile = fopen(resourceInfo.htaccessPath, "r");
    if (htaccessFile == NULL)
    {
        perror("Erro ao abrir o arquivo .htaccess");
        return false;
    }

    char htpasswdFileName[100];
    if (fgets(htpasswdFileName, sizeof(htpasswdFileName), htaccessFile) != NULL)
    {
        htpasswdFileName[strcspn(htpasswdFileName, "\n")] = 0; // Remove a quebra de linha
    }
    else
    {
        perror("Erro ao ler o nome do arquivo .htpasswd");
        fclose(htaccessFile);
        return false;
    }

    fclose(htaccessFile);

    // Busca a autenticação no arquivo .passwd
    FILE *htpasswdFile = fopen(htpasswdFileName, "r");

    if (htpasswdFile == NULL)
    {
        perror("Erro ao abrir o arquivo .htpasswd");
        return false;
    }

    // Compara decoded com cada linha do arquivo .htpasswd usando getline()
    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, htpasswdFile)) != -1)
    {
        line[strcspn(line, "\n")] = 0; // Remove a quebra de linha
        if (strcmp(decodedAuth, line) == 0)
        {
            authenticated = true;
            break;
        }
    }

    free(line);
    fclose(htpasswdFile);
    return authenticated;
}


void cryptPassword(char *auth, char *decodedAuth){
// Decodifica base64
    char *decoded = b64_decode(auth, strlen(auth));
    char *authCopy = strdup(decoded);

    // Separa usuário e senha
    char *saveptr;
    char *token = strtok_r(authCopy, ":", &saveptr);
    if (token == NULL){
        snprintf(decodedAuth, 512, "NULL");
        return;
    }

    // Obtém usuário
    char *username = strdup(token);
    
    // Obtém senha
    token = strtok_r(NULL, ":", &saveptr);
    if (token == NULL){
        snprintf(decodedAuth, 512, "NULL");
        return;
    }
    char *password = strdup(token);

    // Criptografa a senha com SHA256 e salt 40
    char *cryptedPassword = crypt(password, "$5$40$");

    snprintf(decodedAuth, 512, "%s:%s", username, cryptedPassword);
    
    free(decoded);
    free(authCopy);
    free(username);
    free(password);
}