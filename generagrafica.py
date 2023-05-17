import csv
import matplotlib.pyplot as plt

# Listas para almacenar los tiempos de los procesos
tiempos_proceso0 = []
tiempos_proceso1 = []
tiempos_proceso2 = []
tiempos_proceso3 = []

# Leer el archivo CSV
with open('data.csv', 'r') as file:
    reader = csv.reader(file)
    for row in reader:
        nodo = row[0]
        tiempo = row[2]
        proceso = row[3]
        accion = row[5]
        
        if proceso == '0':
            if accion == 'PIDE':
                tiempos_proceso0.append(tiempo)
            elif accion == 'ENTRA':
                tiempos_proceso0.append(tiempo)
        elif proceso == '1':
            if accion == 'PIDE':
                tiempos_proceso1.append(tiempo)
            elif accion == 'ENTRA':
                tiempos_proceso1.append(tiempo)
        elif proceso == '2':
            if accion == 'PIDE':
                tiempos_proceso2.append(tiempo)
            elif accion == 'ENTRA':
                tiempos_proceso2.append(tiempo)
        elif proceso == '3':
            if accion == 'PIDE':
                tiempos_proceso3.append(tiempo)
            elif accion == 'ENTRA':
                tiempos_proceso3.append(tiempo)

# Crear las gráficas
plt.plot(tiempos_proceso0[::2], label='Proceso 0')
plt.plot(tiempos_proceso1[::2], label='Proceso 1')
plt.plot(tiempos_proceso2[::2], label='Proceso 2')
plt.plot(tiempos_proceso3[::2], label='Proceso 3')
plt.xlabel('Solicitud')
plt.ylabel('Tiempo')
plt.title('Tiempo de Procesos 0, 1, 2 y 3 desde PIDE hasta ENTRA SC en 100 nodos')
plt.legend()
plt.show()

