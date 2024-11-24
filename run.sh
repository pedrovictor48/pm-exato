#!/bin/bash

if [ ! -d "./outs" ]; then
    mkdir "./outs"
fi

for ARQUIVO in ./instances/*; do
    echo "Executando $ARQUIVO"
    NOME_ARQUIVO=$(basename "$ARQUIVO")
    NOME_SEM_EXTENSAO="${NOME_ARQUIVO%.*}"
    ./run "$ARQUIVO" &> "./outs/${NOME_SEM_EXTENSAO}.out"
done
