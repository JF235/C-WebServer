#include "../includes/essentials.h"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

extern int requests;
CommandList *globalCmdList;
// Global que indica o caminho absoluto do Path
extern char webspacePath[PATH_SIZE_MEDIUM];

extern int workingThreads;
extern pthread_mutex_t count_mutex;
extern pthread_mutex_t parser_mutex;

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

int processConnection(int newSock, bool *keepalive)
{
    char request[BUFFER_SIZE_BIG]; // Conteúdo da requisição

    // status pode ser o número de bytes lidos do socket
    int status = (int)readRequest(newSock, request);
    if (status == 0) // Encerrou a conexao com EOF ou deu TIMEOUT
    {
        *keepalive = false;
        return status;
    }

    // Realiza o parsing, variáveis globais envolvidas...
    pthread_mutex_lock(&parser_mutex);
    CommandList *cmdList = parseRequest(request);
    pthread_mutex_unlock(&parser_mutex);

    // Busca o comando Connection para atualizar o estado de
    // keepalive
    Command *cmd = findCommand("Connection", cmdList);
    if (cmd == NULL)
    {
        // Se o comando não estiver presente, false
        *keepalive = false;
    }
    else
    {
        // Se estiver presente, olha para o nome da opção
        char *optionName = cmd->optionList.head->optionName;
        if (!strcmp(optionName, "keep-alive"))
            *keepalive = true;
        else
            *keepalive = false;
    }

    // Vai gerar a resposta 
    // (grande complexidade dentro dessa função)
    webResource req = respondRequest(newSock, cmdList);

    // Fecha a conexão imediatamente, para a requisição não fica pendurada
    if (req.httpCode > 399){
        *keepalive = false;
    }

    freeCommandList(cmdList);

    CHLD_RESP_TRACE;

    return status;
}

ssize_t readRequest(int newSock, char *request)
{
    int numFds;
    ssize_t bytes_received;

    // Configurando o Poll
    struct pollfd fds[10];
    memset(fds, 0, sizeof(fds));
    fds[0].fd = newSock;
    fds[0].events = POLLIN;
    int timeout = SERVER_READ_TIMEOUT_MS;

    CHLD_WAIT_TRACE;

    TRY_ERR(numFds = poll(fds, 10, timeout));

    if (numFds == 0)
    {
        // Tempo expirou, TIMEOUT
        CHLD_TIMEDOUT_TRACE;
        return 0;
    }

    // Lê a requisição no socket e salva no buffer `request`
    TRY_ERR(bytes_received = read(newSock, request, MAX_BUFFER_SIZE));

    CHLD_READ_TRACE;

    if (bytes_received == 0)
    {
        // Leu um EOF
        return 0;
    }
    // Adiciona um caracterece de fim de string no final da leitura.
    request[bytes_received] = '\0';

    // Imprime toda a requisição
    CHLDV_REQ_TRACE;

    return bytes_received;
}

CommandList *parseRequest(char *request)
{
    globalCmdList = createCommandList();
    YY_BUFFER_STATE buff = yy_scan_string(request);
    yyparse();

    PARSER_TRACE;

    // yylex_destroy();
    yy_delete_buffer(buff);
    return globalCmdList;
}

webResource respondRequest(int newSock, CommandList *cmdList)
{
    char *response = (char*)malloc(8*1024*1024); // 8MB
    ssize_t bytes_enviados;

    char *requestMethod = cmdList->head->commandName;
    char *resourcePath = cmdList->head->optionList.head->optionName;

    // Verifica se o campo Authorization com usuario e senha estão presentes
    Command *cmd = findCommand("Authorization", cmdList);
    char *auth;

    if (cmd == NULL)
    {
        auth = NULL;
    }
    else
    {
        auth = cmd->optionList.head->optionName;
        auth = auth + 6; // Remove prefix
    }

    // Ponteiro para último comando (possivelmente o conteúdo de um POST)
    char *newCredentials = cmdList->tail->commandName;

    // Processa a requisição e gera uma resposta armazenada no buffer response
    webResource req = httpRequest(response, resourcePath, requestMethod, auth, newCredentials);

    // Envia a response para o cliente
    // Envia o byte com a terminação '\0'
    TRY_ERR(bytes_enviados = write(newSock, response, req.bytes));
    if (bytes_enviados != req.bytes){
        fprintf(stderr,"Erro na escrita ao socket. Bytes faltando.\n");
    }

    CHLDV_RESP_TRACE;
    free(response);
    return req;
}

