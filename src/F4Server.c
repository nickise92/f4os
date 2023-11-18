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
#include <sys/stat.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>

#include "errExit.h"
#include "message.h"

/* Setup code dei messaggi fra server e giocatori */
int msqP1ID;
int msqP2ID;

int main(int argc, char * argv[]) {

    /* Verifichiamo che il server sia avviato con il numero corretto di argomenti */
    if (argc < 5) {
        const char *msg ="Errore! Devi specificare le dimensioni del campo di gioco.\n"
                         "Esempio di avvio: './F4Server 5 5 O X'\n";
        errExit(msg);
    }

    /* Estrapoliamo in due variabili intere le righe e le colonne della matrice che
     * andremo a generare come campo di gioco */
    int row = atoi(argv[1]);
    int col = atoi(argv[2]);

    /* Creiamo il campo di gioco. Le dimensioni minime sono 5x5, quindi verifichiamo che
     * le dimensioni inserite siano corrette prima di generare la matrice */
    if (row < 5 || col < 5) {
        const char *msg = "Errore! Il campo di gioco minimo deve avere dimensioni 5x5!\n";
        printf("%s", msg);
        exit(1);
    }

    // Allochiamo la matrice che funge da campo di gioco in uno spazio di memoria condivisa
    char ** gameBoard;
    gameBoard = (char **) malloc (sizeof(char *) * row);
    for (int i = 0; i < col; i++) {
        gameBoard[i] = (char *) malloc (sizeof(char) * col);
    }

    /* Setup della coda condivisa tra giocatore 1 e server */
    int msgKeyP1 = 1234;
    msqP1ID = msgget(msgKeyP1, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqP1ID == -1) {
        errExit("msgget failed");
    }

    struct message msg;

    /* Leggiamo il messaggio inviato dal client quando si connette */
    size_t mSize = sizeof(struct message) - sizeof(long);
    if (msgrcv(msqP1ID, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }
    char * p1Name = msg.content;
    // Stampa a video il nome del giocatore 1
    printf("Benvenuto %s, sei il primo giocatore.", p1Name);

    /* Prepariamo il messaggio da inviare al client, per comunicare il suo simbolo */
    msg.mtype = 1; // default
    strcpy(msg.content, "Il tuo simbolo è: ");
    strcpy(msg.content, argv[3]);

    if (msgsnd(msqP1ID, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }

    // debug
    printf("Done\n");


}