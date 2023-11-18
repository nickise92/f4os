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

#include "errExit.h"
#include "message.h"


void printBoard(char **, int, int);

int main(int argc, char * argv[]) {

    /* Verifica che il numero di argomenti al lancio del gioco sia corretto */
    if (argc < 2) {
        const char *msg = "Errore! Inserire il nome utente per avviare una sessione di gioco\n";
        errExit(msg);
    }

    char * playerName = argv[1];

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

    printf("In attesa del secondo giocatore...\n");

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