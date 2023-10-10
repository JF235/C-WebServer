#include "../includes/essentials.h"

/*
    GENERATE LOG FILE
*/
#define LOG 0
#if LOG
char *logFileName;
#endif

#define NETWORK 1
#if NETWORK
/*
    NETWORK SERVER
*/




int main(int argc, char **argv)
{
    struct sockaddr_in cliente;
    int sock, newSock;
    unsigned int nome_compr;
    

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
    fflush(stdout);

    while (1)
    {
        // Aceita pedido de conexao
        nome_compr = sizeof(cliente);
        if ((newSock = accept(sock, (struct sockaddr *)&cliente, &nome_compr)) == -1)
        {
            perror("Erro em accept()");
            exit(EXIT_FAILURE);
        }

        processConnection(newSock);
        
        // Fecha a conexão
        shutdown(newSock, SHUT_RDWR);

    } // END LOOP

    shutdown(sock, SHUT_RDWR);
    yylex_destroy();
    printf("O servidor terminou com erro.\n");
    exit(EXIT_FAILURE);
}

#else
/*
    LOCAL SERVER
*/

int main(int argc, char *argv[])
{
    FILE *arquivo_requisicao;

    // Verifica a entrada
    if (argc == 5)
    {
        // Abre o arquivo de requisição para leitura.
        arquivo_requisicao = fopen(argv[2], "r");
        if (arquivo_requisicao == NULL)
        {
            perror("Erro ao abrir o arquivo de requisicao.");
            exit(EXIT_FAILURE);
        }

        file2stdout(arquivo_requisicao);

#if LOG
        // Imprime a entrada no log.
        logFileName = argv[4];
        FILE *logfile = freopen(logFileName, "a", stdout);
        if (logfile == NULL)
        {
            perror("Erro ao abrir o arquivo de saída");
            exit(EXIT_FAILURE);
        }
        printf("REQUISICAO:\n\n");
        while ((caractere = fgetc(arquivo_requisicao)) != EOF)
            printf("%c", caractere);

        fclose(logfile);
        fseek(arquivo_requisicao, 0, SEEK_SET);
#endif
    }
    else
    {
        fprintf(stderr, "./servidor <webspace> req_N.txt resp_N.txt registro.txt\n");
        exit(EXIT_FAILURE);
    }

    // Realiza o parse e gera a estrutura de dados
    cmdList = createCommandList();
    yyin = arquivo_requisicao;
    yyparse();
    fclose(arquivo_requisicao);
    // printCommandList(cmdList);

    // Texto da requisicao e Caminho do recurso
    char *reqText = cmdList->head->commandName;
    char *resourcePath = cmdList->head->optionList.head->optionName;

    // Atende a requisicao

    // Redireciona a saída
    // Cria o arquivo para escrita (ou trunca se já existir).
    FILE *outputFile = freopen(argv[3], "w", stdout);
    if (outputFile == NULL)
    {
        perror("Erro ao abrir o arquivo de saída");
        exit(EXIT_FAILURE);
    }
    webResource req = httpRequest(argv[1], resourcePath, reqText);
    fclose(outputFile);

    freeCommandList(cmdList);
    yylex_destroy();
    return 0;
}
#endif