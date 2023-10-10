#include "../includes/essentials.h"

// GENERATE LOG FILE
#if LOG
char *logFileName;
#endif

#define loop while (1)

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

    unsigned short port = (unsigned short)atoi(argv[1]);
    sock = createAndBind(port);

    if (listen(sock, 5) != 0)
    {
        perror("Error in listen()");
        exit(EXIT_FAILURE);
    }

    // Sinalização de funcionamento
    printf("\n%s já está aceitando conexões de clientes HTTP na porta %d.\n\n", argv[0], port);

    loop
    {
        clientSize = sizeof(cliente);
        if ((newSock = accept(sock, (struct sockaddr *)&cliente, &clientSize)) == -1)
        {
            perror("Erro em accept()");
            exit(EXIT_FAILURE);
        }

        // Uma vez que a conexão foi aceita, processa a conexão.
        processConnection(newSock);

        shutdown(newSock, SHUT_RDWR);

    } // END LOOP

    shutdown(sock, SHUT_RDWR);
    yylex_destroy();
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}