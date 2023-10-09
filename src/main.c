#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include "../includes/parser_data_structure.h"
#include "../includes/webspacemanager.h"
#include "../includes/httpHelper.h"
#include "../includes/parser_bison.tab.h"
#include "../includes/main.h"
#include "../includes/ioHelper.h"

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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define MAX_BUFFER_SIZE 1024

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern int yyparse();
extern YY_BUFFER_STATE yy_scan_string(char *str);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

int main(arg_cont, arg_valor)
int arg_cont;
char **arg_valor;
{
    unsigned short porta;
    struct sockaddr_in cliente, servidor;
    int soquete, novo_soquete;
    unsigned int nome_compr;
    char buffer[MAX_BUFFER_SIZE], resposta[MAX_BUFFER_SIZE];

    // Verifica argumentos
    if (arg_cont != 2)
    {
        fprintf(stderr, "Uso: %s <numero_da_porta>\n", arg_valor[0]);
        exit(EXIT_FAILURE);
    }

    // Cria um socket para o servidor e associa a porta passada como argumento.
    porta = (unsigned short)atoi(arg_valor[1]);
    if ((soquete = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("Erro em socket()");
        exit(EXIT_FAILURE);
    }
    servidor.sin_family = AF_INET;
    servidor.sin_port = htons(porta);
    servidor.sin_addr.s_addr = INADDR_ANY;

    // Realiza a bind do soquete
    if (bind(soquete, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
    {
        perror("Erro em bind()");
        exit(EXIT_FAILURE);
    }

    // Aguarda por conexões
    if (listen(soquete, 5) != 0)
    {
        perror("Erro em listen()");
        exit(EXIT_FAILURE);
    }

    // Sinalização de funcionamento
    printf("\n%s já está aceitando conexões de clientes HTTP na porta %d.\n\n", arg_valor[0], porta);
    fflush(stdout);

    while (1)
    {
        LOOP:

        // Aceita pedido de conexao
        nome_compr = sizeof(cliente);
        if ((novo_soquete = accept(soquete, (struct sockaddr *)&cliente, &nome_compr)) == -1)
        {
            perror("Erro em accept()");
            exit(EXIT_FAILURE);
        }

        // Leia a requisição
        ssize_t bytes_received = recv(novo_soquete, buffer, MAX_BUFFER_SIZE, 0);

        if (bytes_received < 0)
        {
            perror("Erro ao receber dados");
            exit(EXIT_FAILURE);
        } else if (bytes_received == 0) {
            // Reinicia a conexao
            goto LOOP;
        }
        // Adicione um terminador nulo ao final da string recebida
        buffer[bytes_received] = '\0';

        // Imprima a requisição recebida
        // printf("bytes_received: %d\n\n", (int)bytes_received);
        //printf("REQUISIÇÃO RECEBIDA:\n\n%s\n", buffer);

        // Se a requisicao for um arquivo com a linha "bye" e uma quebra com \r\n, entao encerra o servidor.
        if (!strcmp("bye\r\n", buffer))
        {
            printf("Fim do servidor.\n");
            exit(EXIT_FAILURE);
        }

        // Passe a requisição para o parser
        cmdList = createCommandList();
        YY_BUFFER_STATE buff = yy_scan_string(buffer);
        yyparse();
        yy_delete_buffer(buff);
        // printCommandList(cmdList);

        // Texto da requisicao e caminho do recurso
        char *reqText = cmdList->head->commandName;
        char *resourcePath = cmdList->head->optionList.head->optionName;

        // Atende a requisição
        FILE *outputFile = freopen("temp.txt", "w", stdout);
        if (outputFile == NULL)
        {
            perror("Erro ao abrir o arquivo de saída");
            exit(EXIT_FAILURE);
        }
        webResource req = httpRequest("./web/meu-webspace", resourcePath, reqText);
        fclose(outputFile);

        // Reseta stdout para o terminal.
        freopen("/dev/tty", "w", stdout);

        // Recupera o conteúdo de temp.txt
        fileName2buffer("temp.txt", resposta);
        remove("temp.txt");

        // Envia a resposta para o cliente
        ssize_t bytes_enviados = write(novo_soquete, resposta, strlen(resposta) + 1); // Envia o byte com a terminação '\0'
        if (bytes_enviados == -1)
        {
            perror("Erro ao enviar a resposta");
            exit(EXIT_FAILURE);
        }

        printf("%s - Mensagem enviada com sucesso (%ld bytes).\n", getHttpStatusText(req.httpCode), bytes_enviados);

        // Fecha a conexão
        shutdown(novo_soquete, 2);

    } // END LOOP

    shutdown(soquete, 2);
    freeCommandList(cmdList);
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