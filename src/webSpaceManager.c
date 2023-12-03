#include "../includes/essentials.h"

char webspacePath[512];

webResource httpRequest(char *response, char *resource, char *reqText, char *auth)
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
            authenticated = authenticate(resourceInfo, auth);
            if (authenticated)
            {
                resourceInfo = checkWebResource(resource, true);
            }
        }
    }
    else if (req == HTTP_POST)
    {
        resourceInfo.httpCode = HTTP_NOT_IMPLEMENTED;
    }

    httpRespond(response, resourceInfo, req);

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
        // Caminhos relativos são proibidos.
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
            if (strlen(specialPath) != 0 && htaccesPath != NULL) {
                // É um arquivo especial
                resourceInfo.httpCode = HTTP_OK;
                strncpy(resourceInfo.resourcePath, specialPath, MAX_PATH_SIZE);
                return resourceInfo;
            } else {
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

void httpRespond(char *response, webResource resourceInfo, http_request req)
{
    switch (resourceInfo.httpCode)
    {
    case HTTP_OK:
        // Requisição atendida
        printHeader(response, resourceInfo.resourcePath, req);
        if (req == HTTP_GET)
        {
            printResource(response, resourceInfo.resourcePath);
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
        break;
    case HTTP_FORBIDDEN:
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(response, ".", req);
        else
            printErrorHeader(response, resourceInfo.httpCode);
        break;
    case HTTP_UNAUTHORIZED:
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(response, ".", req);
        else
            printErrorHeader(response, resourceInfo.httpCode);
        break;
    default:
        printErrorHeader(response, resourceInfo.httpCode);
        break;
    }
}

void printHeader(char *buffer, const char *resourcePath, http_request req)
{
    // Obter a data atual.
    time_t now;
    time(&now);
    char dateStr[128];
    char dateLastModified[128];
    strftime(dateStr, sizeof(dateStr), "%a %b %d %H:%M:%S %Y BRT", localtime(&now));

    // Imprimir o cabeçalho certo para cada requisição
    if (req == HTTP_GET || req == HTTP_HEAD)
    {
        // Obter informações sobre o arquivo (tamanho e data de modificação).
        struct stat fileInfo;
        if (stat(resourcePath, &fileInfo) == -1)
        {
            perror("Erro ao obter informações do arquivo");
            exit(EXIT_FAILURE);
        }
        strftime(dateLastModified, sizeof(dateLastModified), "%a %b %d %H:%M:%S %Y BRT", localtime(&fileInfo.st_mtime));

        snprintf(buffer, MAX_BUFFER_SIZE,
                 "HTTP/1.1 200 OK\r\n"
                 "Date: %s\r\n"
                 "Server: JFCM Server 0.1\r\n"
                 "Connection: %s\r\n"
                 "Last-Modified: %s\r\n"
                 "Content-Length: %ld\r\n"
                 "Content-Type: text/html\r\n"
                 "\r\n",
                 dateStr, "keep-alive", dateLastModified, (long)fileInfo.st_size);
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

void printResource(char *response, char *resourcePath)
{
    struct stat fileInfo;
    int fd;
    int bytesRead;

    // Lê informações do arquivo
    TRY_ERR(stat(resourcePath, &fileInfo));

    // Abre o arquivo e lê o seu tamanho
    TRY_ERR(fd = open(resourcePath, O_RDONLY));

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
    TRY_ERR(bytesRead = read(fd, buffer, fileInfo.st_size));
    if (bytesRead == 0)
    {
        printf("Arquivo com tamanho 0");
        exit(EXIT_FAILURE);
    }
    buffer[bytesRead] = '\0';
    close(fd);

    // Concatena o header (response) com o conteúdo (buffer)
    strcat(response, buffer);

    // Libere a memória alocada e feche o arquivo
    free(buffer);
}

int isValid(const char *filePath)
{
    if (strstr(filePath, "/..") != NULL || strstr(filePath, "/.") != NULL)
        return 0;
    return 1;
}

void config_webspace()
{
    char cwdPath[256];
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
        // printf("%s ", htaccessPath);

        // Verifica se o arquivo .htaccess existe no diretório atual
        if (access(htaccessPath, F_OK) != -1)
        {
            char *resultPath = (char *)malloc(MAX_PATH_SIZE);
            strncpy(resultPath, htaccessPath, MAX_PATH_SIZE);
            // printf("encontrado\n");
            return resultPath;
        }

        // printf("não encontrado\n");
        //  Remove o último diretório do caminho
        char *lastSlash = strrchr(currentPath, '/');
        if (lastSlash != NULL)
            *lastSlash = '\0';
        else
            break; // Se não houver mais diretórios, interrompe a busca
    }

    return NULL;
}


void checkSpecialResource(char *resourcePath, char *specialPath){
    char *lastSlash = strrchr(resourcePath, '/');
    if (lastSlash != NULL){
        char *specialResource = lastSlash + 1;
        if (!strcmp(specialResource, "forms.html")){
            strncpy(specialPath, "web/forms.html", MAX_PATH_SIZE);
        }
    }
}