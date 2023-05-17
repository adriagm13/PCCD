#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h> 
#include <string.h>
#include <signal.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>


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

struct Peticion{
    long mtype;          //Destinatario DEST
    int id_nodo;        //Remitente SRC
    int num_procesos;     //Número de procesos de prioProceso
    PrioProceso prio_proceso;
};

int IDNodosPendientes[100000];
int NodosPendientes = 0;

int consultasActivas = 0;
int consultasEnSC = 0;
int colaConsultas = 0;
int colaAdminreser = 0;
int colaPagos = 0;
int colaAnulaciones = 0;
int prioridadActual = NULO;

int colaProcesos = 0;

int SECCIONCRITICA = 0;
int QUIEROSC = 0;


int REPLY = 1;
int REQUEST = 2;

int max_ticket = 0;
int mi_ticket = 0;

int IDMiNodo;
int IDBuzonNodo;
//Array con los ids de los buzones de los otros nodos
int idNodos[100000];

sem_t sem_crear_hilos;
sem_t sem_block_SC;
sem_t sem_block_quierosc;
sem_t sem_block_ticket;
sem_t sem_block_peticionesprocesos;
sem_t sem_block_colaconsultas;


sem_t sem_consultas;
sem_t sem_adminreser;
sem_t sem_pagos;
sem_t sem_anulaciones;

sem_t sem_solicitaSC;
sem_t sem_saleSC;

void networkrcv(int *IDnodo_origen, int tipocola, PrioProceso *prioproceso_origen, int *ticket_origen);
void networksend(int IDbuzon,int tipocola, PrioProceso prioproceso, int ticket, int miBuzon);
void* procesoHiloConsultas(void * prioProceso);
void* procesoHilo(int * prioProceso);

void printEntraTime(int prio, int hilo_pid) {

        // Obtener el tiempo actual con precisión de milisegundos
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Obtener la estructura struct tm
    time_t tiempo = tv.tv_sec;
    struct tm* tiempo_local = localtime(&tiempo);

    // Obtener los milisegundos
    int milisegundos = tv.tv_usec / 1000;

    // Formatear la cadena de tiempo con hora, minutos, segundos y milisegundos
    char cadena_tiempo[20];
    strftime(cadena_tiempo, sizeof(cadena_tiempo), "%H:%M:%S", tiempo_local);

    printf("[NODO:%i][%s:%03d] PROCESO con prioridad: %i con PID: %i ENTRA a la SC\n",IDMiNodo,cadena_tiempo, milisegundos, prio, hilo_pid);

    // Abrir el archivo en modo de escritura
    FILE* archivo = fopen("data.csv", "a");
    if (archivo == NULL) {
        printf("Error al abrir el archivo\n");
        return 1;
    }

    // Imprimir en el archivo en formato CSV
    fprintf(archivo, "NODO,%i,%s:%03d,%i,%i,ENTRA\n", IDMiNodo, cadena_tiempo, milisegundos, prio, hilo_pid);

    // Cerrar el archivo
    fclose(archivo);

}

void printPideTime(int prio, int hilo_pid) {

        // Obtener el tiempo actual con precisión de milisegundos
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Obtener la estructura struct tm
    time_t tiempo = tv.tv_sec;
    struct tm* tiempo_local = localtime(&tiempo);

    // Obtener los milisegundos
    int milisegundos = tv.tv_usec / 1000;

    // Formatear la cadena de tiempo con hora, minutos, segundos y milisegundos
    char cadena_tiempo[20];
    strftime(cadena_tiempo, sizeof(cadena_tiempo), "%H:%M:%S", tiempo_local);

    printf("[NODO:%i][%s:%03d] PROCESO con prioridad: %i con PID: %i PIDE SC\n",IDMiNodo,cadena_tiempo, milisegundos, prio, hilo_pid);

    // Abrir el archivo en modo de escritura
    FILE* archivo = fopen("data.csv", "a");
    if (archivo == NULL) {
        printf("Error al abrir el archivo\n");
        return 1;
    }

    // Imprimir en el archivo en formato CSV
    fprintf(archivo, "NODO,%i,%s:%03d,%i,%i,PIDE\n", IDMiNodo, cadena_tiempo, milisegundos, prio, hilo_pid);

    // Cerrar el archivo
    fclose(archivo);

}

