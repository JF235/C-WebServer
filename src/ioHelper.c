#include "../includes/essentials.h"

void fileName2stdout(char *filename)
{
    FILE *file;
    TRY_NULL( file = fopen(filename, "r") );
    file2stdout(file);
    fclose(file);
}

void file2stdout(FILE *file)
{
    char c; // char
    while ((c = fgetc(file)) != EOF)
        printf("%c", c);
    fseek(file, 0, SEEK_SET); // Reset seek point
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
    printf("%s\n", filename);
    TRY_NULL( file = fopen(filename, "r") );
    file2buffer(file, buff);
    fclose(file);
}

void buffer2fileName(char *buffer, char *filename, size_t bufferSize)
{
    int fd;
    size_t bytesWritten;
    // Abra o arquivo para escrita em modo binário
    TRY_ERR( fd = open(filename, 2) );

    // Escreva o conteúdo do buffer no arquivo
    TRY_ERR( bytesWritten = write(fd, buffer, bufferSize) );

    // Verifique se ocorreu erro ao escrever todo conteúdo
    if (bytesWritten != bufferSize)
    {
        perror("Erro ao escrever no arquivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    close(fd);
}