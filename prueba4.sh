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
echo "MANDA PETICIONES CONSULTAS EN TODOS LOS NODOS"
echo "MIENTRAS SE EJECUTAN CONSULTAS, ENTRAN PETICIONES DE MAYOR
PRIORIDAD EN DISTINTOS NODOS"

# Lanzar el proceso en cada nodo
for ((IDNODO=0; IDNODO<NUMNODOS; IDNODO++)); do
    ./lanzaproceso.o $NUMNODOS $IDNODO 3 40
done

sleep 7

    # Las peticiones llegan en orden contrario
    for ((PRIOPROCESO=2; PRIOPROCESO>-1; PRIOPROCESO--)); do
            # Lanzar el 2 procesos de mayor prioridad en cada nodo
        for ((IDNODO=0; IDNODO<NUMNODOS; IDNODO++)); do
            ./lanzaproceso.o $NUMNODOS $IDNODO $PRIOPROCESO 2
        done
    done
    