void printSaleTime(int prio, int hilo_pid) {

        // Obtener el tiempo actual con precisión de milisegundos
    struct timeval tv;
    gettimeofday(&tv, NULL);

    // Obtener la estructura struct tm
    time_t tiempo = tv.tv_sec;
    struct tm* tiempo_local = localtime(&tiempo);

    // Obtener los milisegundos
    int milisegundos = tv.tv_usec / 1000;

    // Formatear la cadena de tiempo con hora, minutos, segundos y milisegundos
    char cadena_tiempo[20];
    strftime(cadena_tiempo, sizeof(cadena_tiempo), "%H:%M:%S", tiempo_local);


    printf("[NODO:%i][%s:%03d] PROCESO con prioridad: %i con PID: %i SALE de la SC\n",IDMiNodo,cadena_tiempo, milisegundos, prio, hilo_pid);

        // Abrir el archivo en modo de escritura
    FILE* archivo = fopen("data.csv", "a");
    if (archivo == NULL) {
        printf("Error al abrir el archivo\n");
        return 1;
    }

    // Imprimir en el archivo en formato CSV
    fprintf(archivo, "NODO,%i,%s:%03d,%i,%i,SALE\n", IDMiNodo, cadena_tiempo, milisegundos, prio, hilo_pid);

    // Cerrar el archivo
    fclose(archivo);

}

int maximo(int num1, int num2) {
    if (num1 > num2) {
        return num1;
    } else {
        return num2;
    }
}

void networkrcv(int *IDnodo_origen, int tipocola, PrioProceso *prioproceso_origen, int *ticket_origen) {

    struct Paquete paquete;
    if (msgrcv(IDBuzonNodo, (struct msgbuf *)&paquete,  sizeof(paquete.id_nodo)*3,tipocola, 0) == -1) {
       printf("ERROR NO RECIBIDO\n");
        exit(1);
    }else{
        *IDnodo_origen = paquete.id_nodo;
        *ticket_origen = paquete.num_ticket;
        *prioproceso_origen = paquete.prio_proceso;

        // Obtener el tiempo actual
    time_t tiempo = time(NULL);
    struct tm* tiempo_local = localtime(&tiempo);

        if(paquete.mtype == REPLY){

            printf("[%s][Nodo %i][RECIBIDO REPLY] Recibo de nodo %i.\n", asctime(tiempo_local),IDMiNodo,paquete.id_nodo);
        
        }else{

            printf("[%s][Nodo %i][RECIBIDO] Recibo de nodo %i. Numero de ticket %i Prioridad de proceso: %i\n", asctime(tiempo_local),IDMiNodo,paquete.id_nodo,paquete.num_ticket, paquete.prio_proceso);
        
        }
    }



}

void networksend(int nododestino,int tipocola, PrioProceso prioproceso, int ticket, int miBuzon) {
      

    // Enviamos el paquete al buzón correspondiente
    struct Paquete message;

    message.id_nodo=miBuzon;
    message.prio_proceso=prioproceso;
    message.mtype=tipocola;
    message.num_ticket=ticket;
        
    if (msgsnd(nododestino, (struct msgbuf *)&message,  sizeof(message.id_nodo)*3, 0) == -1) {
       printf("ERROR NO ENVIADO: Nodo destino:%i  Tipo mensaje: %li \n", nododestino, message.mtype);
        exit(1);
    }else{
                // Obtener el tiempo actual
    time_t tiempo6 = time(NULL);
    struct tm* tiempo_local6 = localtime(&tiempo6);

        if(message.mtype == REPLY){

        printf("[%s][Nodo %i][ENVIADO REPLY] Envío a nodo %i.\n", asctime(tiempo_local6),IDMiNodo,nododestino);
 
        } else{

        printf("[%s][Nodo %i][ENVIADO] Envío a nodo %i. Numero de ticket %i \n", asctime(tiempo_local6),IDMiNodo,nododestino,ticket);
 
        }  
    }


}


