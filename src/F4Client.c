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
#include <sys/msg.h>

#include "errExit.h"
#include "message.h"
#include "shmbrd.h"


void printBoard(char **, int, int);

int main(int argc, char * argv[]) {

    /* Chiavi per le code dei messaggi */
    key_t serverKey = 100; // Coda di invio messaggi al Server
    key_t clientKey = 101; // Coda di ricezione dei messaggi dal Server

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090; // Chiave per lo spazio di memoria condivisa su cui e' presente il campo di gioco

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

    printf("%s\n", msg.content);
    int row = msg.row;
    int col = msg.col;

    /* Accesso alla memoria condivisa per il campo di gioco, la dimensione viene comunicata
     * dal server. */
    size_t boardSize = sizeof(struct shared);
    int shmid = shmget(boardKey, boardSize, S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        errExit("shmget failed");
    }

    struct shared * ptr_gb = shmat(shmid, 0, 0);
    printf("<F4Client> rows: %d; cols: %d.\n", row, col);
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (j == 0) {
                printf("| %c |", ptr_gb->board[i][j]);
            } else {
                printf(" %c |", ptr_gb->board[i][j]);
            }
        }
        printf("\n");
    }

    printf("Dimensione del tabellone: %ld\n", boardSize);


    /* Chiusura della shared memory */
    if (shmdt(ptr_gb) == -1) {
        errExit("shmdt failed");
    }
    return 0;
}


/* Il Client e' responsabile della stampa del campo di gioco
 * @param gB matrice in memoria condivisa che rappresenta il campo
 * @param row numero di righe della matrice
 * @param col numero di colonne della matrice
 **/
void printBoard(char  *gB[], int row, int col) {
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