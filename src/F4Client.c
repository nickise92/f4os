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
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <signal.h>

#include "errExit.h"
#include "message.h"
#include "shmbrd.h"
#include "semaphore.h"

/* Variabili globali */
int count_sig = 0;
struct shared_board *ptr_gb;
struct shared_pid *ptr_playersPid;
struct winning *ptr_winCheck;
int semid;

/* Funzioni & Procedure */
/* Il Client e' responsabile della richiesta al giocatore su quale casella vuole
 * giocare il suo token e di verificarne la legittimita'.
 */
bool checkValidity(struct shared_board *ptr_sh, int col, int rows, char token) {
    for (int i = ptr_gb->rows - 1; i >= 0; i--) {
        if (ptr_gb->board[i][col] == ' ') {
            ptr_gb->board[i][col] = token;
            return true;
        }
    }
    return false;
}

/* Il Client e' responsabile della stampa del campo di gioco
 * @param ptr_sh matrice in memoria condivisa che rappresenta il campo
 * @param row numero di righe della matrice
 * @param col numero di colonne della matrice
 */
void printBoard(struct shared_board  *ptr_sh, int row, int col) {
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (j == 0) {
                printf("| %c |", ptr_sh->board[i][j]);
            } else {
                printf(" %c |", ptr_sh->board[i][j]);
            }
        }
        printf("\n");
    }
    // stampo il numero delle colonne sotto al tabellone
    for (int i = 0; i < col; i++) {
        printf("  %d ", i);
    }
}

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

        /* Informo il server che il processo client corrente ha abandonato */
        ptr_winCheck->playerLeft = getpid();
        exit(0);
    }
}

int main(int argc, char * argv[]) {

    /* Handler CTRL-C */
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("signal handler failed");
    }

    /* VARIABILI LOCALI CLIENT */

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090; // Chiave per lo spazio di memoria condivisa su cui e' presente il campo di gioco
    key_t pidKey = 6050; // Chiave per l'accesso al pid dei giocatori.

    /* Accesso alla memoria condivisa per il campo di gioco, la dimensione viene comunicata
     * dal server. */
    size_t boardSize = sizeof(struct shared_board);
    int shBoardID = shmget(boardKey, boardSize, S_IRUSR | S_IWUSR);
    if (shBoardID == -1) {
        errExit("shmget failed");
    }

    ptr_gb = shmat(shBoardID, 0, 0);

    int row = ptr_gb->rows;
    int col = ptr_gb->cols;
    //printf("<F4Client> rows: %d; cols: %d.\n", row, col);                 // debug
    //printBoard(ptr_gb, row, col);                                     // debug

    /* Chiave di accesso del semaforo */
    key_t semKey = 6060;

    // Accesso al semaforo
    semid = semget(semKey, 3, S_IRUSR | S_IWUSR);
    if (semid == -1) {
        errExit("semget failed");
    }

    /* Accesso alla memoria condivisa per il pid dei giocatori */
    size_t pidSize = sizeof(struct shared_pid);
    int shPidID = shmget(pidKey, pidSize, S_IRUSR);
    if (shPidID == -1) {
        errExit("shmget failed");
    }

    ptr_playersPid = shmat(shPidID, 0, 0);

    /* Verifica che il numero di argomenti al lancio del gioco sia corretto */
    if (argc < 2) {
        const char *msg = "Errore! Inserire il nome utente per avviare una sessione di gioco\n";
        errExit(msg);
    }
    /* Verifichiamo se il client corrente e' il primo giocatore: nel caso
     * in cui 'first' nella shared memory sia true, abbiamo il giocatore 1*/
    if (ptr_playersPid->first) {
        ptr_playersPid->first = false;
        ptr_playersPid->player1 = getpid();
        strcpy(ptr_playersPid->player1Name, argv[1]);
    } else {
        /* altrimenti il secondo giocatore */
        ptr_playersPid->player2 = getpid();
        strcpy(ptr_playersPid->player2Name, argv[1]);
    }

    printf("<F4Client> In attesa di conferma della connessione al Server...\n");

    /* Attende la risposta dal server per la conferma della connessione e per
     * conoscere il simbolo di gioco. */
    if (getpid() == ptr_playersPid->player1) {
        semOp(semid, 0, 1);                             // libero il server
        semOp(semid, (unsigned short)1, -1);            // attendo conferma connsessione
        printf("<F4Client> Connessione al Server confermata.\n\tGettone: %c\n", ptr_playersPid->player1Token);
        printf("<F4Client> In attesa della connessione del giocatore 2.\n");
        semOp(semid, (unsigned short) 1, -1);           // attendo connessione player 2 & il turno
    } else if (getpid() == ptr_playersPid->player2) {
        semOp(semid, 0, 1);                             // libero il server
        semOp(semid, (unsigned short)2, -1);            // attendo conferma connessione
        printf("<F4Client> Connessione al Server confermata.\nGettone: %c\n", ptr_playersPid->player2Token);
        semOp(semid, 0, 1);                             // libero il server
        semOp(semid, (unsigned short) 2, -1);           // attendo il turno
    }

    /********************** TURNI **********************/
    int move;
    int flag;
    bool endGame = false;
    do {
        /* Visualizzo la board per il giocatore */
        printBoard(ptr_gb, row, col);
        /* GIOCATORE 1 */
        if (getpid() == ptr_playersPid->player1) {
            printf("\n<%s::%d> Inserire la colonna di gioco: ",
                   ptr_playersPid->player1Name, ptr_playersPid->player1);
            do {
                flag = 1;
                scanf("%d", &move);

                if (!checkValidity(ptr_gb, move, ptr_gb->rows, ptr_playersPid->player1Token)) {
                    printf("<F4Client> Colonna non valida, riprova: ");
                } else {
                    flag = 0;       // ho inserito il token nella colonna desiderata
                }
            } while (flag);
            printBoard(ptr_gb, row, col);
            /* Libero il server e attendo il turno */
            semOp(semid, 0, 1);
            semOp(semid, 1, -1);
        } else {    /* GIOCATORE 2 */
            printf("\n<%s::%d> Inserire la colonna di gioco: ",
                   ptr_playersPid->player2Name, ptr_playersPid->player2);
            do {
                flag = 1;
                scanf("%d", &move);
                if (!checkValidity(ptr_gb, move, ptr_gb->rows, ptr_playersPid->player2Token)) {
                    printf("<F4Client> Colonna non valida, riprova: ");
                } else {
                    flag = 0;       // ho inserito il token nella colonna desiderata
                }
            } while(flag);
            printBoard(ptr_gb, row, col);
            /* Libero il server e attendo il turno */
            semOp(semid, 0, 1);
            semOp(semid, 2, -1);
        }

    } while (!ptr_winCheck->end);

    if (ptr_winCheck->player1Win) {
        printf("<F4Client> %s ha vinto la partita!\n");
    }

    semOp(semid, 2, 1);     // libero il player 2;
    return 0;
}