void* recepcion(void* args){

    //Tenemos que definir los tipos de cada uno de los paquetes.
    // Espera a recibir un mensaje en la cola de mensajes
    printf("Escuchando en el buzon: %i\n",IDBuzonNodo);

    int id_nodo_origen = 0;
    int ticket_origen = 0;
    PrioProceso prio_proceso_origen = NULO;

    do{
        

        networkrcv(&id_nodo_origen, REQUEST, &prio_proceso_origen,&ticket_origen);

        sem_wait(&sem_block_ticket);
        max_ticket = maximo(max_ticket, ticket_origen);
        sem_post(&sem_block_ticket);

        if( SECCIONCRITICA == 0 && ((!QUIEROSC) 
        || (ticket_origen < mi_ticket) 
        || ((ticket_origen == mi_ticket) && (prio_proceso_origen < prioridadActual)) 
        || ((ticket_origen == mi_ticket) && (id_nodo_origen < IDBuzonNodo)))){

            networksend(id_nodo_origen,REPLY,NULO,0, IDBuzonNodo);
        
        }else{
            //NodosPendientes++;
            IDNodosPendientes[NodosPendientes++] = id_nodo_origen;
        }

    }while(1);

}

void initparam(){

    //INICIALIZACIÓN DE SEMAFOROS
    sem_init(&sem_solicitaSC,0,0);
    sem_init(&sem_crear_hilos,0,1);

    sem_init(&sem_block_SC,0,1);
    sem_init(&sem_block_ticket,0,1);
    sem_init(&sem_block_quierosc,0,1);
    sem_init(&sem_block_peticionesprocesos,0,1);
    sem_init(&sem_block_colaconsultas,0,1);

    sem_init(&sem_consultas,0,0);
    sem_init(&sem_anulaciones,0,0);
    sem_init(&sem_pagos,0,0);
    sem_init(&sem_adminreser,0,0);

    sem_init(&sem_saleSC,0,0);

}

// CREA numProcesos HILOS de PROCESOS con prioridad prioProceso
// prioProceso = 3 -> CONSULTAS
// prioProceso = 2 -> ADMINISTRACION Y RESERVAS
// prioProceso = 1 -> PAGOS
// prioProceso = 0 -> ANULACIONES
int creaHiloProceso(int prioProceso, int NumProcesos){

    pthread_t procesoHilos[100000];

    if(prioProceso == CONSULTAS){

        for(int i = 0; i < NumProcesos; i++){

        pthread_create(&procesoHilos[i],NULL,(void *)procesoHiloConsultas,(void *)&prioProceso);
        sem_wait(&sem_crear_hilos);

        }

    }else{

        for (int i = 0; i < NumProcesos; i++){

        pthread_create(&procesoHilos[i],NULL,(void *)procesoHilo,prioProceso);
        sem_wait(&sem_crear_hilos);

        }
    }

    return 0;

}


void * procesoHiloConsultas(void * prioProceso){

    sem_t* sem_proceso;
    pid_t hilo_pid = gettid();

    sem_proceso = &sem_consultas;

    printf("Creado proceso de prioridad: %i con PID: %i\n", CONSULTAS, hilo_pid);
    sem_post(&sem_crear_hilos);

    sem_wait(&sem_block_peticionesprocesos);
    colaProcesos++;
    sem_post(&sem_block_peticionesprocesos);


    if(consultasActivas == 1){
        //El proceso creado es CONSULTA, las consultas están ACTIVAS y hay consultas en SC, continua a la SC
        sem_wait(&sem_block_colaconsultas);
        colaConsultas++;
        sem_post(&sem_block_colaconsultas);

    }else{
        //Esperamos a que el nodo nos de permiso

        if(colaConsultas == 0){

        sem_post(&sem_solicitaSC);

        printPideTime(CONSULTAS,hilo_pid);

        }

        sem_wait(&sem_block_colaconsultas);
        colaConsultas++;
        sem_post(&sem_block_colaconsultas);

        sem_wait(sem_proceso);

    }

    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos

    printEntraTime(CONSULTAS, hilo_pid);
    
    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos


    printSaleTime(CONSULTAS, hilo_pid);

    //sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos

    sem_wait(&sem_block_colaconsultas);
    colaConsultas--;
    sem_post(&sem_block_colaconsultas);
    if(colaConsultas == 0){
        consultasActivas = 0;
    }

    //Si soy la ultima consulta, aviso de que acabamos todas

    printf("COLA CONSULTAS: %i\n", colaConsultas);
    if(colaConsultas == 0){

    sem_post(&sem_saleSC);

    }

    printf("FIN PROCESO PRIO: %i con PID: %i\n", CONSULTAS, hilo_pid);

    sem_wait(&sem_block_peticionesprocesos);
    colaProcesos--;
    sem_post(&sem_block_peticionesprocesos);

    pthread_exit(NULL);

}

