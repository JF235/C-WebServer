#include <sys/socket.h>
#include <string.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/tcp.h>

int connectToServer(char *serverIp, char *port)
{
    struct sockaddr_in server;

    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    printf("%d\n", serverSock);

    // Server internet addres (ip and port)
    server.sin_family = AF_INET;
    server.sin_port = htons(atoi(port));
    inet_aton("localhost", (struct in_addr *)&server.sin_addr.s_addr);
    printf("server.sin_port %d\n", server.sin_port);
    printf("server.sin_addr.s_addr %d\n", server.sin_addr.s_addr);


    // int synRetries = 2; // Send a total of 3 SYN packets => Timeout ~7s
    // if (setsockopt(serverSock, IPPROTO_TCP, TCP_SYNCNT, &synRetries, sizeof(synRetries)) < 0){
    //     perror("erro em setsockopt");
    //     exit(1);
    // }

    printf("Conexão...\n");
    if (connect(serverSock, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("connect() error");
        exit(EXIT_FAILURE);
    }

    printf("Conexão concluída (sock = %d)\n", serverSock);

    return serverSock;
}

int main(int argc, char **argv)
{
    int serverSock;
    char request[1024] = {0};
    char reqFilePath[1024];
    char buffer[1024];

    if (argc != 3)
    {
        fprintf(stderr, "Use: %s <endereço IP> <porta>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        printf("Client ready\n");

        serverSock = connectToServer(argv[1], argv[2]);
       
        printf("Digite o comando:\n");
        scanf("%s", buffer);
    }
}