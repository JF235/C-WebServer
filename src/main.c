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
    bool keepalive = true;
    unsigned int clientSize;

    if (argc != 2)
    {
        fprintf(stderr, "Use: %s <portNum>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Config webspacePath
    config_webspace();


    unsigned short port = (unsigned short)atoi(argv[1]);
    sock = createAndBind(port);

    TRY_ERR(listen(sock, 0));

    // Sinalização de funcionamento
    SERVER_START_TRACE;

    loop
    {
        clientSize = sizeof(cliente);

        SERVER_ACCEPTING_TRACE;

        TRY_ERR(newSock = accept(sock, (struct sockaddr *)&cliente, &clientSize));

        CHLD_CREATED_TRACE;

        while (keepalive)
            // Uma vez que a conexão foi aceita, processa a conexão.
            processConnection(newSock, &keepalive);
        keepalive=true;
        
        close(newSock);

    } // END LOOP

    while (wait(NULL) > 0)
        ; // Wait for child
    shutdown(sock, SHUT_RDWR);
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}