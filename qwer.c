#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
			//1 box, 1 corredor al tiempo, varios corredores
#define TRUE 1
#define FALSE 0
#define NULO -1

int numCorredoresHastaAhora=0;
int filaBoxes;
struct corredores{
	int identificador;
	int posicionArray;
	int sancionado;
	pthread_t tid;
};
struct corredores misCorredores;
pthread_mutex_t mutex; //utilizado para las señales de sincronización de corredores con boxes y juez
pthread_cond_t condicionReparado; // han entrado los cinco threads
pthread_cond_t condicionEsperaSancion; //esto está remadre
pthread_cond_t condicionSancionCumplida; // han entrado los cinco threads
FILE *LOG;

void procesamiento_SIGUSR1();
void escribeEnLog(char * mensaje, int identificador, char entidad); //Habrá que tener en cuenta si quien llama a la función es un corredor, un box o el juez
int calculaDigitos(int numero);
void nuevoCorredor();
void accionesCorredor();
int accionesBox(int atendidos, int posicion);
void accionesJuez();

void *hiloCorredores(void *ptr){
	int posicion;
	posicion = *(int *)ptr;

	accionesCorredor();
	pthread_exit(NULL);
}

void *hiloBoxes(void *ptr) {
	int posicion, atendidos=0;
	posicion = *(int *)ptr;

	while(TRUE){
		atendidos=accionesBox(atendidos, posicion);
	}
}

void *hiloJuez(void *arg) {
	while(TRUE){
		sleep(10);
		accionesJuez();
	}
}



int main(){
	int i, cero=0;
	pthread_t box1, juez;
	pthread_attr_t attrBoxes, attrJuez;

	srand(time(NULL)); //semilla plantada
	//inicializamos valores de la estructura misCorredores
	misCorredores.identificador = NULO;
	misCorredores.posicionArray = NULO;
	misCorredores.sancionado = NULO;
	misCorredores.tid = NULO;
	//inicializacion de condiciones
	pthread_cond_init(&condicionReparado, NULL);
	pthread_cond_init(&condicionEsperaSancion, NULL); //condicionReparado condicionEsperaSancion condicionSancionCumplida
	pthread_cond_init(&condicionSancionCumplida, NULL);
	//inicialización de atributos
	pthread_attr_init(&attrBoxes);
	pthread_attr_init(&attrJuez);
	//creacion de hilos
	pthread_create(&box1, &attrBoxes, hiloBoxes, (void *)&cero);
	pthread_create(&juez, &attrJuez, hiloJuez, NULL);

	//kill(getpid(), SIGUSR1);
	nuevoCorredor();
	pthread_join(box1, NULL);
	pthread_join(juez, NULL);
}


void procesamiento_SIGUSR1(){
	if(signal(SIGUSR1, procesamiento_SIGUSR1)==SIG_ERR){
		perror("Error processing signal SIGUSR1");
	}
	nuevoCorredor();
}


void escribeEnLog(char * mensaje, int numeroIdentificador, char entidad){
	int digitosIdentificador=calculaDigitos(numeroIdentificador);
	char intToChar[digitosIdentificador];
	char * cadenaIdentificador;

	if(entidad=='c'){
		cadenaIdentificador = (char *)malloc((digitosIdentificador+10)*sizeof(char));
		strcat(cadenaIdentificador, "Corredor_");
		sprintf(intToChar, "%d", numeroIdentificador);
		strcat(cadenaIdentificador, intToChar); 
	}else if(entidad=='b'){
		cadenaIdentificador = (char *)malloc((digitosIdentificador+5)*sizeof(char));
		strcat(cadenaIdentificador, "Box_");
		sprintf(intToChar, "%d", numeroIdentificador);
		strcat(cadenaIdentificador, intToChar); 
	}else if(entidad=='j'){
		cadenaIdentificador = (char *)malloc(5*sizeof(char));
		strcat(cadenaIdentificador, "Juez");
	}

	time_t now = time (0);
	struct tm * tlocal = localtime(&now);
	char stnow [19];
	strftime(stnow, 19, "%y/%m/%d %H:%M:%S", tlocal);

	LOG = fopen("LogCarrerita1.3.log", "a");
	fprintf(LOG , "[20%s] %s: %s\n", stnow, cadenaIdentificador, mensaje);
	fclose(LOG);
	free(cadenaIdentificador);
}


int calculaDigitos(int numero){
	int i;

	for(numero=1; numero>10; i++){
		numero = numero/10;
	}
	return i;
}


