#include "../includes/essentials.h"

// GENERATE LOG FILE
#if LOG
char *logFileName;
#endif

int requests = 0;

int main(int argc, char **argv)
{
    struct sockaddr_in cliente;
    int sock, newSock;
    unsigned int clientSize;
    pid_t chld_pid;

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
    printf("\n%s já está aceitando conexões de clientes HTTP em %d.\n\n", argv[0], port);

    loop
    {
        clientSize = sizeof(cliente);

        if (requests < MAX_NUMBER_CHLD)
            printf("%d: Aguardando conexões... %d filho(s) livre(s)\n", getpid(), MAX_NUMBER_CHLD - requests);
        else
            printf("%d: Aguardando conexões... (todos filhos ocupados)\n", getpid());

        if ((newSock = accept(sock, (struct sockaddr *)&cliente, &clientSize)) == -1)
        {
            perror("Erro em accept()");
            exit(EXIT_FAILURE);
        }

        if (requests < MAX_NUMBER_CHLD)
        {
            chld_pid = fork();

            if (chld_pid == 0)
            {
                // Processo filho
                printf("%d: Alguém conectou\n", getpid());
                // Uma vez que a conexão foi aceita, processa a conexão.
                processConnection(newSock);

                shutdown(newSock, SHUT_RDWR);
                printf("%d: Desconectou\n", getpid());
                exit(EXIT_SUCCESS); // Fim do processo filho
            }
            else if (chld_pid > 0)
            {
                requests++;
                close(newSock);
            }
            else
            {
                perror("Erro em fork()");
                exit(EXIT_FAILURE);
            }
        }
        else
        {
            send_response_overload(newSock);
            printf("%d: Mensagem de erro enviada\n", getpid());
            shutdown(newSock, SHUT_RDWR);
        }

    } // END LOOP

    shutdown(sock, SHUT_RDWR);
    yylex_destroy();
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}