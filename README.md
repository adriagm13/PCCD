Para lanzar un nodo:
./nodos.o IDNODO NUMNODOS
Para lanzar un proceso:
./lanzaproceso.o NUMNODOS IDNODO PRIOPROCESO NUMPROCESOS
Lanzar varios nodos:
./lanzarnodos.sh NUMNODOS
Lanzar procesos de una prioridad en todos los nodos:
./lanzaprocesosnodos.sh PRIOPROCESO NUMNODOS NUMPROCESOS
Lanzar pruebas:
./pruebaX.sh NUMNODOS
Generar gráfica en base a los datos de data.csv:
python3 generagrafica.py
Vaciar buzones:
./vaciar.o NUMBUZONES