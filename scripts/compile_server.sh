#! /bin/bash

cd src

# Gerar o parser
flex parser_flex.l
bison -o parser_bison.tab.c -d parser_bison.y

# Compilar os programas
gcc *.c -o ../server -lfl

# Mover os cabeçalhos
mv parser_bison.tab.h ../includes/parser_bison.tab.h