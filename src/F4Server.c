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
<<<<<<< HEAD

=======
#include <signal.h>
>>>>>>> c5d7126bdec0d0977e105231b6387d73d50b0e0b

#include "errExit.h"
#include "message.h"

<<<<<<< HEAD

void reciveMessage(key_t key,int msqid, struct message * msg, int mSize, int msgtyp, int msgflg);
=======
/* Setup code dei messaggi fra server e giocatori */
int msqP1ID;
int msqP2ID;
>>>>>>> c5d7126bdec0d0977e105231b6387d73d50b0e0b

int main(int argc, char * argv[]) {

    key_t serverKey = 100; // Chiave per la ricezione dei messaggi dal Client
    key_t clientKey = 101; // Chiave per l'invio di messaggi al Client


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

<<<<<<< HEAD
    printf("<F4Server> In attesa della connessione dei giocatori...\n");

    // TODO: Allocare matrice per campo di gioco in memoria condivisa

    /* Creazione delle code condivise per lo scambio di messaggi tra client e server. */
    // Coda di invio
    int msqSrv = msgget(serverKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqSrv == -1) {
        errExit("msgget failed");
    }

    // Coda di risposta
    int msqCli = msgget(clientKey, IPC_CREAT | S_IWUSR |S_IRUSR);
    if (msqCli == -1) {
        errExit("msgget failed");
    }

    struct message msg;
    msg.mtype = 1;

    /* Attendiamo i client leggendo i messaggi dalla coda */
    size_t mSize = sizeof(struct message) - sizeof(long);

            /********************** GIOCATORE 1 **********************/
    if (msgrcv(msqSrv, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }
    char *name = msg.content;
    // Invia un messaggio di connessione stabilita al giocatore 1 e comunica
    // il suo simbolo di gioco
    printf("<F4Server> Giocatore 1 connesso: %s.\tGettone: %s\n", name, argv[3]);

    /* Creiamo il messaggio per il giocatore 1 */

    char * response = "<F4Server> Connessione confermata!\tIl tuo gettone e': ";
    unsigned long len = strlen(response);
    for (int i = 0; i < len; i++) {
        msg.content[i] = response[i];
    }
    msg.content[len] = argv[3][0];
    msg.content[len+1] = '\0';

    if (msgsnd(msqCli, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }

    /********************** GIOCATORE 2 **********************/

    if (msgrcv(msqSrv, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }
    name = msg.content;

    // Invia un messaggio di connessione stabilita al giocatore 1 e comunica
    // il suo simbolo di gioco
    printf("<F4Server> Giocatore 2 connesso: %s\tGettone: %s\n", name, argv[4]);

    // Creaiamo il messaggio per il giocatore 2.
    for (int i = 0; i < len; i++) {
        msg.content[i] = response[i];
    }
    msg.content[len] = argv[4][0];
    msg.content[len+1] = '\0';

    if (msgsnd(msqCli, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }
=======
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
>>>>>>> c5d7126bdec0d0977e105231b6387d73d50b0e0b

    return 0;
}

