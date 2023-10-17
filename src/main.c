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

    // Configura o tratamento de sinal para SIGCHLD
    config_signals();

    unsigned short port = (unsigned short)atoi(argv[1]);
    sock = createAndBind(port);

    TRY_ERR( listen(sock, 0) );

    // Sinalização de funcionamento
    SERVER_START_TRACE;

    loop
    {
        clientSize = sizeof(cliente);

        if (requests < MAX_NUMBER_CHLD) SERVER_ACCEPTING_TRACE;
        else SERVER_FULL_TRACE;

        TRY_ERR(
            newSock = accept(sock, (struct sockaddr *)&cliente, &clientSize)
        );

        if (requests < MAX_NUMBER_CHLD)
        {
            TRY_ERR( chld_pid = fork() );

            if (chld_pid == 0)
            {
                // Processo filho
                
                CHLD_CREATED_TRACE;
                // Uma vez que a conexão foi aceita, processa a conexão.
                processConnection(newSock);

                shutdown(newSock, SHUT_RDWR);
                CHLD_EXITED_TRACE;
                exit(EXIT_SUCCESS); // Fim do processo filho
            }
            else if (chld_pid > 0)
            {
                requests++;
                close(newSock);
            }
        }
        else
        {
            send_response_overload(newSock);
            SERVER_OVERLOAD_TRACE;
            shutdown(newSock, SHUT_RDWR);
        }

    } // END LOOP

    shutdown(sock, SHUT_RDWR);
    yylex_destroy();
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}