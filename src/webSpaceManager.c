#include "../includes/essentials.h"

char webspacePath[MAX_PATH_SIZE];

webResource httpRequest(char *response, char *resource, char *reqText, char *auth, char *newCredentials)
{
    webResource resourceInfo;

    // Obtém o número da requisição
    http_request req = httpReqText2Number(reqText);

    if (req == -1)
    {
        resourceInfo.httpCode = HTTP_NOT_IMPLEMENTED;
    }
    else if (req == HTTP_GET)
    {
        // Verifica se o recurso existe e é acessível (Atividade 5)
        resourceInfo = checkWebResource(resource, false);
        if (resourceInfo.httpCode == HTTP_UNAUTHORIZED && auth != NULL)
        {
            bool authenticated;
            authenticated = authenticateB64(resourceInfo, auth);
            if (authenticated)
            {
                resourceInfo = checkWebResource(resource, true);
            }
        }
    }
    else if (req == HTTP_POST)
    {
        // Processar POST

        // Encontrar htacces associado
        snprintf(resourceInfo.resourcePath, sizeof(resourceInfo.resourcePath), "%s%s", webspacePath, resource);
        char *htaccessPath = findHtaccess(resourceInfo.resourcePath);
        if (htaccessPath != NULL)
        {
            strncpy(resourceInfo.htaccessPath, htaccessPath, MAX_PATH_SIZE);
            free(htaccessPath);
            // Atualiza resposta e arquivo de senhas
            postHandler(&resourceInfo, newCredentials);
        }
        else
        {
            resourceInfo.httpCode = HTTP_BAD_REQUEST;
        }
    }

    resourceInfo.bytes = httpRespond(response, resourceInfo, req);

#if LOG
    FILE *logfile = freopen(logFileName, "a", stdout);
    if (logfile == NULL)
    {
        perror("Erro ao abrir o arquivo de saída");
        exit(EXIT_FAILURE);
    }
    printf("RESPOSTA:\n\n");
    if (req == HTTP_GET)
        req = HTTP_HEAD;
    httpRespond(resourceInfo, req);

    fclose(logfile);
#endif
    return resourceInfo;
}

