#! /bin/bash

cd src

# Gerar o parser
bison -o parserBison.tab.c -d parserBison.y
flex -o parserFlex.yy.c parserFlex.l 

# Compilar os programas
gcc *.c -o ../server -lfl -Wall -Wextra -g

# Mover os cabeçalhos