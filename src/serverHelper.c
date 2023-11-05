#include "../includes/essentials.h"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

extern int requests;
CommandList *cmdList;
extern char webspacePath[256];

int processConnection(int newSock, bool *keepalive)
{
    char request[MAX_BUFFER_SIZE];

    int status = (int)readRequest(newSock, request);
    if (status == 0){
        *keepalive = false; // Encerrou a conexao com EOF
        return status;
    }

    parseRequest(request);

    Command *cmd = findCommand("Connection", cmdList);
    char *optionName = cmd->optionList.head->optionName;
    if (!strcmp(optionName, "keep-alive"))
        *keepalive = true;
    else
        *keepalive = false;

    webResource req = respondRequest(newSock);

    freeCommandList(cmdList);

    CHLD_RESP_TRACE;

    return status;
}

int createAndBind(unsigned short port)
{
    struct sockaddr_in server_addr;
    int sock;

    // Cria um socket para o servidor
    TRY_ERR(sock = socket(AF_INET, SOCK_STREAM, 0));

    // Atribui o endereço do socket
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    TRY_ERR(bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)));

    return sock;
}

ssize_t readRequest(int newSock, char *request)
{
    int numFds;
    ssize_t bytes_received;

    CHLD_WAIT_TRACE;

    struct pollfd fds[10];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = newSock;
    fds[0].events = POLLIN;
    int timeout = SERVER_READ_TIMEOUT_MS;

    TRY_ERR(numFds = poll(fds, 10, timeout));

    if (numFds == 0)
    {
        shutdown(newSock, SHUT_RDWR);
        CHLD_TIMEDOUT_TRACE;
        CHLD_EXITED_TRACE;
        exit(EXIT_SUCCESS); // Fim do processo filho
    }

    TRY_ERR(bytes_received = read(newSock, request, MAX_BUFFER_SIZE));

    CHLD_READ_TRACE;

    if (bytes_received == 0)
    {
        return 0;
    }
    request[bytes_received] = '\0';

    CHLDV_REQ_TRACE;

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

    PARSER_TRACE;

    // yylex_destroy();
    yy_delete_buffer(buff);
}

webResource respondRequest(int newSock)
{
    char response[MAX_BUFFER_SIZE];
    ssize_t bytes_enviados;

    char *requestMethod = cmdList->head->commandName;
    char *resourcePath = cmdList->head->optionList.head->optionName;

    webResource req = httpRequest(response, resourcePath, requestMethod);

    // Envia a response para o cliente
    // Envia o byte com a terminação '\0'
    TRY_ERR(bytes_enviados = write(newSock, response, strlen(response) + 1));

    req.bytes = bytes_enviados;

    CHLDV_RESP_TRACE;

    return req;
}

void sigchld_handler(int signo)
{
    (void)signo;
    requests--;
    SERVER_ACCEPTING_TRACE;
}

void config_signals(void)
{
    // Define o tratador de sinal para SIGCHLD
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;

    // Ativa a flag SA_RESTART
    // Nesse caso, apos interrupcao por SIGCHLD, a chamada interrompida é reiniciada.
    sa.sa_flags |= SA_RESTART;

    // Install the signal handler for SIGINT
    TRY_ERR(sigaction(SIGCHLD, &sa, NULL));
}

int send_response_overload(int sock)
{
    ssize_t bytes_enviados;
    char htmlContent[MAX_BUFFER_SIZE] = {0};

    // Cabeçalho
    printErrorHeader(htmlContent, HTTP_SERVICE_UNAVAILABLE);

    // Corpo
    char resourcePath[MAX_PATH_SIZE];
    snprintf(resourcePath, sizeof(resourcePath), "%s%s", webspacePath, "/../overload.html");
    printResource(htmlContent, resourcePath);

    // Envia a response para o cliente
    TRY_ERR(bytes_enviados = write(sock, htmlContent, strlen(htmlContent) + 1)); // Envia o byte com a terminação '\0'

    return (int)bytes_enviados;
}