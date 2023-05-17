#!/bin/bash

# Verificar el número correcto de argumentos
if [ $# -ne 3 ]; then
    echo "Uso: $0 PRIOPROCESO NUMNODOS NUMPROCESOS"
    echo "Donde:"
    echo "  PRIOPROCESO: Prioridad del proceso (de 0 a 3)"
    echo "  NUMNODOS: Número total de nodos objetivo"
    echo "  NUMPROCESOS: Número total de procesos a lanzar en cada nodo"
    exit 1
fi

PRIOPROCESO=$1
NUMNODOS=$2
NUMPROCESOS=$3

# Lanzar el proceso en cada nodo
for ((IDNODO=0; IDNODO<NUMNODOS; IDNODO++)); do
    ./lanzaproceso.o $NUMNODOS $IDNODO $PRIOPROCESO $NUMPROCESOS
done

wait
