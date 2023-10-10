#!/bin/bash

# Executa um exemplo
webspace=/home/jf/meu-webspace
request=web/reqs/get_texto1.txt
response=web/resp/get_texto1.txt
registro=web/registro.txt
./server $webspace $request $response $registro