int send_response_overload(int sock)
{
    ssize_t bytes_enviados;
    char htmlContent[BUFFER_SIZE_BIG] = {0};

    // Cabeçalho
    printErrorHeader(htmlContent, HTTP_SERVICE_UNAVAILABLE, "web/overload.html");

    // Corpo
    char resourcePath[PATH_SIZE_BIG];
    snprintf(resourcePath, sizeof(resourcePath), "%s%s", webspacePath, "/../overload.html");
    printResource(htmlContent, resourcePath);

    // Envia a response para o cliente
    TRY_ERR(bytes_enviados = write(sock, htmlContent, strlen(htmlContent) + 1)); // Envia o byte com a terminação '\0'

    return (int)bytes_enviados;
}

int parseCredentials(char *credentials, char *delimiters, char *fieldValues[], size_t numFields) {
    char *token, *saveptr;

    for (size_t i = 0; i < 2*numFields; ++i) {
        token = strtok_r((i == 0) ? (char *)credentials : NULL, delimiters, &saveptr);

        if (token == NULL) {
            // Faltaram campos
            return -1;
        }

        if (i%2) // Só pega valores e não labels
            snprintf(fieldValues[i/2], 50, "%s", token);
    }

    return 0;
}

void postHandler(webResource *resourceInfo, char *newCredentials)
{
    int result;

    // Separar as credenciais
    char username[50];
    char oldPass[50];
    char newPass[50];
    char cnewPass[50];
    char confirmar[50];
    char *fieldValues[] = {username, oldPass, newPass, cnewPass, confirmar};
    size_t numFields = sizeof(fieldValues) / sizeof(fieldValues[0]);
    char delimiters[] = "&=";
    result = parseCredentials(newCredentials, delimiters, fieldValues, numFields);

    if (result < 0){
        resourceInfo->httpCode = HTTP_BAD_REQUEST;
        return;
    }

    printf("Tentativa de atualização de dados\n");
    printf("username: %s\n", username);
    printf("oldPass: %s\n", oldPass);
    printf("newPass: %s\n", newPass);
    printf("cnewPass: %s\n", cnewPass);

    // verificar as credenciais
    if (strcmp(newPass, cnewPass))
    {
        resourceInfo->httpCode = HTTP_OK;
        strcpy(resourceInfo->resourcePath, "web/invalidAuth.html");
        return;
    }

    // Autenticar dados
    char oldAuth[512];
    char oldCryptedAuth[512];
    snprintf(oldAuth, 512, "%s:%s", username, oldPass);
    cryptPassword(oldAuth, oldCryptedAuth);

    char newAuth[512];
    char newCryptedAuth[512];
    snprintf(newAuth, 512, "%s:%s", username, newPass);
    cryptPassword(newAuth, newCryptedAuth);

    result = updatePassword(*resourceInfo, oldCryptedAuth, newCryptedAuth);
    if (result == 0)
    {
        // Mudança OK
        resourceInfo->httpCode = HTTP_OK;
        strcpy(resourceInfo->resourcePath, "web/okAuth.html");
    }
    else if (result == -1)
    {
        // Autenticação errada
        resourceInfo->httpCode = HTTP_OK;
        strcpy(resourceInfo->resourcePath, "web/erroAuth.html");
    }
    else if (result == -2 || result == -3)
    {
        // Arquivo de senha não encontrado (-2)
        // Problema com os arquivos de senha (-3)
        resourceInfo->httpCode = HTTP_INTERNAL_SERVER_ERROR;
    }
}