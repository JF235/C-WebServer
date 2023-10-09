#!/bin/bash

# Realiza o teste de todos os requests presentes na pasta reqs
# A saída é colocada no arquivo resp e no registro.txt

# Webspace
webspace="/home/jf/meu-webspace"

# Defina o caminho para as pastas de entrada e saída
pasta_reqs="web/reqs"
pasta_resp="web/resp"

# Limpa registro
rm -f web/registro.txt

# Crie um loop para processar cada arquivo arquivox.in
for arquivo_in in "$pasta_reqs"/*.txt; do
    # Verifique se o arquivo .in existe
    if [ -e "$arquivo_in" ]; then
        echo

        # Extraia o nome do arquivo (sem extensão)
        nome_arquivo=$(basename "$arquivo_in" .txt)
        
        # Crie o caminho completo para o arquivo de saída
        arquivo_out="$pasta_resp/$nome_arquivo.txt"

        # Execute o comando com os argumentos adequados
        ./server "$webspace" "$pasta_reqs/$nome_arquivo.txt" "$arquivo_out" web/registro.txt

        echo "Processado: $arquivo_in"
    fi
done
