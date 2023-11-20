#! /bin/bash
cd src

# Gerar o parser
bison -o parserBison.tab.c -d parserBison.y
flex -o parserFlex.yy.c parserFlex.l 

# Compilar os programas
gcc *.c -o ../server -lfl -lpthread -lcrypt -Wall -Wextra -g

# Avisar que o webspace não está pronto

cd ..
caminho_arquivo="web/meu-webspace"

if [ -e "$caminho_arquivo" ]; then
    echo "Tudo pronto."
else
    ./scripts/setup_webspace.sh
fi