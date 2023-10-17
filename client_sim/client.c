#include "client.h"

void separarCabecalhoEConteudo(const char *respostaHTTP, char **cabecalho, char **conteudo)
{
    // Procurar a primeira ocorrência de "\r\n\r\n" para separar o cabeçalho do conteúdo
    char *cabecalhoFim = strstr(respostaHTTP, "\r\n\r\n");

    if (cabecalhoFim != NULL)
    {
        // Calcular o tamanho do cabeçalho
        size_t tamanhoCabecalho = cabecalhoFim - respostaHTTP + 4; // +4 para incluir "\r\n\r\n"

        // Alocar memória para o cabeçalho e copiá-lo
        *cabecalho = (char *)malloc(tamanhoCabecalho + 1); // +1 para o caractere nulo
        strncpy(*cabecalho, respostaHTTP, tamanhoCabecalho);
        (*cabecalho)[tamanhoCabecalho] = '\0';

        // O conteúdo começa logo após o cabeçalho
        *conteudo = strdup(cabecalhoFim + 4); // +4 para pular "\r\n\r\n"
    }
    else
    {
        // Se não encontrar o delimitador, definir o cabeçalho como a resposta inteira
        *cabecalho = strdup(respostaHTTP);
        *conteudo = NULL;
    }
}

int connectToServer(char *serverIp, char *port)
{
    struct sockaddr_in server;

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);

    // Server internet addres (ip and port)
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    inet_aton("0", (struct in_addr *)&server.sin_addr.s_addr);
    // BUG MEDONHO AQUI !!!
    // Preciso escrever esse 0 no punho.
    // printf("server.sin_port %d\n", server.sin_port);
    // printf("server.sin_addr.s_addr %d\n", server.sin_addr.s_addr);

    int synRetries = 2; // Send a total of 3 SYN packets => Timeout ~7s
    setsockopt(serverSock, IPPROTO_TCP, TCP_SYNCNT, &synRetries, sizeof(synRetries));

    printf("Conectando em %s:%s\n", serverIp, port);
    if (connect(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect() error");
        exit(EXIT_FAILURE);
    }

    printf("Conexão concluída (sock = %d)\n", serverSock);

    return serverSock;
}

void sendRequest(int serverSock, char *request)
{
    printf("\n=============== CONTEÚDO DA REQUISIÇÃO =============== \n\n");
    printf("%s\n", request);

    // Envia a requisição pelo serverSock
    write(serverSock, request, strlen(request)); // strlen(request)+1)
}

int receiveResponse(int serverSock)
{
    char response[MAX_BUFFER_SIZE];
    //FILE *file;

    // file = fopen("index.html", "w");
    // if (file == NULL)
    // {
    //     perror("Erro ao abrir o arquivo");
    //     exit(1);
    // }

    int bytesRead;
    int totalBytes = 0;
    while ((bytesRead = read(serverSock, response, sizeof(response))) > 0)
    {
        char *header;
        char *body;
        char *newResponse;
        totalBytes = bytesRead;

        newResponse = strdup(response);

        // printf("bytesRead = %d\n", bytesRead);
        // printf("strlen(response) = %ld\n", strlen(response));
        // printf("response[last] = %d\n", response[strlen(response)]);

        separarCabecalhoEConteudo(response, &header, &body);
        // printf("strlen(body) = %ld\n", strlen(body));

        printf("=============== CONTEÚDO DA RESPOSTA ===============\n\n");
        fwrite(response, 1, bytesRead, stdout); // Escreve na saída padrão
        printf("\n\n");

        //fwrite(body, 1, strlen(body), file); // Escreve no arquivo
        fflush(NULL);

        free(newResponse);
        free(header);
        free(body);
    }
    //fclose(file);
    return totalBytes;
}

// 1 is valid, 0 isn't
int checkFD(int fd)
{
    return fcntl(fd, F_GETFL) != -1 || errno != EBADF;
}

int main(int argc, char **argv)
{
    int serverSock;
    char request[MAX_BUFFER_SIZE] = {0};
    char reqFilePath[MAX_PATH_SIZE];
    char buffer[MAX_PATH_SIZE];

    // Uso indevido com comando
    if (argc != 3)
    {
        fprintf(stderr, "Use: %s <endereço IP> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char comando[32];
    printf("Enter command:\n");
    loop
    {
        scanf("%s", comando);
        if (!strcmp(comando, "connect"))
        {
            serverSock = connectToServer(argv[1], argv[2]);

            if (!checkFD(serverSock))
            {
                fprintf(stderr, "Problema ao conectar com o servidor\n");
                exit(EXIT_FAILURE);
            }

            // Obtém o caminho da requisição que será feita
            strcpy(reqFilePath, REQS_PATH);

            printf("Digite o comando:\n");
            scanf("%s", buffer);
            if (!strcmp(reqFilePath, "bye"))
            {
                printf("bye\n");
                close(serverSock);
                exit(EXIT_SUCCESS);
            }
            else
            {
                strcat(reqFilePath, "/");
                strcat(reqFilePath, buffer);
                strcat(reqFilePath, ".txt");
            }
            // Obtém o conteúdo do arquivo
            fileName2buffer(reqFilePath, request);

            // Envia a requisicao
            sendRequest(serverSock, request);

            // Recebe a requisicao e coloca em um arquivo
            int totalBytes = receiveResponse(serverSock);
            if (totalBytes == 0) printf("Servidor ocupado. Desconectando.\n");

            close(serverSock);
        }
        else if (!strcmp(comando, "quit"))
        {
            exit(EXIT_SUCCESS);
        }
    }
}