void * procesoHilo(int *prioProceso){

    int prioproceso = prioProceso;
    sem_t* sem_proceso;
    pid_t hilo_pid = gettid();

    if(prioproceso == ANULACIONES){
        sem_proceso = &sem_anulaciones;

        sem_wait(&sem_block_peticionesprocesos);
        colaAnulaciones++;
        sem_post(&sem_block_peticionesprocesos);
    }
    if(prioproceso == ADMINRESER){
        sem_proceso = &sem_adminreser;

        sem_wait(&sem_block_peticionesprocesos);
        colaAdminreser++;
        sem_post(&sem_block_peticionesprocesos);
    }
    if(prioproceso == PAGOS){
        sem_proceso = &sem_pagos;

        sem_wait(&sem_block_peticionesprocesos);
        colaPagos++;
        sem_post(&sem_block_peticionesprocesos);
    }

    printf("Creado proceso de prioridad: %i con PID: %i\n", prioproceso, hilo_pid);
    sem_post(&sem_crear_hilos);

    sem_wait(&sem_block_peticionesprocesos);
    colaProcesos++;
    sem_post(&sem_block_peticionesprocesos);


    sem_post(&sem_solicitaSC);

    printPideTime(prioproceso,hilo_pid);

    sem_wait(sem_proceso);

    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos

    printEntraTime(prioproceso, hilo_pid);

    sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos


    printSaleTime(prioproceso, hilo_pid);

    //sleep(rand()%5+1); //Dormir una cantidad de tiempo aleatoria entre 1 y 5 segundos

    if(prioproceso == ANULACIONES){
        sem_wait(&sem_block_peticionesprocesos);
        colaAnulaciones--;
        sem_post(&sem_block_peticionesprocesos);
    }
    if(prioproceso == ADMINRESER){
        sem_wait(&sem_block_peticionesprocesos);
        colaAdminreser--;
        sem_post(&sem_block_peticionesprocesos);
    }
    if(prioproceso == PAGOS){
        sem_wait(&sem_block_peticionesprocesos);
        colaPagos--;
        sem_post(&sem_block_peticionesprocesos);
    }

    sem_post(&sem_saleSC);

    printf("FIN PROCESO PRIO: %i con PID: %i\n", prioproceso, hilo_pid);


    sem_wait(&sem_block_peticionesprocesos);
    colaProcesos--;
    sem_post(&sem_block_peticionesprocesos);

    pthread_exit(NULL);

}