webResource checkWebResource(const char *resource, bool authenticated)
{
    webResource resourceInfo;

    // 1. Combinar caminho da web e recurso
    char resourcePath[MAX_PATH_SIZE];

    // Full Resource Path
    snprintf(resourcePath, sizeof(resourcePath), "%s%s", webspacePath, resource);

    // 1.1 Verificar se é válido
    if (!isValid(resourcePath))
    {
        // Caminhos relativos (com /. ou /..) são proibidos.
        resourceInfo.httpCode = HTTP_FORBIDDEN;
        return resourceInfo;
    }

    // 1.2 Verificar se tem um htacces
    char *htaccesPath = findHtaccess(resourcePath);
    if (htaccesPath != NULL && !authenticated)
    {
        // printf("htaccess encontrado: %s\n", htaccesPath);
        resourceInfo.httpCode = HTTP_UNAUTHORIZED;
        strncpy(resourceInfo.htaccessPath, htaccesPath, MAX_PATH_SIZE + 16);
        free(htaccesPath);
        return resourceInfo;
    }

    // 2. Obtenção do recurso com chamada stat
    struct stat fileStat;
    if (stat(resourcePath, &fileStat) == -1)
    {
        if (errno == ENOENT)
        {
            // Arquivo não encontrado
            char specialPath[MAX_PATH_SIZE] = {0};

            // Verfica se é um arquivo especial (fora do webspace)
            checkSpecialResource(resourcePath, specialPath);
            if (strlen(specialPath) != 0 && htaccesPath != NULL)
            {
                // É um arquivo especial
                resourceInfo.httpCode = HTTP_OK;
                strncpy(resourceInfo.resourcePath, specialPath, MAX_PATH_SIZE);
                return resourceInfo;
            }
            else
            {
                // Arquivo inexistente
                resourceInfo.httpCode = HTTP_NOT_FOUND;
                return resourceInfo;
            }
        }
        else
        {
            // Erro na obtenção de stat
            resourceInfo.httpCode = HTTP_INTERNAL_SERVER_ERROR;
            return resourceInfo;
        }
    }

    // 3. Verifique se o recurso possui permissão de leitura
    if (!(fileStat.st_mode & S_IRUSR))
    {
        // Arquivo não possui permissão de leitura
        resourceInfo.httpCode = HTTP_FORBIDDEN;
        return resourceInfo;
    }

    // 4. Verifique se é um arquivo ou diretório
    if (S_ISREG(fileStat.st_mode))
    {
        // É arquivo regular
        resourceInfo.httpCode = HTTP_OK;
        strcpy(resourceInfo.resourcePath, resourcePath);
        return resourceInfo;
    }
    else if (S_ISDIR(fileStat.st_mode))
    {
        // É diretório

        // 5. Verifique se tem permissão de varredura
        if (!(fileStat.st_mode & S_IXUSR))
        {
            // Não possui opção de varredura
            resourceInfo.httpCode = HTTP_FORBIDDEN;
            return resourceInfo;
        }

        // 6. Verifique se existe index.html ou welcome.html
        char indexPath[MAX_PATH_SIZE + 16];
        snprintf(indexPath, sizeof(indexPath), "%s/index.html", resourcePath);
        char welcomePath[MAX_PATH_SIZE + 16];
        snprintf(welcomePath, sizeof(welcomePath), "%s/welcome.html", resourcePath);

        if (access(indexPath, R_OK) == 0)
        {
            // Encontrou index.html com permissão de leitura
            stat(indexPath, &fileStat);
            resourceInfo.httpCode = HTTP_OK;
            strcpy(resourceInfo.resourcePath, indexPath);
            return resourceInfo;
        }
        else if (access(welcomePath, R_OK) == 0)
        {
            // Encontrou welcome.html com permissão de leitura
            stat(welcomePath, &fileStat);
            resourceInfo.httpCode = HTTP_OK;
            strcpy(resourceInfo.resourcePath, welcomePath);
            return resourceInfo;
        }
        else
        {
            access(welcomePath, R_OK);
            int welcomeFound = !(errno == ENOENT); // Encontrou welcome?
            access(indexPath, R_OK);
            int indexFound = !(errno == ENOENT); // Encontrou index?

            if (welcomeFound || indexFound)
            {
                // Arquivos sem permissão
                resourceInfo.httpCode = HTTP_FORBIDDEN;
                return resourceInfo;
            }
            else
            {
                // Não encontrou nenhum
                resourceInfo.httpCode = HTTP_NOT_FOUND;
                return resourceInfo;
            }
        }
    }
    else
    {
        // Outro tipo de arquivo (não é regular nem diretório)
        resourceInfo.httpCode = HTTP_INTERNAL_SERVER_ERROR;
        return resourceInfo;
    }
}

int httpRespond(char *response, webResource resourceInfo, http_request req)
{
    int responseSize = 0;
    switch (resourceInfo.httpCode)
    {
    case HTTP_OK:
        // Requisição atendida
        printHeader(response, resourceInfo.resourcePath, req);
        responseSize = strlen(response); // Header size
        if (req == HTTP_GET || req == HTTP_POST)
        {
            int resourceSize = printResource(response, resourceInfo.resourcePath);

            responseSize += resourceSize;
        }
        break;
    case HTTP_NOT_FOUND:
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(response, ".", req);
        else
        {
            printErrorHeader(response, resourceInfo.httpCode);
            printResource(response, "web/notfound.html");
        }
        responseSize = strlen(response);
        break;
    case HTTP_FORBIDDEN:
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(response, ".", req);
        else
            printErrorHeader(response, resourceInfo.httpCode);
        responseSize = strlen(response);
        break;
    case HTTP_UNAUTHORIZED:
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(response, ".", req);
        else
            printErrorHeader(response, resourceInfo.httpCode);
        responseSize = strlen(response);
        break;
    default:
        printErrorHeader(response, resourceInfo.httpCode);
        responseSize = strlen(response);
        break;
    }
    return responseSize;
}

