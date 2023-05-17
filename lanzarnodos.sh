#!/bin/bash

# Verificar el número correcto de argumentos
if [ $# -ne 1 ]; then
    echo "Uso: $0 NUMERODENODOS"
    echo "Donde:"
    echo "  NUMERODENODOS: Número total de nodos a lanzar (de 0 a NUMERODENODOS-1)"
    exit 1
fi

NUMERODENODOS=$1


# Lanzar cada nodo
for ((i=0; i<NUMERODENODOS; i++)); do
    ./nodos.o $i $NUMERODENODOS &
done

wait
