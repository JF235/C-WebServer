#include "../includes/essentials.h"

void fileName2stdout(char *filename)
{
    // Abre o arquivo de requisição para leitura.
    FILE *arquivo_requisicao = fopen(filename, "r");
    if (arquivo_requisicao == NULL)
    {
        perror("Erro ao abrir o arquivo de requisicao.");
        exit(EXIT_FAILURE);
    }
    fclose(arquivo_requisicao);
}

void file2stdout(FILE *file)
{
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

void buffer2fileName(char *buffer, char *filename, size_t bufferSize)
{
    // Abra o arquivo para escrita em modo binário
    int fd = open(filename, 2);

    // Verifique se o arquivo foi aberto com sucesso
    if (fd == -1)
    {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Escreva o conteúdo do buffer no arquivo
    size_t bytesEscritos = write(fd, buffer, bufferSize);

    // Verifique se ocorreu um erro ao escrever
    if (bytesEscritos != bufferSize)
    {
        perror("Erro ao escrever no arquivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    // Feche o arquivo após a escrita
    close(fd);
}