#include "../includes/essentials.h"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

CommandList *cmdList;

int processConnection(int newSock)
{
    char request[MAX_BUFFER_SIZE];

    readRequest(newSock, request);

    parseRequest(request);
    
    webResource req = respondRequest(newSock);
    
    freeCommandList(cmdList);
    
    printf("%s - Mensagem enviada com sucesso (%ld bytes).\n", getHttpStatusText(req.httpCode), req.bytes);
}

int createAndBind(unsigned short port)
{
    struct sockaddr_in server_addr;
    int sock;

    // Cria um socket para o servidor
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Error in socket()");
        exit(EXIT_FAILURE);
    }

    // Atribui o endereço do socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Realiza a bind do soquete com o seu endereço
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Error in bind()");
        exit(EXIT_FAILURE);
    }

    return sock;
}

ssize_t readRequest(int newSock, char *request)
{
    ssize_t bytes_received = read(newSock, request, MAX_BUFFER_SIZE);

    if (bytes_received <= 0)
    {
        if (bytes_received == 0)
            printf("b=0\n");
        perror("Erro ao receber dados");
        exit(EXIT_FAILURE);
    }
    request[bytes_received] = '\0';

    // DEBUG
    // printf("bytes_received: %d\n\n", (int)bytes_received);
    // printf("REQUISIÇÃO RECEBIDA:\n\n%s\n", request);

    // Se a requisicao for um arquivo com a linha "bye" e uma quebra com \r\n, entao encerra o servidor.
    if (!strcmp("bye\r\n", request))
    {
        printf("Fim do servidor.\n");
        exit(EXIT_SUCCESS);
    }

    return bytes_received;
}

void parseRequest(char *request)
{
    cmdList = createCommandList();
    YY_BUFFER_STATE buff = yy_scan_string(request);
    yyparse();
    yy_delete_buffer(buff);
}

webResource respondRequest(int newSock){
    char response[MAX_BUFFER_SIZE];
    
    char *requestMethod = cmdList->head->commandName;
    char *resourcePath = cmdList->head->optionList.head->optionName;

    // Redirects stdout to temp.txt (BAD!!!)
    FILE *outputFile = freopen("temp.txt", "w", stdout);
    if (outputFile == NULL)
    {
        perror("Erro ao abrir o arquivo de saída");
        exit(EXIT_FAILURE);
    }
    webResource req = httpRequest("./web/meu-webspace", resourcePath, requestMethod);
    fclose(outputFile);

    // Reseta stdout para o terminal.
    freopen("/dev/tty", "w", stdout);

    // Recupera o conteúdo de temp.txt
    fileName2buffer("temp.txt", response);
    remove("temp.txt");

    // Envia a response para o cliente
    ssize_t bytes_enviados = write(newSock, response, strlen(response) + 1); // Envia o byte com a terminação '\0'
    if (bytes_enviados == -1)
    {
        perror("Erro ao enviar a response");
        exit(EXIT_FAILURE);
    }
    req.bytes = bytes_enviados;

    return req;
}

