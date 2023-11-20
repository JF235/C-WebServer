#! /bin/bash
cd src

# Gerar o parser
bison -o parserBison.tab.c -d parserBison.y
flex -o parserFlex.yy.c parserFlex.l 

# Compilar os programas
gcc *.c -o ../server -lfl -lpthread -lcrypt -Wall -Wextra -g

# Avisar que o webspace não está pronto

caminho_arquivo="../web/meu-webspace"

if [ -e "$caminho_arquivo" ]; then
    echo "Tudo pronto."
else
    echo -e "O webspace não está montado.\nExecute o script setup_webspace.sh para montar."
fi