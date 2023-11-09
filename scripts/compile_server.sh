#! /bin/bash
cd src

# Gerar o parser
bison -o parserBison.tab.c -d parserBison.y
flex -o parserFlex.yy.c parserFlex.l 

# Compilar os programas
gcc *.c -o ../server -lfl -lpthread -Wall -Wextra -g