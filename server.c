#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netdb.h>
#include <time.h>
#include <string.h>
#include <sys/select.h>

#define SERVICEADDR 0x7f000001
#define BUFLEN  1024

#define MAX_CLIENT 4+1


// indirizzo del server (localhost -> 127.0.0.1) espresso come intero a 32 bit
#define SERVICEPORT 8080 //porta del server non standard
const char SERVICENAME[] = "127.0.0.1";    //oppure char SERVICENAME[] = "www.localhost";

const char errOverflow[] = "OVER";
const char risultato[] = "Rrisultato";
const char chiusura[] = "Q";

const char operazione[]="+-*/";
 

/*“overflow” → “OVER”
“risultato” --> “Rrisultato”
“chiusura” --> “Q”
“operazione” --> “xOp1SegnoOp2” */
 
/*
recv
if ((rcvlen = recv(listaSocket[i].sock,rcvbuf, BUFLEN,0)) < 0) {                        
    error(NULL);
    exit (2);
}
send
if (send (listaSocket[i].sock, sendbuf, strlen(sendbuf), 0) < 0) {
    error(NULL);
    exit (2);
} 

*/
double calcoloSoluzione(const char *operazione);
char  *creaOperazione(char *operazioneOutput);
void error(char *msg);
int main () {
    struct sockaddr_in local,remote;
    int s, sc;
    int i;
    unsigned int rcvlen;
    char rcvbuf [BUFLEN], sendbuf[BUFLEN];
    unsigned int remotelen;
    double soluzioni[MAX_CLIENT];
	
    srand(time(NULL));

	fd_set set;  						//definisco var set di tipo fd_set
	
	static struct connection {			//creo struct per contenere dati dei socket dei client connessi
	    int sock;
	    //.......							//aggiungere alla struct le var  da associare a ogni client
	  
	} listaSocket [MAX_CLIENT];
  
	
    for(i=0;i<MAX_CLIENT;i++){		//azzero la lista socket
		listaSocket[i].sock=-1;		//inizializzo l'array con  -1 per poi controllare facilmente se lo slot è libero 
	}
	
	
    printf("Inizializzando il Server!\n");
    
    //////////////////////   crea socket   /////////////////////

    if ((s = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror ("Creazione Socket: ");
        exit (1);
    }

    printf("Socket creato\n\n");
    
    /////////// compila indirizzo del server (il proprio) e fa la bind ////////
    
    local.sin_family = AF_INET;
    local.sin_port = htons(SERVICEPORT);
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind (s, (struct sockaddr*)&local, sizeof(local)) < 0) {
        perror ("errore Bind: ");
        close(s);
        exit (2);
    }

    printf ("Bind corretta\n\n");

    //////////// ricevo richiesta dal client //////////////////////
    
    if (listen(s, 4) < 0) {
        error(NULL);
    }
  
	printf("Sono in ascolto....\n\n");
    
     
    do{
            
        FD_ZERO(&set);               // Svuota il set
        FD_SET(s,&set);      		 // Ci aggiunge il socket in listen s

        for (i = 0;i < MAX_CLIENT; i++) {
            if (listaSocket[i].sock >= 0) {	// aggiungo anche tutti i socket client (sc) che sono attivi
                FD_SET(listaSocket[i].sock, &set); 
            }  
        }
        
        //SELECT()
        
        if (select(FD_SETSIZE, &set, NULL, NULL, NULL)<0){   //ritorna i set che hanno un evento
            error(NULL);
        }
        
        if (FD_ISSET(s,&set)) {		//se ho un evento su s, cioè un socket che ha chiesto connessione
            remotelen = sizeof(remote);
            
            sc = accept(s, (struct sockaddr *)&remote, &remotelen);
            
            
            //aggiungo sc alla lista se c'è posto
            
            for (i = 0;i < MAX_CLIENT; i++) {
                if (listaSocket[i].sock == -1) {	// in uno slot libero metto sc 
                    printf("pos %d - Accetto richiesta di connessione di sc = %d n\n", i,sc);
                    listaSocket[i].sock = sc;
                    //......							//azzero var della struct
                    char operazioneOutput[BUFLEN]="P";
                    char temp[BUFLEN];
                    char temp1[BUFLEN];
                    creaOperazione(temp1);
                    strcpy(temp,temp1);
                    strcat(operazioneOutput,temp);
                    printf("%s",operazioneOutput);
                    soluzioni[i]=calcoloSoluzione(operazioneOutput);
                    strcpy(sendbuf,operazioneOutput);


                    if (send (listaSocket[i].sock, sendbuf, strlen(sendbuf), 0) < 0) {
                        error(NULL);
                        exit (2);
                    } 

                    break;
                }
            }
            
            //se non c'è posto mando "OVER" e chiudo connessione sc 
            
            if (i ==MAX_CLIENT) {
                
                printf("\nClient nuovo, nessun posto libero\n\n");
                strcpy (sendbuf, errOverflow);   
                printf ("Invio messaggio al client %s\n", sendbuf);  
                if (send (sc, sendbuf, strlen(sendbuf), 0) < 0) {
                    error(NULL);
                    exit (2);
                }
                close(sc);
            
            }
        
        }
        
        
        // aggiorno la lista effettuando la recv di tutti i socket sc in lista
        
            for (i = 0;i < MAX_CLIENT; i++) {
                if (FD_ISSET(listaSocket[i].sock,&set)) {	// Dato sul socket
                    if ((rcvlen = recv(listaSocket[i].sock,rcvbuf, BUFLEN,0)) < 0) {                        
                        error(NULL);
                        exit (2);
                    }
                    rcvbuf[rcvlen]=0;
                    char output[BUFLEN];
                    if(strcmp(rcvbuf,"stop")==0){
                        strcpy(output,"STOP");
                        if (send (listaSocket[i].sock, output, strlen(output), 0) < 0) {
                            error(NULL);
                            exit (2);
                        } 
                        close(sc);
                        
                    }else{
                        printf("pachetto ricevuto da %d :%s\n",i,rcvbuf);

                        /*Estraggo la risposta*/
                        char rispostaData[BUFLEN];

                        for(int c=0;c<strlen(rcvbuf);c++){
                            rispostaData[c+1]=rcvbuf[c];
                        }

                        
                        int temp=atoi(rcvbuf);
                        printf("%d:%d\n",temp,risultato[i]);
                        if(temp==soluzioni[i]){
                            output[0]='K';
                            output[1]='\n';
                        }else{
                            output[0]='E';
                            output[1]='\n';
                        }
                        char tempStr[BUFLEN];
                        creaOperazione(tempStr);
                        soluzioni[i]=calcoloSoluzione(tempStr);
                        sprintf(output,"%c%s",output[0],tempStr);
                        printf("%s\n",output);
                        printf("Mandando la risposta\n");
                        if (send (listaSocket[i].sock, output, strlen(output), 0) < 0) {
                            error(NULL);
                            exit (2);
                        }
                        printf("Risposta mandata: %s\n",output);
                    }
                    

                }
            
        }  // for (i = 0;i < MAX_CLIENT; i++) 
    
    } while(1);  // il server non termina
    
    
    close (s);    
    return 0;
}
char *creaOperazione(char *operazioneOutput){
    int x,y;
    char op=operazione[rand()%4]; 
    x=rand()%100;
    y=rand()%100;
    if(op=='-'&&y>x){
        int temp=y;
        y=x;
        x=temp;
    }
    sprintf(operazioneOutput,"%d%c%d",x,op,y);
    return operazioneOutput;
}
double calcoloSoluzione(const char *operazione){
    char op;    
    int op1;
    int op2;
    char temp;


    sscanf(operazione,"%c%d%c%d\n",&temp,&op1,&op,&op2);

   
    switch (op)
    {
    case '+':
        return op1+op2;
        break;
    case '-':
        return op1-op2;
        break;
    case '*':
        return op1*op2;
        break;
    case '/':
        if (op2==0){
            return 0;
        }else{
            return op1/op2;
        }
        break;
    }
}
void error(char *msg){
    perror(msg);
    exit(-1);
}