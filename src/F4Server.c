/**********************************8
 * VR456714,
 * Niccolo' Iselle
 * VR455975,
 * Pietro Bianchedi
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

/* Variabili globali */
int count_sig = 0;
struct shared_pid *ptr_playersPid;
struct shared_board *ptr_gb;
pid_t pidServer;
int semid;

/* Handler del segnale di interruzione Ctrl+C */
void sigHandler(int sig) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("signal handler failed");
    }
    if (count_sig == 0) {
        printf("<Server> Attenzione pressione CTRL+C rilevata. Un'ulteriore pressione comporta la chiusura del gioco!\n");
        count_sig++;
    }
    else if (count_sig == 1) {
        printf("<F4Server> Gioco terminato dal Server.\n");

        /* Chiusura delle shared memory */
        if (shmdt(ptr_playersPid) == -1) {
            errExit("shmdt failed");
        } else {
            printf("<F4Server> Memoria condivisa delle info giocatori eliminata con successo.\n");
        }

        if (shmdt(ptr_gb) == -1) {
            errExit("shmdt failed");
        } else {
            printf("<F4Server> Memoria condivisa del tabellone di gioco eliminata con successo.\n");
        }

        /* Chiusura dei semafori */
        if (semctl(semid, 0, IPC_RMID, 0) == -1) {
            errExit("semctl failed");
        } else {
            printf("<F4Server> Semafori rimossi con successo.\n");
        }

        /* Invio del segnale di chiusura ai processi F4Client */
        if (kill(ptr_playersPid->player1,SIGKILL) == -1 || kill(ptr_playersPid->player2,SIGKILL) == -1 ) {
            errExit("kill failed");
        } else {
            printf("<F4Server> Tutti i giocatori sono usciti dalla partita!\n");
            printf("<F4Server> Partita terminata!");
        }
        exit(0);
    }
}

int main(int argc, char * argv[]) {

    /* GESTIONE SIGINT (CTRL-C) */
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("change signal handler failed");
    }

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090; // Chiave per lo spazio di memoria condivisa su cui e' presente il campo di gioco
    key_t pidKey = 6050; // Chiave per l'accesso al pid dei giocatori.

    /* Chiave di accesso al semaforo */
    key_t semKey = 6060;

    /********************** SEMAFORO REGOLATORE TURNI **********************/
    /* Attesa su semaforo bloccante per dare il turno ai giocatori. Il giocatore
     * 1 è bloccato finché non si connette il giocatore 2. Il server deve liberare
     * il giocatore uno dopo la connessione del giocatore 2. Il giocatore 2 resterà
     * bloccato fino al ritorno del controllo al server, che poi cederà il turno al
     * giocatore 2.*/
    // Creazione di 3 semafori
    semid = semget(semKey, 3, IPC_CREAT | S_IWUSR | S_IRUSR);
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
        errExit("shmget board failed");
    }

    ptr_gb = shmat(shBoardID, NULL, 0);

    // Inizializzazione del campo vuoto.
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            ptr_gb->board[i][j] = ' ';
        }
    }

    ptr_gb->rows = row;
    ptr_gb->cols = col;

    /********************** ALLOCAZIONE MEMORIA CONDIVISA PID GIOCATORI **********************/
    size_t pidSize = sizeof(struct shared_pid);
    int shPidID = shmget(pidKey, pidSize, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shPidID == -1) {
        errExit("shmget players failed");
    }

    ptr_playersPid = shmat(shPidID, NULL, 0);
    ptr_playersPid->player1 = -1;
    ptr_playersPid->player2 = -2;
    ptr_playersPid->first = true;

    // Attendo connessione del client
    printf("<F4Server> In attesa della connessione dei giocatori...\n");

    /********************** GIOCATORE 1 **********************/
    // Verifica lo stato di connessione del giocatore 1
    semOp(semid, (unsigned short) 0, -1);        // attendo connessione del client 1
    ptr_playersPid->player1Token = argv[3][0];
    printf("<F4Server> Giocatore 1 connesso.\nNome: %s;\nPID: %d;\nGettone: %c;\n", ptr_playersPid->player1Name, ptr_playersPid->player1, ptr_playersPid->player1Token);
    semOp(semid, 1,1);         // libero client 1 per confermare connessione avvenuta

    /********************** GIOCATORE 2 **********************/
    // Verifica lo stato di connessione del giocatore 1
    semOp(semid, (unsigned short) 0, -1);        // attendo connessione del client 2
    ptr_playersPid->player2Token = argv[4][0];
    printf("<F4Server> Giocatore 2 connesso.\nNome: %s;\nPID: %d;\nGettone: %c;\n", ptr_playersPid->player2Name, ptr_playersPid->player2, ptr_playersPid->player2Token);
    semOp(semid, (unsigned short) 2,1);         // libero client 2 per confermare connessione avvenuta
    semOp(semid, (unsigned short) 0, -1);        // attesa conferma player 2

    /********************** GESTIONE TURNI **********************/
    printf("<F4Server> Tutti i giocatori connessi! La partita può iniziare.\n");
    int count = 0; // debug
    do {
        count++;
        semOp(semid, (unsigned short) 1, 1);    // turno giocatore 1
        printf("<F4Server> Turno di %s\n", ptr_playersPid->player1Name);
        semOp(semid, (unsigned short) 0, -1);   // server attende
        printf("<F4Server> Controllo tornato al Server...\n");
        // Verifica lo stato della partita
        semOp(semid, (unsigned short) 2, 1);     // turno giocatore 2
        printf("<F4Server> Turno di %s\n", ptr_playersPid->player2Name);
        semOp(semid, (unsigned short) 0, -1);   // server attende
        printf("<F4Server> Controllo tornato al Server...\n");
    } while (count < 4);


    /* Chiusura della shared_board memory */
    if (shmdt(ptr_gb) == -1) {
        errExit("shmdt failed");
    } else {
        printf("<F4Server> Memoria condivisa tabellone rimossa con successo.\n");
    }
    if (shmdt(ptr_playersPid) == -1) {
        errExit("shmdt failed");
    } else {
        printf("<F4Server> Memoria condivisa giocatori rimossa con successo.\n");
    }

    /* Rimozione dei semafori */
    if (semctl(semid, 0, IPC_RMID, 0) == -1) {
        errExit("semctl failed");
    } else {
        printf("<F4Server> Semafori rimossi con successo.\n");
    }

    return 0;
}