void printHeader(char *buffer, char *resourcePath, http_request req)
{
    // Obter a data atual.
    time_t now;
    time(&now);
    char dateStr[128];
    char dateLastModified[128];
    strftime(dateStr, sizeof(dateStr), "%a %b %d %H:%M:%S %Y BRT", localtime(&now));

    // Imprimir o cabeçalho certo para cada requisição
    if (req == HTTP_GET || req == HTTP_HEAD || req == HTTP_POST)
    {
        // Obter informações sobre o arquivo (tamanho e data de modificação).
        struct stat fileInfo;
        if (stat(resourcePath, &fileInfo) == -1)
        {
            perror("Erro ao obter informações do arquivo");
            exit(EXIT_FAILURE);
        }
        strftime(dateLastModified, sizeof(dateLastModified), "%a %b %d %H:%M:%S %Y BRT", localtime(&fileInfo.st_mtime));

        char contentType[64];
        setContentType(resourcePath, contentType);

        snprintf(buffer, MAX_BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "Date: %s\r\n"
                 "Server: JFCM Server 0.1\r\n"
                 "Connection: %s\r\n"
                 "Last-Modified: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Type: %s\r\n"
                 "\r\n",
                 dateStr, "keep-alive", dateLastModified, (long)fileInfo.st_size, contentType);
    }
    else if (req == HTTP_OPTIONS)
    {
        snprintf(buffer, MAX_BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "Allow: OPTIONS, GET, HEAD, TRACE\r\n"
                 "Date: %s\r\n"
                 "Server: JFCM Server 0.1\r\n"
                 "Connection: %s\r\n"
                 "Content-Length: 0\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n",
                 dateStr, "keep-alive");
    }
    else if (req == HTTP_TRACE)
    {
        snprintf(buffer, MAX_BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "Date: %s\r\n"
                 "Server: JFCM Server 0.1\r\n"
                 "Connection: %s\r\n"
                 "Content-Type: text/plain\r\n"
                 "\r\n",
                 dateStr, "keep-alive");
    }
}

void printErrorHeader(char *buffer, http_code code)
{
    // Obter a data atual.
    time_t now;
    time(&now);
    char dateStr[128];
    strftime(dateStr, sizeof(dateStr), "%a %b %d %H:%M:%S %Y BRT", localtime(&now));

    if (code == HTTP_UNAUTHORIZED)
    {
        snprintf(buffer, MAX_BUFFER_SIZE,
                 "HTTP/1.1 %s\r\n"
                 "Date: %s\r\n"
                 "Server: JFCM Server 0.1\r\n"
                 "WWW-Authenticate: Basic realm=\"(realm)\"\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: %s\r\n"
                 "\r\n",
                 getHttpStatusText(code), dateStr, "close");
    }
    else
    {
        snprintf(buffer, MAX_BUFFER_SIZE,
                 "HTTP/1.1 %s\r\n"
                 "Date: %s\r\n"
                 "Server: JFCM Server 0.1\r\n"
                 "Content-Type: text/html\r\n"
                 "Connection: %s\r\n"
                 "\r\n",
                 getHttpStatusText(code), dateStr, "close");
    }
}

