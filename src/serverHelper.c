#include "../includes/essentials.h"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

extern int requests;
CommandList *cmdList;

int processConnection(int newSock)
{
    char request[MAX_BUFFER_SIZE];

    int status = (int)readRequest(newSock, request);
    if (status == 0) return status;

    parseRequest(request);

    webResource req = respondRequest(newSock);

    freeCommandList(cmdList);

    printf("%s - Mensagem enviada com sucesso (%ld bytes).\n", getHttpStatusText(req.httpCode), req.bytes);

    return status;
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
    printf("Aguardando dados...\n");
    // Flush é necessário, pois read é uma chamada bloqueante
    fflush(stdout);

    struct pollfd fds[10];
    memset(fds, 0 , sizeof(fds));
    fds[0].fd = newSock;
    fds[0].events = POLLIN;
    int timeout = SERVER_READ_TIMEOUT_MS;

    int rc = poll(fds, 10, timeout);

    if (rc < 0)
    {
      perror("poll() error");
      exit(EXIT_FAILURE);
    }
    if (rc == 0) {
      fprintf(stderr,"poll() timed out\n");
      exit(EXIT_FAILURE);
    }
    
    ssize_t bytes_received = read(newSock, request, MAX_BUFFER_SIZE);

    if (bytes_received < 0)
    {
        perror("Erro ao receber dados");
        exit(EXIT_FAILURE);
    } else if (bytes_received == 0){
        printf("Leitura com tamanho 0\n");
        return 0;
    }
    printf("Leitura concluída\n");
    request[bytes_received] = '\0';

#if DEBUG
    printf("============================== REQUISIÇÃO RECEBIDA ==============================\n\n%s\n", request);
    printf("bytes_received: %d\n\n", (int)bytes_received);
#endif

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
    
    if (DEBUG){
        printf("============================== PARSER CMD LIST ==============================\n\n");
        printCommandList(cmdList);
        printf("\n\n");
    }
    
    yy_delete_buffer(buff);
}

webResource respondRequest(int newSock)
{
    char response[MAX_BUFFER_SIZE];

    char *requestMethod = cmdList->head->commandName;
    char *resourcePath = cmdList->head->optionList.head->optionName;

    webResource req = httpRequest(response, resourcePath, requestMethod);

    // Envia a response para o cliente
    ssize_t bytes_enviados = write(newSock, response, strlen(response) + 1); // Envia o byte com a terminação '\0'
    if (bytes_enviados == -1)
    {
        perror("Erro ao enviar a response");
        exit(EXIT_FAILURE);
    }
    req.bytes = bytes_enviados;

#if DEBUG
    printf("============================== RESPOSTA ENVIADA ==============================\n\n%s\n", response);
    printf("bytes_enviados: %d\n\n", (int)bytes_enviados);
#endif

    return req;
}

void sigchld_handler(int signo) {
    (void)signo;
    requests--;
    printf("%d: Aguardando conexões... %d filho(s) livre(s)\n", getpid(), MAX_NUMBER_CHLD - requests);
}

void config_signals(void){
    // Define o tratador de sinal para SIGCHLD
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = sigchld_handler;
    
    // Ativa a flag SA_RESTART
    // Nesse caso, apos interrupcao por SIGCHLD, a chamada interrompida é reiniciada.
    sa.sa_flags |= SA_RESTART;

    // Install the signal handler for SIGINT
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }
}

int send_response_overload(int sock){
    // Construindo a resposta
    char htmlContent[MAX_BUFFER_SIZE] = {0};
    
    // Cabeçalho
    printErrorHeader(htmlContent, HTTP_SERVICE_UNAVAILABLE);

    // Corpo
    char resourcePath[MAX_PATH_SIZE];
    snprintf(resourcePath, sizeof(resourcePath), "%s%s", WEBSPACE_PATH, "/../overload.html");
    printResource(htmlContent, resourcePath);

    // Envia a response para o cliente
    ssize_t bytes_enviados = write(sock, htmlContent, strlen(htmlContent) + 1); // Envia o byte com a terminação '\0'
    if (bytes_enviados == -1)
    {
        perror("Erro ao enviar a response");
        exit(EXIT_FAILURE);
    }
    return (int)bytes_enviados;
}