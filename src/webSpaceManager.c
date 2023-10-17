#include "../includes/essentials.h"

webResource httpRequest(char response[MAX_BUFFER_SIZE], char *resource, char *reqText)
{
    // Obtém o número da requisição
    http_request req = httpReqText2Number(reqText);

    // Verifica se o recurso existe e é acessível (Atividade 5)
    webResource resourceInfo = checkWebResource(resource);
    if (req == -1)
        resourceInfo.httpCode = HTTP_NOT_IMPLEMENTED;

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

webResource checkWebResource(const char *resource)
{
    webResource resourceInfo;

    // 1. Combinar caminho da web e recurso
    char resourcePath[MAX_PATH_SIZE];
    snprintf(resourcePath, sizeof(resourcePath), "%s%s", WEBSPACE_PATH, resource);

    if (!isSubfile(resourcePath, WEBSPACE_PATH))
    {
        // Arquivo não possui permissão de leitura, pois esta fora do webspace.
        resourceInfo.httpCode = HTTP_FORBIDDEN;
        return resourceInfo;
    }

    // 2. Obtenção do recurso com chamada stat
    struct stat fileStat;
    if (stat(resourcePath, &fileStat) == -1)
    {
        if (errno == ENOENT)
        {
            // Arquivo não encontrado
            resourceInfo.httpCode = HTTP_NOT_FOUND;
            return resourceInfo;
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

void httpRespond(char response[MAX_BUFFER_SIZE], webResource resourceInfo, http_request req)
{
    switch (resourceInfo.httpCode)
    {
    case 200: // OK
        // Requisição atendida
        printHeader(response, resourceInfo.resourcePath, req);
        if (req == HTTP_GET)
        {
            printResource(response, resourceInfo.resourcePath);
        }
        break;
    case 404: // NOT FOUND
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(response, ".", req);
        else
            printErrorHeader(response, resourceInfo.httpCode);
        break;
    case 403: // FORBIDENN
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

void printHeader(char buffer[MAX_BUFFER_SIZE], const char *resourcePath, http_request req)
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
            exit(1);
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

void printErrorHeader(char buffer[MAX_BUFFER_SIZE], http_code code)
{
    // Obter a data atual.
    time_t now;
    time(&now);
    char dateStr[128];
    strftime(dateStr, sizeof(dateStr), "%a %b %d %H:%M:%S %Y BRT", localtime(&now));

    snprintf(buffer, MAX_BUFFER_SIZE,
             "HTTP/1.1 %s\r\n"
             "Date: %s\r\n"
             "Server: JFCM Server 0.1\r\n"
             "Content-Type: text/html\r\n"
             "Connection: %s\r\n"
             "\r\n",
             getHttpStatusText(code), dateStr, "close");
}

void printResource(char response[MAX_BUFFER_SIZE], char *resourcePath)
{
    struct stat fileInfo;
    int fd;
    int bytesRead;
    
    // Lê informações do arquivo
    TRY_ERR( stat(resourcePath, &fileInfo) );

    // Abre o arquivo e lê o seu tamanho
    TRY_ERR( fd = open(resourcePath, O_RDONLY) );

    // Crie um buffer com um tamanho adequado para o arquivo
    char *buffer = (char *)malloc(2*fileInfo.st_size); // Esta linha me custou horas... 
    // LIÇÕES APRENDIDAS: USAR MALLOC NUNCA É TRIVIAL.
    // QUANDO FOR ALOCAR ESPAÇO PARA UM BUFFER, SEMPRE ALOCAR MAIS QUE O NECESSÁRIO.
    // VOCÊ IRÁ PRECISAR......
    if (buffer == NULL)
    {
        perror("Erro ao alocar memória");
        exit(EXIT_FAILURE);
    }

    // Leia o arquivo completo e armazene no buffer
    TRY_ERR( bytesRead = read(fd, buffer, fileInfo.st_size) );
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

int isSubfile(const char *filePath, const char *folderPath)
{
    char actualpath_file[MAX_PATH_SIZE];
    char actualpath_webspace[MAX_PATH_SIZE];
    if (realpath(filePath, actualpath_file) == NULL)
        return -1;
    if (realpath(folderPath, actualpath_webspace) == NULL)
        return -1;

    // Verifica se o caminho do webspace faz parte do caminho do arquivo.
    if (strstr(actualpath_file, actualpath_webspace) != NULL)
        return 1;
    else
        return 0;
}