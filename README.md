# C WebServer

Servidor HTTP desenvolvido para disciplina EA872 FEEC-Unicamp.

## Building

### Server

Executar o script shell `./scripts/compile_server.sh`.

**compile_server.sh**

```bash
#! /bin/bash
cd src

# Gerar o parser
bison -o parserBison.tab.c -d parserBison.y
flex -o parserFlex.yy.c parserFlex.l 

# Compilar os programas
gcc *.c -o ../server -lfl -Wall -Wextra -g
```

### Simulador de Cliente

Executar o script shell `./scripts/compile_client.sh`.

**compile_client.sh**

```bash
#! /bin/bash
gcc -o client ./client_sim/*.c ./src/ioHelper.c -Wall -Wextra
```

## Using

### Server

Basta rodar o executável com o número da porta.

```bash
./server 8080
```

Por padrão, a seguinte mensagem deve aparecer (o número 32143 poderá ser diferente).

```
./server já está aceitando conexões de clientes HTTP em 8080.

[32143] Aguardando conexões... 2 filho(s) livre(s)
```

### Client

Rodar o comando com ip e porta

```bash
./client localhost 8080
```

O programa irá pedir por um comando. Para conectar ao servidor, digite `connect`

```
Enter command:
connect

Conectando em localhost:8080
Conexão concluída (sock = 3)
```

Em seguida, o programa vai pedir por uma requisição. A requisição deve ser o nome de um arquivo da pasta `./client_sim/reqs` (sem a extensão). Como exemplo

```
Requisição:
get_index

=============== CONTEÚDO DA REQUISIÇÃO =============== 

GET / HTTP/1.1
Host: localhost:5544
User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:109.0) Gecko/20100101 Firefox/117.0
...


=============== CONTEÚDO DA RESPOSTA ===============

HTTP/1.1 200 OK
Date: Tue Oct 17 17:45:46 2023 BRT
Server: JFCM Server 0.1
...

<!DOCTYPE html>
...
```