int main(int args,char *argv[]){ //./nodos.o IDNODO NNODOS NUMCONSULTAS

    IDMiNodo = atoi(argv[1]);
    int NumNodosEnRed = atoi(argv[2]);

    int id_aux = 0;
    PrioProceso prio_proceso_aux = NULO;
    int ticket_aux = 0;

    //id de mi nodo
    key_t key=ftok("/bin/ls",IDMiNodo);
    IDBuzonNodo=msgget(key,0777 | IPC_CREAT);

        //ahora vamos a obtener el id del resto de nodos
    key_t NodosExternos[10000];
    int j=0;
    for(int v=0;v<NumNodosEnRed;v++){
        if(v!=IDMiNodo){
            NodosExternos[j]= ftok("/bin/ls",v); //claves
            idNodos[j]=msgget(NodosExternos[j],0777 | IPC_CREAT); //id de los nodos, creamos buzon          
            j++;
        }
    }

    printf("MI ID NODO: %i\n", IDMiNodo);
    for(int i = 0; i < j; i++){
    printf("IDs buzones otros nodos: %i\n", idNodos[i]);
    }

    initparam();

    srand(time(NULL));

    pthread_t pthreceptor;
    pthread_create(&pthreceptor,NULL,(void *)recepcion,NULL);

    struct Peticion peticionproceso;

    while(1){


    //Comprobamos si se ha recibido una petición de un proceso interno
    struct msqid_ds estado_buzon;
    if (msgctl(IDBuzonNodo, IPC_STAT, &estado_buzon) == -1) {
        perror("Error al obtener el estado del buzon");
        return 1;
    }

    printf("COLA PROCESOS: %i\n", colaProcesos);
    // Verificar si hay mensajes en espera
    if ((estado_buzon.msg_qnum > 0) || ((colaProcesos == 0) && (estado_buzon.msg_qnum == 0))) {

    printf("ESCUCHANDO PETICIONES DE PROCESOS INTERNOS\n");

    //Recogemos todas las peticiones de procesos internas que haya
    do{

    msgrcv(IDBuzonNodo,&peticionproceso, sizeof(peticionproceso),100,0);
    
    creaHiloProceso(peticionproceso.prio_proceso, peticionproceso.num_procesos);

    sleep(2);   //Tiempo de recepción

    if (msgctl(IDBuzonNodo, IPC_STAT, &estado_buzon) == -1) {
        perror("Error al obtener el estado del buzon");
        return 1;
    }


    }while((estado_buzon.msg_qnum > 0));

    }

    printf("ESPERO A QUE SE ME SOLICITE SC\n");

    sem_wait(&sem_solicitaSC);

    printf("Se ha solicitado la SECCION CRITICA\n");

    sem_wait(&sem_block_quierosc);
    QUIEROSC = 1;
    sem_post(&sem_block_quierosc);

    sem_wait(&sem_block_ticket);
    mi_ticket = max_ticket+1; 
    sem_post(&sem_block_ticket);

        if(colaAnulaciones > 0){
            prioridadActual = ANULACIONES;

        }else if(colaPagos > 0){
            prioridadActual = PAGOS;


        }else if(colaAdminreser > 0){
            prioridadActual = ADMINRESER;


        }else if(colaConsultas > 0){
            prioridadActual = CONSULTAS;

        }else{
            prioridadActual = NULO;
        }

    for(int i = 0 ; i < NumNodosEnRed-1; i++){ // No metemos al primero del array porque es el propio nodo
        
        //enviamos a todos los nodos los tickets
        networksend(idNodos[i],REQUEST,prioridadActual,mi_ticket, IDBuzonNodo);
        printf("Solicitud enviada al nodo %i\n",idNodos[i]);

    }

   for(int i = 0; i < NumNodosEnRed-1; i++){
        //recibimos los tickets de todos los nodos

        networkrcv(&id_aux,REPLY, &prio_proceso_aux, &ticket_aux);  // idColaReply //////////Cuidado
        printf("Confirmacion recibida del nodo %i\n",id_aux);
    }


    sem_wait(&sem_block_SC);
    SECCIONCRITICA = 1;
    sem_post(&sem_block_SC);

    if((colaAnulaciones == 0) && (colaPagos == 0) && (colaAdminreser == 0)){
        consultasActivas = 1;
    }else{
        consultasActivas = 0;
    }
    if(consultasActivas > 0){

        for(int i = 0; i < colaConsultas; i++){

            printf("NODO manda entrar en la SC a CONSULTAS...\n");
            sem_post(&sem_consultas);

        }

    sem_wait(&sem_saleSC);

    }else{
        //SI HAY OTRAS PRIOS, HAGO LAS OTRAS PRIMERO
        //SI NO, CONSULTAS ACTIVAS A 1

        if(colaAnulaciones > 0){
            printf("NODO manda entrar en la SC a ANULACION...\n");
            sem_post(&sem_anulaciones);

            sem_wait(&sem_saleSC);

        }else if(colaPagos > 0){
            printf("NODO manda entrar en la SC a PAGO...\n");
            sem_post(&sem_pagos);

            sem_wait(&sem_saleSC);
        }else if(colaAdminreser > 0){
            printf("NODO manda entrar en la SC a ADMINS Y RESERVAS...\n");
            sem_post(&sem_adminreser);

            sem_wait(&sem_saleSC);
        }

    }

    sem_wait(&sem_block_SC);
    SECCIONCRITICA = 0;
    sem_post(&sem_block_SC);

    sem_wait(&sem_block_quierosc);
    QUIEROSC = 0;
    sem_post(&sem_block_quierosc);

    
    if(colaAnulaciones > 0){
        prioridadActual = ANULACIONES;

    }else if(colaPagos > 0){
        prioridadActual = PAGOS;

    }else if(colaAdminreser > 0){
        prioridadActual = ADMINRESER;

    }else if(colaConsultas > 0){
        prioridadActual = CONSULTAS;

    }else{
        prioridadActual = NULO;
    }

    for(int i = 0; i < NodosPendientes; i++){
        //enviamos a cada nodo un reply de que hemos pasado la seccion critica
        networksend(IDNodosPendientes[i],REPLY,NULO,0, IDBuzonNodo);
        printf("Respuesta enviada al nodo %i\n",IDNodosPendientes[i]);
    }

    NodosPendientes = 0;

    }

}


