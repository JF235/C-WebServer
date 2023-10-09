#include <stdio.h>
#include <stdlib.h>
#include "../includes/ioHelper.h"

void fileName2stdout(char *filename){
    // Abre o arquivo de requisição para leitura.
    FILE *arquivo_requisicao = fopen(filename, "r");
    if (arquivo_requisicao == NULL)
    {
        perror("Erro ao abrir o arquivo de requisicao.");
        exit(EXIT_FAILURE);
    }
    fclose(arquivo_requisicao);
}

void file2stdout(FILE *file){
    // Imprime o conteúdo da requisição na tela.
    char caractere;
    while ((caractere = fgetc(file)) != EOF)
        printf("%c", caractere);

    // Reseta o offset
    fseek(file, 0, SEEK_SET);
}

void file2buffer(FILE *file, char *buff)
{
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    int bytes_read = fread(buff, 1, file_size, file);
    if (ferror(file))
    {
        perror("Erro ao ler o arquivo");
        exit(EXIT_FAILURE);
    }

    buff[bytes_read] = '\0';
}

void fileName2buffer(char *filename, char *buff)
{
    FILE *file;
    file = fopen(filename, "r");
    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    file2buffer(file, buff);
    fclose(file);
}


    