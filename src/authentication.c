#include "../includes/essentials.h"

char *findPasswdPath(char *htaccessPath)
{
    // Obtém o caminho do passwd
    FILE *htaccessFile = fopen(htaccessPath, "r");
    if (htaccessFile == NULL)
    {
        perror("Erro ao abrir o arquivo .htaccess");
        return NULL;
    }

    // Le a primeira linha
    char passwdPath[256];
    if (fgets(passwdPath, sizeof(passwdPath), htaccessFile) != NULL)
    {
        passwdPath[strcspn(passwdPath, "\n")] = 0; // Remove a quebra de linha
    }
    else
    {
        perror("Erro ao ler o nome do arquivo .htpasswd");
        fclose(htaccessFile);
        return NULL;
    }
    fclose(htaccessFile);

    // Aloca espaço e devolve
    char *result = strdup(passwdPath);
    return result;
}

bool authenticate(webResource resourceInfo, char *auth)
{
    bool authenticated = false;
    char decodedAuth[512];

    cryptPassword(auth, decodedAuth);
    if (!strcmp(decodedAuth, "NULL"))
    {
        // Faltou usuário e/ou senha
        return false;
    }

    char *htpasswdFileName = findPasswdPath(resourceInfo.htaccessPath);
    if (htpasswdFileName == NULL)
    {
        return false;
    }

    // Busca a autenticação no arquivo .passwd
    FILE *htpasswdFile = fopen(htpasswdFileName, "r");

    if (htpasswdFile == NULL)
    {
        perror("Erro ao abrir o arquivo .htpasswd");
        free(htpasswdFileName);
        fclose(htpasswdFile);
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
    free(htpasswdFileName);
    fclose(htpasswdFile);
    return authenticated;
}

bool authenticateB64(webResource resourceInfo, char *authB64)
{
    char *decoded = b64_decode(authB64, strlen(authB64));
    bool authenticated = authenticate(resourceInfo, decoded);
    free(decoded);
    return authenticated;
}

void cryptPassword(char *auth, char *cryptedAuth)
{
    char *authCopy = strdup(auth);

    // Separa usuário e senha
    char *saveptr;
    char *token = strtok_r(authCopy, ":", &saveptr);
    if (token == NULL)
    {
        snprintf(cryptedAuth, 512, "NULL");
        return;
    }

    // Obtém usuário
    char *username = strdup(token);

    // Obtém senha
    token = strtok_r(NULL, ":", &saveptr);
    if (token == NULL)
    {
        snprintf(cryptedAuth, 512, "NULL");
        return;
    }
    char *password = strdup(token);

    // Criptografa a senha com salt EA
    char *cryptedPassword = crypt(password, SALT);

    snprintf(cryptedAuth, 512, "%s:%s", username, cryptedPassword);

    free(authCopy);
    free(username);
    free(password);
}

void cryptPasswordB64(char *authB64, char *decodedAuth)
{
    // Decodifica base64
    char *decoded = b64_decode(authB64, strlen(authB64));
    cryptPassword(decoded, decodedAuth);
    free(decoded);
}

int updatePassword(webResource resourceInfo, char *oldAuth, char *newAuth)
{
    // Busca caminho do arquivo de senhas
    char *htpasswdFileName = findPasswdPath(resourceInfo.htaccessPath);
    if (htpasswdFileName == NULL)
    {
        return -3;
    }

    // Abre o arquivo .htpasswd para leitura
    FILE *htpasswdFile = fopen(htpasswdFileName, "r");
    if (htpasswdFile == NULL)
    {
        perror("Erro ao abrir o arquivo .htpasswd");
        free(htpasswdFileName);
        return -2;
    }

    // Cria um arquivo temporário
    char tempFileName[] = "web/temp_htpasswd.txt";
    FILE *tempFile = fopen(tempFileName, "w");
    if (tempFile == NULL)
    {
        perror("Erro ao criar o arquivo temporário de senhas");
        free(htpasswdFileName);
        fclose(htpasswdFile);
        return -2;
    }

    // Compara decoded com cada linha do arquivo .htpasswd usando getline()
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int result = -1;

    while ((read = getline(&line, &len, htpasswdFile)) != -1)
    {
        line[strcspn(line, "\n")] = 0; // Remove a quebra de linha
        if (strcmp(oldAuth, line) == 0)
        {
            // Encontrou a senha antiga, substitui pela nova senha
            fprintf(tempFile, "%s\n", newAuth);
            result = 0;
        }
        else
        {
            // Copia as outras linhas inalteradas
            fprintf(tempFile, "%s\n", line);
        }
    }

    // Remove o arquivo original
    if (remove(htpasswdFileName) < 0 || rename(tempFileName, htpasswdFileName) < 0)
    {
        result = -2;
    }
    
    free(line);
    free(htpasswdFileName);
    fclose(htpasswdFile);
    fclose(tempFile);

    return result;
}