void nuevoCorredor(){
	if(misCorredores.identificador==NULO){
		misCorredores.identificador = ++numCorredoresHastaAhora;
		pthread_create(&(misCorredores.tid), NULL, hiloCorredores, NULL);
		printf("Corredor añadido\n");
	}
}


void accionesCorredor(){
	int i, tiempoDeEstaVuelta;
	char mensaje[16], vuelta[2];

	escribeEnLog("entro en la carrera", misCorredores.identificador, 'c');
	for (i = 0; i < 5; i++) {
		tiempoDeEstaVuelta = rand() % 4 + 2;
		sleep(tiempoDeEstaVuelta);

		strcat(mensaje, "dio la vuelta ");
		sprintf(vuelta, "%d", i+1); 
		strcat(mensaje, vuelta);
		escribeEnLog(mensaje, misCorredores.identificador, 'c');
		strcpy(mensaje, "");
		printf("di la puta vuelta %d\n", i+1);

		if (rand()%2 == 1){
			printf("pal psicologo tio, por matar a mis amiguitos del colegio\n");
			filaBoxes=1;
/*ep*/			pthread_cond_wait(&condicionReparado, &mutex);//Es wart bis die Zeichen gesendet wird (espera hasta que la señal se envia)
			printf("bueno, salí de esta\n");
		}

		if (misCorredores.sancionado == TRUE) {
			printf("otia tio, la GC\n");
/*ep*/			pthread_cond_signal(&condicionEsperaSancion);//señal de la verga
/*ep*/			pthread_cond_wait(&condicionSancionCumplida, &mutex);//espera la puta
			printf("otia tio, meh dajao tos los napos\n");
		}
	}
	escribeEnLog("ha terminado la puta carrera", misCorredores.identificador, 'c');
	printf("he terminado la puta carrera\n");
	misCorredores.identificador=NULO;
	misCorredores.posicionArray=NULO;
	misCorredores.sancionado=NULO;
	misCorredores.tid=NULO;
}

int accionesBox(int atendidos, int posicion){ //A veces da error "*** stack smashing detected ***: ./carrerita1.3 terminated\nAborted", tanto cuando repara como cuando expulsa
	int tiempoAtencion, hayProblemas, digitosIdentificador=calculaDigitos(misCorredores.identificador);
	char mensaje[48];
	char * cadenaIdentificador;

	if(filaBoxes==NULO){
		sleep(1);
	}else{
		atendidos++;
		tiempoAtencion = rand() % 3 + 1;	
		sleep(tiempoAtencion);
		hayProblemas = rand() % 10;
		if (hayProblemas < 3) {
			filaBoxes=NULO;
			printf("A LA PUTA CALLE MALPARIO\n");

			strcat(mensaje, "se expulso a corredor_");
			cadenaIdentificador = (char *)malloc((digitosIdentificador+1)*sizeof(char));
			sprintf(cadenaIdentificador, "%d", misCorredores.identificador);
			strcat(mensaje, cadenaIdentificador);
			strcat(mensaje, " por problemas mecanicos");

			escribeEnLog(mensaje, posicion, 'b');
			strcpy(mensaje, "");

			misCorredores.identificador=0;
			misCorredores.posicionArray=NULO;
			misCorredores.sancionado=FALSE;
			misCorredores.tid=0;

			free(cadenaIdentificador);
			pthread_cancel(misCorredores.tid);
		}else{
			filaBoxes=NULO;
			printf("QUE NO VUELVA A PASAR\n");
			strcat(mensaje, "se atendio a corredor_");
			cadenaIdentificador = (char *)malloc((digitosIdentificador+1)*sizeof(char));
			sprintf(cadenaIdentificador, "%d", misCorredores.identificador);
			strcat(mensaje, cadenaIdentificador);
			strcat(mensaje, " con exito");

			escribeEnLog(mensaje, posicion, 'b');
	/*ep*/		pthread_cond_signal(&condicionReparado);
			strcpy(mensaje, "");
			free(cadenaIdentificador);
		}
		if (atendidos >= 3) {
			printf("AY K SIESTIKA M BOI HA HEXAR");
			sleep(20);
			atendidos = 0;
		} 
	}
	return atendidos;
}


void accionesJuez(){
	int posicion;

		posicion = rand()%2;
		if(posicion==1){
			misCorredores.sancionado = TRUE;
/*ep*/			pthread_cond_wait(&condicionEsperaSancion, &mutex);
			sleep(3);
/*ep*/			pthread_cond_signal(&condicionSancionCumplida);//esto es la mayor inventada que me he pegado en mucho tiempo
			misCorredores.sancionado = FALSE;

			escribeEnLog("corredor_1 cumplio su sancion", NULO, 'j');
		}
}
