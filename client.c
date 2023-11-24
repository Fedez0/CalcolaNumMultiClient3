#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8080
#define BUFFER_SIZE 1024


const char errOverflow[] = "OVER";
/*
//send
if (send(client_socket, message, strlen(message), 0) == -1) {
        perror("Errore nell'invio dei dati al server");
        exit(EXIT_FAILURE);
}
//recv
if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
        perror("Errore nella recv dal server");
        exit(EXIT_FAILURE);
}
*/ 
int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];

    // Creazione del socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Errore nella creazione del socket");
        exit(EXIT_FAILURE);
    }

    // Inizializzazione della struttura del server
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Connessione al server
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Errore nella connessione al server");
        exit(EXIT_FAILURE);
    }
//////////////////////////////////////////////////////////////////////////////////////////
//                  inizio programma
    printf("Connesso al server %s:%d\n", SERVER_IP, PORT);

    // Invio dei dati al server
    if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
        perror("Errore nella recv dal server");
        exit(EXIT_FAILURE);
    }
    buffer[sizeof(buffer)]=0;
    if(strcmp(buffer,errOverflow)==0){
        printf("Errore ci sono troppi client collegati al server\n");
        close(client_socket);
        exit(-1);
    }else{
        do{ 
            char pacchettoPulito[BUFFER_SIZE];
            int i;
            for(i=1;i<strlen(buffer);i++){
                pacchettoPulito[i-1]=buffer[i];
            }
            pacchettoPulito[i-1]='\n';
            //leggi il buffer e capisci dall'iniziale cosa è successo prima
            printf("%c\n",buffer[0]);
            switch (buffer[0]){
                case 'P'://nuova partita da zero
                    printf("\nBenvenuto, rispondi a queste operazioni senza sbagliare!\n%s\n",pacchettoPulito);
                    break;
                case 'E'://sbagliata quella precedente
                    printf("\nRisposta precedente sbagliata provare con questa nuova \n%s\n",pacchettoPulito);
                    break;
                case 'K'://giusta quella precedente
                    printf("\nRisposta corretta,rispondere a questa nuova operazione\n%s\n",pacchettoPulito);
                    break;
                default:
                    printf("Arrivederci!\n");
                    close(client_socket);
                    exit(0);
            }
            scanf("%s",buffer);
            if (send(client_socket, buffer, strlen(buffer), 0) == -1) {
                    perror("Errore nell'invio dei dati al server");
                    exit(EXIT_FAILURE);
            }
            if (recv(client_socket, buffer, sizeof(buffer), 0) == -1) {
                perror("Errore nella recv dal server");
                exit(EXIT_FAILURE);
            }
            buffer[sizeof(buffer)]=0;

        }while(1);
    }
    return 0;
}
