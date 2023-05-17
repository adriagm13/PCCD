#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>



typedef enum {
    ANULACIONES,
    PAGOS,
    ADMINRESER,
    CONSULTAS,
    NULO
} PrioProceso;

struct Peticion{
    long mtype;          //Destinatario DEST
    int id_nodo;        //Remitente SRC
    int num_procesos;     //Número de procesos de prioProceso
    PrioProceso prio_proceso;
};

int main(int argc, char * argv[]){ // ./lanzaproceso.o NUMNODOS IDNODO PRIOPROCESO NUMPROCESOS


int NumNodos=atoi(argv[1]);
int NumMinodo= atoi(argv[2]);
int prioridad=atoi(argv[3]);
int numprocesos=atoi(argv[4]);




//ahora vamos a obtener el id del resto de nodos
int id_nodos[100000];
key_t NodosExternos[10000];

for(int v=0;v<NumNodos;v++){
          NodosExternos[v]= ftok("/bin/ls",v); //claves
          id_nodos[v]=msgget(NodosExternos[v],0777 | IPC_CREAT); //id de los nodos, creamos buzon          
  	      printf("Id obtenido, nodo %d--> id %d\n",v,id_nodos[v]);
      
    }

        struct Peticion petiproceso;
        petiproceso.mtype=100;//para mandarlo a otro nodo
        petiproceso.id_nodo = 100;
        petiproceso.num_procesos = numprocesos;
        petiproceso.prio_proceso = prioridad;
      

        printf("Lanzamos en %d un proceso de prio %d\n",id_nodos[NumMinodo], petiproceso.prio_proceso);

        msgsnd(id_nodos[NumMinodo],(struct msgbuf *)&petiproceso,sizeof(petiproceso.id_nodo)*3,0);

}