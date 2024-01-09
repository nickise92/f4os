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
#include <sys/sem.h>
#include <signal.h>

#include "errExit.h"
#include "message.h"
#include "shmbrd.h"
#include "semaphore.h"

#define P1 0
#define P2 1



/* Setup code dei messaggi fra server e giocatori */
int msqSrv = -1;
int msqCli = -1;

// TODO: Non funziona, da sistemare
/* Handler del segnale di interruzione Ctrl+C */
void sigHandler(int sig) {

    if (msqSrv > 0) {
        if(msgctl(msqSrv, IPC_RMID, NULL) == -1) {
            errExit("msgctl failed");
        } else {
            printf("<Server> Coda di messaggi del server eliminata con successo.\n");
        }
    }

    if (msqCli > 0) {
        if (msgctl(msqCli, IPC_RMID, NULL) == -1) {
            errExit("msgctl failed");
        } else {
            printf("<Server> Coda di messaggi del client eliminata con successo.\n");
        }
    }
    exit(0);
}

int main(int argc, char * argv[]) {

    /* Chiavi per le code dei messaggi */
    key_t serverKey = 100; // Coda di ricezione dei  messaggi dal Client
    key_t clientKey = 101; // Coda di invio dei messaggi al Client

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090; // Chiave per lo spazio di memoria condivisa su cui e' presente il campo di gioco
    key_t pidKey = 6010; // Chiave per l'accesso al pid dei giocatori.

    /* Chiave di accesso al semaforo */
    key_t semKey = 6060;

    /* Verifichiamo che il server sia avviato con il numero corretto di argomenti */
    if (argc < 5) {
        const char *err ="Errore! Devi specificare le dimensioni del campo di gioco.\n"
                         "Esempio di avvio: './F4Server 5 5 O X'\n";
        errExit(err);
    }

    /* Estrapoliamo in due variabili intere le righe e le colonne della matrice che
     * andremo a generare come campo di gioco */
    int row = atoi(argv[1]);
    int col = atoi(argv[2]);

    /* Creiamo il campo di gioco. Le dimensioni minime sono 5x5, quindi verifichiamo che
     * le dimensioni inserite siano corrette prima di generare la matrice */
    if (row < 5 || col < 5) {
        printf("Errore! Il campo di gioco minimo deve avere dimensioni 5x5!\n");
        exit(1);
    }

    /********************** ALLOCAZIONE MEMORIA CONDIVISA TABELLONE **********************/
    // Associo alla memoria condivisa il campo di gioco
    size_t boardSize = sizeof(struct shared_board);
    int shBoardID = shmget(boardKey, boardSize, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shBoardID == -1) {
        errExit("shmget failed");
    }

    struct shared_board *ptr_gb = shmat(shBoardID, NULL, 0);

    // Inizializzazione del campo vuoto.
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            ptr_gb->board[i][j] = ' ';
        }
    }
    /********************** ALLOCAZIONE MEMORIA CONDIVISA PID GIOCATORI **********************/
    size_t pidSize = sizeof(struct shared_pid);
    int shPidID = shmget(pidKey, pidSize, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shPidID == -1) {
        errExit("shmget failed");
    }

    struct shared_pid * ptr_playersPid = shmat(shPidID, NULL, 0);
    ptr_playersPid->player1 = -1;
    ptr_playersPid->player2 = -2;

    // Attendo connessione del client
    printf("<F4Server> In attesa della connessione dei giocatori...\n");

    /********************** CODE MESSAGGI CONDIVISI **********************/
    /* Creazione delle code condivise per lo scambio di messaggi tra client e server. */
    // Coda di invio
    msqSrv = msgget(serverKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqSrv == -1) {
        errExit("msgget failed");
    }

    // Coda di risposta
    msqCli = msgget(clientKey, IPC_CREAT | S_IWUSR |S_IRUSR);
    if (msqCli == -1) {
        errExit("msgget failed");
    }

    struct message msg;
    msg.mtype = 1;

    /* Attendiamo i client leggendo i messaggi dalla coda */
    size_t mSize = sizeof(struct message) - sizeof(long);

    /********************** GIOCATORE 1 **********************/
    // RICEZIONE
    if (msgrcv(msqSrv, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }
    char *name = msg.content;
    ptr_playersPid->player1 = msg.pid;

    // Invia un messaggio di connessione stabilita al giocatore 1 e comunica
    // il suo simbolo di gioco
    printf("<F4Server> Giocatore 1 connesso.\nNome: %s;\nPID: %d;\nGettone: %s;\n", name, ptr_playersPid->player1, argv[3]);

    // INVIO
    /* Creiamo il messaggio per il giocatore 1, in cui confermiamo il suo gettone e
     * comunichiamo la dimensione della board di gioco */
    char * response = "<F4Server> Connessione confermata, il tuo gettone e': ";
    unsigned long len = strlen(response);
    for (int i = 0; i < len; i++) {
        msg.content[i] = response[i];
    }
    msg.content[len] = argv[3][0];
    msg.content[len+1] = '\0';
    msg.row = row;
    msg.col = col;
    msg.token = argv[3][0];

    if (msgsnd(msqCli, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }

    /********************** GIOCATORE 2 **********************/
    // RICEZIONE
    if (msgrcv(msqSrv, &msg, mSize, 0, 0) == -1) {
        errExit("msgrcv failed");
    }
    name = msg.content;
    ptr_playersPid->player2 = msg.pid;

    // Invia un messaggio di connessione stabilita al giocatore 1 e comunica
    // il suo simbolo di gioco
    printf("<F4Server> Giocatore 2 connesso.\nNome: %s;\nPID: %d;\nGettone: %s;\n", name, ptr_playersPid->player2, argv[4]);

    // Creaiamo il messaggio per il giocatore 2.
    for (int i = 0; i < len; i++) {
        msg.content[i] = response[i];
    }
    msg.content[len] = argv[4][0];
    msg.content[len+1] = '\0';
    msg.row = row;
    msg.col = col;
    msg.token = argv[4][0];

    if (msgsnd(msqCli, &msg, mSize, 0) == -1) {
        errExit("msgsnd failed");
    }

    /********************** SEMAFORO REGOLATORE TURNI **********************/
    /* Attesa su semaforo bloccante per dare il turno ai giocatori. Il giocatore
     * 1 è bloccato finché non si connette il giocatore 2. Il server deve liberare
     * il giocatore uno dopo la connessione del giocatore 2. Il giocatore 2 resterà
     * bloccato fino al ritorno del controllo al server, che poi cederà il turno al
     * giocatore 2.*/
    // Creazione di 3 semafori
    int semid = semget(semKey, 3, IPC_CREAT | S_IWUSR | S_IRUSR);
    if (semid == -1) {
        errExit("semget failed.");
    }
    // Inizializzazione del semaforo
    unsigned short semInitVal[] = {[0]=0, [1]=0, [2]=0};
    union semun arg;
    arg.array = semInitVal;

    if (semctl(semid, 0, SETALL, arg) == -1) {
        errExit("semctl SETALL failed");
    }

    semOp(semid, (unsigned short) 0, -1);
    printf("<F4Server> Tutti i giocatori connessi! La partita può iniziare.\n");
    semOp(semid, (unsigned short)1, 1);
    semOp(semid, (unsigned short) 0, -1);
    // Verifica lo stato della partita
    semOp(semid, (unsigned short)2, 1);     // turno giocatore 2
    semOp(semid, (unsigned short) 0, -1);   // attende

    /* Chiusura della shared_board memory */
    if (shmdt(ptr_gb) == -1) {
        errExit("shmdt failed");
    }

    return 0;
}

