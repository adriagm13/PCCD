#!/bin/bash

# Verificar el número correcto de argumentos
if [ $# -ne 1 ]; then
    echo "Uso: $0 NUMNODOS"
    echo "Donde:"
    echo "  NUMNODOS: Número total de nodos objetivo"
    exit 1
fi

NUMNODOS=$1

echo "COMPROBACIÓN DE PRIORIDADES EN UN NODO"
echo "MANDA PETICIÓN UN PROCESO DE CADA PRIORIDAD EN UN NODO"
# Lanzar procesos en un unico nodo
for ((PRIOPROCESO=0; PRIOPROCESO<4; PRIOPROCESO++)); do
    ./lanzaproceso.o $NUMNODOS 0 $PRIOPROCESO 1
done
# Las peticiones llegan en orden contrario
for ((PRIOPROCESO=3; PRIOPROCESO>-1; PRIOPROCESO--)); do
    ./lanzaproceso.o $NUMNODOS 0 $PRIOPROCESO 1
done

wait