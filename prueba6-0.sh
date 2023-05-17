#!/bin/bash

# Verificar el número correcto de argumentos
if [ $# -ne 1 ]; then
    echo "Uso: $0 NUMNODOS"
    echo "Donde:"
    echo "  NUMNODOS: Número total de nodos objetivo"
    exit 1
fi

NUMNODOS=$1

echo "COMPROBACIÓN DE PRIORIDADES Y TICKETS EN VARIOS NODOS"
echo "MANDA PETICIÓN UN PROCESO DE CADA PRIORIDAD EN TODOS LOS NODOS"

# Las peticiones llegan en orden contrario, procesos 3 y 2
for ((PRIOPROCESO=3; PRIOPROCESO>1; PRIOPROCESO--)); do
        # Lanzar el proceso en cada nodo
    for ((IDNODO=0; IDNODO<NUMNODOS; IDNODO++)); do
        ./lanzaproceso.o $NUMNODOS $IDNODO $PRIOPROCESO 1
    done
done

wait