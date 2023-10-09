#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h>
#include "../includes/webspacemanager.h"
#include "../includes/httpHelper.h"

webResource httpRequest(char *webPath, char *resource, char *reqText)
{
    // Obtém o número da requisição
    http_request req = httpReqText2Number(reqText);

    // Verifica se o recurso existe e é acessível (Atividade 5)
    webResource resourceInfo = checkWebResource(webPath, resource);
    if (req == -1){
        resourceInfo.httpCode = HTTP_NOT_IMPLEMENTED;
    }

    // Imprime a resposta http no outputFileName.
    httpRespond(resourceInfo, req);

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

webResource checkWebResource(const char *webPath, const char *resource)
{
    webResource resourceInfo;

    // 1. Combinar caminho da web e recurso
    char resourcePath[MAX_PATH_SIZE];
    snprintf(resourcePath, sizeof(resourcePath), "%s%s", webPath, resource);

    if (!isSubfile(resourcePath, webPath)){
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

void httpRespond(webResource resourceInfo, http_request req)
{
    switch (resourceInfo.httpCode)
    {
    case 200: // OK
        // Requisição atendida
        printHeader(resourceInfo.httpCode, resourceInfo.resourcePath, req);
        if (req == HTTP_GET)
        {
            fseek(stdout, -1, SEEK_SET); // Garante que o conteúdo será colocado ao final do arquivo.
            printResource(resourceInfo.resourcePath);
        }
        break;
    case 404: // NOT FOUND
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(resourceInfo.httpCode, ".", req);
        else
            printErrorHeader(resourceInfo.httpCode);
        break;
    case 403: // FORBIDENN
        if (req == HTTP_TRACE || req == HTTP_OPTIONS)
            printHeader(resourceInfo.httpCode, ".", req);
        else
            printErrorHeader(resourceInfo.httpCode);
        break;
    default:
        printErrorHeader(resourceInfo.httpCode);
        break;
    }
}

void printHeader(http_code code, const char *resourcePath, http_request req)
{
    // Obter a data atual.
    time_t now;
    time(&now);
    char dateStr[128];
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

        printf("HTTP/1.1 200 OK\r\n");
        printf("Date: %s\r\n", dateStr);
        printf("Server: JFCM Server 0.1\r\n");
        printf("Connection: %s\r\n", "keep-alive");
        strftime(dateStr, sizeof(dateStr), "%a %b %d %H:%M:%S %Y BRT", localtime(&fileInfo.st_mtime));
        printf("Last-Modified: %s\r\n", dateStr);
        printf("Content-Length: %ld\r\n", (long)fileInfo.st_size);
        printf("Content-Type: text/html\r\n");
        printf("\r\n");
    }
    else if (req == HTTP_OPTIONS)
    {
        printf("HTTP/1.1 200 OK\r\n");
        printf("Allow: OPTIONS, GET, HEAD, TRACE\r\n");
        printf("Date: %s\r\n", dateStr);
        printf("Server: JFCM Server 0.1\r\n");
        printf("Connection: %s\r\n", "keep-alive");
        printf("Content-Length: 0\r\n");
        printf("Content-Type: text/plain\r\n");
        printf("\r\n");
    }
    else if (req == HTTP_TRACE)
    {
        printf("HTTP/1.1 200 OK\r\n");
        printf("Date: %s\r\n", dateStr);
        printf("Server: JFCM Server 0.1\r\n");
        printf("Connection: %s\r\n", "keep-alive");
        printf("Content-Type: message/http\r\n");
        printf("\r\n");
    }
}

void printErrorHeader(http_code code)
{
    // Obter a data atual.
    time_t now;
    time(&now);
    char dateStr[128];
    strftime(dateStr, sizeof(dateStr), "%a %b %d %H:%M:%S %Y BRT", localtime(&now));

    printf("HTTP/1.1 %s\r\n", getHttpStatusText(code));
    printf("Date: %s\r\n", dateStr);
    printf("Server: JFCM Server 0.1\r\n");
    printf("Content-Type: text/html\r\n");
    printf("Connection: %s\r\n", "close");
    printf("\r\n");
}

void printResource(const char *resourcePath)
{
    struct stat fileInfo;
    if (stat(resourcePath, &fileInfo) == -1)
    {
        perror("Erro ao obter informações do arquivo");
        exit(1);
    }

    // Abre o arquivo no modo leitura
    int fd = open(resourcePath, O_RDONLY);
    if (fd == -1)
    {
        perror("Erro ao abrir arquivo");
        exit(1);
    }

    // Crie um buffer com um tamanho adequado para o arquivo
    char *buffer = (char *)malloc(fileInfo.st_size);
    if (buffer == NULL)
    {
        perror("Erro ao alocar memória");
        exit(1);
    }

    // Leia o arquivo completo e armazene no buffer
    ssize_t totalBytesRead = 0;
    ssize_t bytesRead;
    while (totalBytesRead < fileInfo.st_size)
    {
        bytesRead = read(fd, buffer + totalBytesRead, fileInfo.st_size - totalBytesRead);
        if (bytesRead == -1)
        {
            perror("Erro ao ler arquivo");
            free(buffer);
            close(fd);
            exit(1);
        }
        totalBytesRead += bytesRead;
    }

    // Escreva o conteúdo do arquivo na saída padrão
    write(STDOUT_FILENO, buffer, fileInfo.st_size);

    // Libere a memória alocada e feche o arquivo
    free(buffer);
    close(fd);
}

int isSubfile(const char *filePath, const char *folderPath) {
    char actualpath_file[1024];
    char actualpath_webspace[1024];
    if (realpath(filePath, actualpath_file) == NULL){
        return -1;
    }
    if (realpath(folderPath, actualpath_webspace) == NULL){
        return -1;
    }

    // Verifica se o caminho do webspace faz parte do caminho do arquivo. 
    if (strstr(actualpath_file, actualpath_webspace) != NULL){
        return 1;
    } else {
        return 0;
    }
}