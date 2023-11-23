/**********************************8
 * VR456714
 * Niccolo' Iselle
 * 2023-11-16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
<<<<<<< HEAD
#include <sys/msg.h>

#include "errExit.h"
#include "message.h"
=======

#include "errExit.h"
#include "message.h"

>>>>>>> c5d7126bdec0d0977e105231b6387d73d50b0e0b

void printBoard(char **, int, int);

int main(int argc, char * argv[]) {

    key_t serverKey = 100; // Coda di invio messaggi al Server
    key_t clientKey = 101; // Coda di ricezione dei messaggi dal Server

    /* Verifica che il numero di argomenti al lancio del gioco sia corretto */
    if (argc < 2) {
        const char *msg = "Errore! Inserire il nome utente per avviare una sessione di gioco\n";
        errExit(msg);
    }

    // Creiamo la coda di ricezione
    int msqCli = msgget(clientKey,  S_IRUSR | S_IWUSR);
    if (msqCli == -1) {
        errExit("msgget failed");
    }

    unsigned long len = strlen(argv[1]);
    /* Invia al server il nome del giocatore che si e' connsesso */

    // Creiamo la coda di invio
    int msqSrv = msgget(serverKey, S_IRUSR | S_IWUSR);
    if (msqSrv == -1) {
        errExit("msgget failed");
    }
    // inizializzo il messaggio
    struct message msg;
    msg.mtype = 1;

    for (int i = 0; i < len; i++) {
        msg.content[i] = argv[1][i];
    }
    msg.content[len] = '\0';

    size_t mSize = sizeof(struct message) - sizeof(long);

    if (msgsnd(msqSrv, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }

    printf("<F4Client> In attesa di conferma della connessione al Server...\n");

    /* Attende la risposta dal server per la conferma della connessione e per
     * conoscere il simbolo di gioco */

    if (msgrcv(msqCli, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }

    printf("%s", msg.content);

<<<<<<< HEAD
=======
    printf("Giocatore 1: %s\n", playerName);
    /* Invia al server il nome del giocatore e riceve il simbolo */
    int msgKey = 1234;
    int msqP1ID = msgget(msgKey, S_IRUSR | S_IWUSR);
    if (msqP1ID == -1) {
        errExit("msgget failed");
    }

    struct message msg;

    msg.mtype = 1;
    msg.content = argv[1];

    size_t mSize = sizeof(struct message) - sizeof(long);
    if (msgsnd(msqP1ID, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }
    /* Ricezione del simbolo */
    if (msgrcv(msqP1ID, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }

    printf("%s", msg.content);
>>>>>>> c5d7126bdec0d0977e105231b6387d73d50b0e0b


    return 0;
}


/* Il Client e' responsabile della stampa del campo di gioco
 * @param gB matrice in memoria condivisa che rappresenta il campo
 * @param row numero di righe della matrice
 * @param col numero di colonne della matrice
 **/
void printBoard(char ** gB, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (j == 0) {
                printf("| %c |", gB[i][j]);
            } else {
                printf(" %c |", gB[i][j]);
            }
        }
        printf("\n");
    }


}