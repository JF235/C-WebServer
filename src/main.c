#include "../includes/essentials.h"

// GENERATE LOG FILE
#if LOG
char *logFileName;
#endif

//============================================
//============================================
// SERVIDOR MULTITHREADED
//============================================
//============================================

int workingThreads = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t parser_mutex = PTHREAD_MUTEX_INITIALIZER;

void *threadFunction(void *arg)
{
    int newSock = *((int *)arg);
    bool keepalive = true;

    CHLD_CREATED_TRACE;

    while (keepalive){
        processConnection(newSock, &keepalive);
    }

    close(newSock);
    free(arg);
    
    pthread_mutex_lock(&count_mutex);
    workingThreads--;
    pthread_mutex_unlock(&count_mutex);
    
    CHLD_EXITED_TRACE;
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    struct sockaddr_in cliente;
    socklen_t clientSize = sizeof(cliente);

    int newSock;
    int serverSock;

    if (argc != 2)
    {
        fprintf(stderr, "Use: %s <portNum>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Config webspacePath
    config_webspace();

    unsigned short port = (unsigned short)atoi(argv[1]);
    serverSock = createAndBind(port);

    TRY_ERR(listen(serverSock, 0));

    // Sinalização de funcionamento
    SERVER_START_TRACE;
    SERVER_ACCEPTING_TRACE;

    loop
    {
        TRY_ERR(newSock = accept(serverSock, (struct sockaddr *)&cliente, &clientSize));

        // Lock para fazer a leitura
        pthread_mutex_lock(&count_mutex);
        if (workingThreads < MAX_THREADS){
            workingThreads++;
            pthread_mutex_unlock(&count_mutex);

            pthread_t thread;
            int *arg = malloc(sizeof(int));
            *arg = newSock;
            TRY_ERR(pthread_create(&thread, NULL, threadFunction, arg));
            SERVER_ACCEPTING_TRACE;
        }
        else{
            pthread_mutex_unlock(&count_mutex);

            send_response_overload(newSock);
            SERVER_OVERLOAD_TRACE;

            close(newSock);
        }

    } // END LOOP

    close(serverSock);
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}