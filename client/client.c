#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include "../includes/ioHelper.h"

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

int main(int argc, char **argv)
{
    int soquete;
    struct sockaddr_in destino;
    char msg_ida[2048], msg_volta[2048];
    FILE *file;
    int bytes_read;

    // Uso indevido com comando
    if (argc != 4)
    {
        fprintf(stderr, "Uso: %s <endereço IP> <porta> <http-req-file>\n", argv[0]);
        exit(1);
    }

    // Criando socket TCP
    soquete = socket(AF_INET, SOCK_STREAM, 0);

    // Definindo socket de destino (IP e PORTA)
    // htons e inet_aton são responsáveis por formatar a resposta
    destino.sin_family = AF_INET;
    destino.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1], (struct in_addr *)&destino.sin_addr.s_addr);

    // Cria a requisição HTTP
    fileName2buffer(argv[3], msg_ida);
    printf("\n=============== CONTEÚDO DA REQUISIÇÃO =============== \n\n");
    printf("%s\n", msg_ida);

    // Estabelece a conexão com o destino
    if (connect(soquete, (struct sockaddr *)&destino, sizeof(destino)) < 0)
    {
        perror("Erro ao conectar");
        exit(1);
    }

    // Envia a requisição pelo soquete
    write(soquete, msg_ida, strlen(msg_ida)); // strlen(msg_ida)+1)

    // Abre o arquivo para escrever a resposta
    file = fopen("index.html", "w");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(1);
    }

    // Lê a resposta do servidor e escreve no arquivo
    while ((bytes_read = read(soquete, msg_volta, sizeof(msg_volta))) > 0)
    {
        char *cabecalho;
        char *conteudo;
        char *nova_respostaHTTP;

        nova_respostaHTTP = strdup(msg_volta);
        
        // printf("bytes_read = %d\n", bytes_read);
        // printf("strlen(msg_volta) = %ld\n", strlen(msg_volta));
        // printf("msg_volta[last] = %d\n", msg_volta[strlen(msg_volta)]);

        separarCabecalhoEConteudo(msg_volta, &cabecalho, &conteudo);
        //printf("strlen(conteudo) = %ld\n", strlen(conteudo));

        printf("=============== CONTEÚDO DA RESPOSTA ===============\n\n");
        fwrite(msg_volta, 1, bytes_read, stdout);    // Escreve na saída padrão
        printf("\n\n");
        
        fwrite(conteudo, 1, strlen(conteudo), file); // Escreve no arquivo
        fflush(NULL);

        free(nova_respostaHTTP);
        free(cabecalho);
        free(conteudo);
    }

    fclose(file);
    close(soquete);
}