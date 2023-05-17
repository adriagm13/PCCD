#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <pthread.h>
#include <stdlib.h>
#include <semaphore.h>
int  vector_atendidas[20][6]={0};

//tipo=1 peticion, tipo=2 testigo, tipo=3 minitestigo

typedef enum {
    ANULACIONES,
    PAGOS,
    ADMINRESER,
    CONSULTAS,
    NULO
} PrioProceso;

struct Paquete{
    long mtype;          //Destinatario DEST
    int id_nodo;        //Remitente SRC
    int num_ticket;     //El número de ticket  
    PrioProceso prio_proceso;
};




int main(int argc, char * argv[]){


int NumNodos=atoi(argv[1]);

int id_nodos[100000];
key_t NodosExternos[10000];
key_t key_minodo= ftok("/bin/ls",50); //claves
int id_global=msgget(key_minodo,0777 | IPC_CREAT); 

for(int v=0;v<NumNodos;v++){
          NodosExternos[v]= ftok("/bin/ls",v); //claves
          id_nodos[v]=msgget(NodosExternos[v],0777 | IPC_CREAT); //id de los nodos, creamos buzon          
  	      printf("Id obtenido, nodo %d--> id %d\n",v,id_nodos[v]);
      
    }

for(int i = 0 ;i < NumNodos;i++){

        int status = 0;

        status = msgctl(id_nodos[i],IPC_RMID,NULL);

        if(status == -1){
            printf("Error al eliminar la cola.\n");
            exit(-1);
        }

        printf("Eliminada la cola con identificador: %i\n",id_nodos[i]);

    }

for(int a=0;a<8;a++){

for(int i=0;i<NumNodos;i++){

    for(int v=0;v<1000;v++){
  struct Paquete salida;
    msgrcv(id_nodos[i],&salida,sizeof(salida),v,IPC_NOWAIT);  
    }
}

 for(int v=0;v<1000;v++){
  struct Paquete salida;
    msgrcv(id_global,&salida,sizeof(salida),v,IPC_NOWAIT);  
    }

}



}