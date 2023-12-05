#include "../includes/essentials.h"

// Contador de Threads
int workingThreads = 0;
pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;

// Mutex para o parser que faz uso de variáveis globais
pthread_mutex_t parser_mutex = PTHREAD_MUTEX_INITIALIZER;

// Global somente de leitura webspacePath
extern char webspacePath[MAX_PATH_SIZE];

void *threadFunction(void *arg)
{
    // arg é um ponteiro para o socket descriptor
    int newSock = *((int *)arg);

    CHLD_CREATED_TRACE;

    // A conexão não é fechada após primeira resposta
    bool keepalive = true;

    while (keepalive){
        // A função `processConnection()` pode alterar o estado
        // de keepalive
        processConnection(newSock, &keepalive);
    }

    close(newSock);
    free(arg); // Libera o espaço alocado pela thread pai
    
    pthread_mutex_lock(&count_mutex);
    workingThreads--;
    pthread_mutex_unlock(&count_mutex);
    
    CHLD_EXITED_TRACE;
    pthread_exit(NULL);
}

int main(int argc, char **argv)
{
    // Verifica os argumentos
    if (argc != 2)
    {
        fprintf(stderr, "Use: %s <portNum>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int serverSock;
    int newSock;
    struct sockaddr_in cliente;
    socklen_t clientSize = sizeof(cliente);

    // Configurando a variável global `webspacePath`
    configWebspace();

    // Seta o valor da porta (primeiro argumento)
    unsigned short port = (unsigned short)atoi(argv[1]);
    // Cria e dá bind no soquete, retorna socket descriptor
    serverSock = createAndBind(port);

    // Escuta na porta (não tem fila)
    TRY_ERR(listen(serverSock, 0));

    // Sinalização de funcionamento
    SERVER_START_TRACE;

    SERVER_ACCEPTING_TRACE;
    loop
    {
        TRY_ERR(newSock = accept(serverSock, (struct sockaddr *)&cliente, &clientSize));

        // Lock para fazer a leitura do contador de threads
        pthread_mutex_lock(&count_mutex);
        if (workingThreads < MAX_THREADS){
            workingThreads++;
            pthread_mutex_unlock(&count_mutex);

            // Cria uma thread para processar a nova conexão
            pthread_t thread;
            int *arg = malloc(sizeof(int));
            *arg = newSock;
            TRY_ERR(pthread_create(&thread, NULL, threadFunction, arg));
            
            // Finalizou o processamento, reenvia mensagem de espera
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