int printResource(char *response, char *resourcePath)
{
    struct stat fileInfo;
    FILE *file;
    int bytesRead;

    // Lê informações do arquivo
    TRY_ERR(stat(resourcePath, &fileInfo));

    // Abre o arquivo e lê o seu tamanho
    TRY_NULL(file = fopen(resourcePath, "rb"));

    // Crie um buffer com um tamanho adequado para o arquivo
    char *buffer = (char *)malloc(2 * fileInfo.st_size); // Esta linha me custou horas...
    // LIÇÕES APRENDIDAS: USAR MALLOC NUNCA É TRIVIAL.
    // QUANDO FOR ALOCAR ESPAÇO PARA UM BUFFER, SEMPRE ALOCAR MAIS QUE O NECESSÁRIO.
    // VOCÊ IRÁ PRECISAR......
    if (buffer == NULL)
    {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    // Leia o arquivo completo e armazene no buffer
    TRY_ERR(bytesRead = fread(buffer, 1, fileInfo.st_size, file));
    if (bytesRead == 0)
    {
        printf("Arquivo com tamanho 0");
        exit(EXIT_FAILURE);
    }
    fclose(file);

    // Concatena o header (response) com o conteúdo (buffer)
    // Move o ponteiro para o final do cabeçalho
    char *dataStart = response + strlen(response);
    // Está dando segmentation fault aqui
    TRY_NULL(memcpy(dataStart, buffer, bytesRead));

    // Libere a memória alocada e feche o arquivo
    free(buffer);

    return bytesRead;
}

int isValid(const char *filePath)
{
    if (strstr(filePath, "/..") != NULL || strstr(filePath, "/.") != NULL)
        return 0;
    return 1;
}

void configWebspace()
{
    char cwdPath[MAX_PATH_SIZE];
    // Current Workind Directory
    getcwd(cwdPath, sizeof(cwdPath));
    // Full Web Path
    snprintf(webspacePath, sizeof(webspacePath), "%s%s", cwdPath, WEBSPACE_REL_PATH);
}

char *findHtaccess(char *resourcePath)
{
    // Cria uma cópia do caminho do recurso
    char currentPath[MAX_PATH_SIZE];
    strncpy(currentPath, resourcePath, sizeof(currentPath));

    // printf("Searching...\n");

    // Enquanto estiver dentro do webspace...
    while (strlen(currentPath) >= strlen(webspacePath))
    {
        char htaccessPath[MAX_PATH_SIZE + 16];
        snprintf(htaccessPath, sizeof(htaccessPath), "%s/.htaccess", currentPath);

        // Verifica se o arquivo .htaccess existe no diretório atual
        if (access(htaccessPath, F_OK) != -1)
        {
            char *resultPath = (char *)malloc(MAX_PATH_SIZE);
            strncpy(resultPath, htaccessPath, MAX_PATH_SIZE);
            return resultPath;
        }

        //  Remove o último diretório do caminho
        char *lastSlash = strrchr(currentPath, '/');
        if (lastSlash != NULL)
            *lastSlash = '\0';
        else
            break; // Se não houver mais diretórios, interrompe a busca
    }

    return NULL;
}

void checkSpecialResource(char *resourcePath, char *specialPath)
{
    char *lastSlash = strrchr(resourcePath, '/');
    if (lastSlash != NULL)
    {
        char *specialResource = lastSlash + 1;
        if (!strcmp(specialResource, "forms.html"))
        {
            strncpy(specialPath, "web/forms.html", MAX_PATH_SIZE);
        }
    }
}

void setContentType(char *resourcePath, char *contentType)
{
    char *fileExtension = strrchr(resourcePath, '.');

    if (fileExtension != NULL)
    {
        if (!strcmp(fileExtension, ".html"))
        {
            strcpy(contentType, "text/html");
        }
        else if (!strcmp(fileExtension, ".jpg") || !strcmp(fileExtension, ".jpeg"))
        {
            strcpy(contentType, "image/jpeg");
        }
        else if (!strcmp(fileExtension, ".png"))
        {
            strcpy(contentType, "image/png");
        }
        else if (!strcmp(fileExtension, ".pdf"))
        {
            strcpy(contentType, "application/pdf");
        }
        else if (!strcmp(fileExtension, ".gif"))
        {
            strcpy(contentType, "image/gif");
        }
        else
        {
            strcpy(contentType, "text/plain");
        }
    }
}