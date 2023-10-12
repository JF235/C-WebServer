#include "../includes/essentials.h"

// GENERATE LOG FILE
#if LOG
char *logFileName;
#endif

int main(int argc, char **argv)
{
    struct sockaddr_in cliente;
    int sock, newSock;
    unsigned int clientSize;

    if (argc != 2)
    {
        fprintf(stderr, "Use: %s <portNum>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    config_signals();

    unsigned short port = (unsigned short)atoi(argv[1]);
    sock = createAndBind(port);

    if (listen(sock, 0) != 0)
    {
        perror("Error in listen()");
        exit(EXIT_FAILURE);
    }

    // Sinalização de funcionamento
    printf("\n%s já está aceitando conexões de clientes HTTP na porta %d.\n\n", argv[0], port);

    loop
    {
        clientSize = sizeof(cliente);
        
        printf("Aguardando conexões...\n");
        if ((newSock = accept(sock, (struct sockaddr *)&cliente, &clientSize)) == -1)
        {
            perror("Erro em accept()");
            exit(EXIT_FAILURE);
        }
        printf("Alguém conectou (%d)\n", newSock);

        // Uma vez que a conexão foi aceita, processa a conexão.
        processConnection(newSock);

        shutdown(newSock, SHUT_RDWR);
        printf("Desconectou (%d)\n", newSock);

    } // END LOOP

    shutdown(sock, SHUT_RDWR);
    yylex_destroy();
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}