/**********************************
 * VR456714,
 * Niccolò Iselle
 * 2023-11-16
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/stat.h>
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

/* Il Client è responsabile della stampa del campo di gioco 'ptr_sh' è il puntatore
 * alla matrice in memoria condivisa che rappresenta il tabellone
 */
void printBoard(struct shared_board  *ptr_sh) {

    for (int i = 0; i <  ptr_sh->rows; i++) {
        for (int j = 0; j < ptr_sh->cols; j++) {
            if (j == 0) {
                printf("| %c |", ptr_sh->board[i][j]);
            } else {
                printf(" %c |", ptr_sh->board[i][j]);
            }
        }
        printf("\n");
    }
    // stampo il numero delle colonne sotto al tabellone
    for (int i = 1; i <= ptr_sh->cols; i++) {
        printf("  %d ", i);
    }
    printf("\n");
}

/* Handler del segnale di interruzione Ctrl+C */
void sigHandler(int sig) {
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("signal handler failed");
    }

    if (count_sig == 0) {
        printf("\n<F4Server> Attenzione pressione CTRL+C rilevata. Un'ulteriore pressione comporta la chiusura del gioco!\n");
        count_sig++;
    } else if (count_sig == 1) {
        printf("\n<F4Server> Gioco terminato dal Server.\n");

        /* Informo il server che il processo client corrente ha abandonato */
        ptr_winCheck->playerLeft = getpid();
        if (kill(ptr_playersPid->serverPid, SIGUSR1) == -1) {
            errExit("kill SIGUSR1 failed");
        }

        exit(0);
    }
}

void sigHandlerPlayerLeft(int sig) {

    if (ptr_playersPid->player1 == ptr_winCheck->playerLeft) {
        printf("\n<F4Client> Hai vinto per abbandono di %s.\n", ptr_playersPid->player1Name);
    } else {
        printf("\n<F4Client> Hai vinto per abbandono di %s.\n", ptr_playersPid->player2Name);
    }

    exit(0);
}

int main(int argc, char * argv[]) {

    /* Handler CTRL-C */
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("signal handler failed");
    }

    /* Handler abbandono partita di un giocatore */
    if (signal(SIGUSR2, sigHandlerPlayerLeft) == SIG_ERR) {
        errExit("signal handler failed");
    }

    /* VARIABILI LOCALI CLIENT */

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090;  // Chiave per lo spazio di memoria condivisa su cui è presente il campo di gioco
    key_t pidKey = 6050;    // Chiave per l'accesso al pid dei giocatori.
    key_t winKey = 4070;    // Chiave per l'accesso alla struttura winning

    /* Accesso alla memoria condivisa per il campo di gioco */
    size_t boardSize = sizeof(struct shared_board);
    int shBoardID = shmget(boardKey, boardSize, S_IRUSR | S_IWUSR);
    if (shBoardID == -1) {
        errExit("shmget shared_board failed");
    }

    ptr_gb = shmat(shBoardID, 0, 0);

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
        errExit("shmget shared_pid failed");
    }

    ptr_playersPid = shmat(shPidID, 0, 0);

    /* Accesso alla meomria condivisa per la verifica dello stato partita */
    size_t winSize = sizeof(struct winning);
    int shWinID = shmget(winKey, winSize, S_IRUSR);
    if (shWinID == -1) {
        errExit("shmget winning failed");
    }

    ptr_winCheck = shmat(shWinID, 0, 0);

    /* Verifica che il numero di argomenti al lancio del gioco sia corretto */
    if (argc < 2) {
        const char *msg = "Errore! Inserire il nome utente per avviare una sessione di gioco\n";
        errExit(msg);
    }
    /* Verifichiamo se il client corrente è il primo giocatore: nel caso
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

        count_sig = 0;
        /* Visualizzo la board per il giocatore */
        printBoard(ptr_gb);
        /* GIOCATORE 1, se il game non è stato vinto da nessuno, esegui la giocata */
        if (getpid() == ptr_playersPid->player1 && !endGame) {

            printf("\n<%s::%d> Inserire la colonna di gioco: ",
                   ptr_playersPid->player1Name, ptr_playersPid->player1);
            do {
                flag = 1;
                scanf("%d", &move);
                move = move - 1;
                if (!checkValidity(ptr_gb, move, ptr_gb->rows, ptr_playersPid->player1Token)) {
                    printf("<F4Client> Colonna non valida, riprova: ");
                } else {
                    flag = 0;       // ho inserito il token nella colonna desiderata
                }
            } while (flag);
            printBoard(ptr_gb);     // visualizzo la giocata appena eseguita
            /* Libero il server che verifica se qualcuno ha vinto */
            semOp(semid, (unsigned short) 0, 1);
            semOp(semid, (unsigned short) 1, -1);    // attendo il mio turno
        }

        /* GIOCATORE 2 */
        if (getpid() == ptr_playersPid->player2 && !endGame) {

            printf("\n<%s::%d> Inserire la colonna di gioco: ",
                   ptr_playersPid->player2Name, ptr_playersPid->player2);
            do {
                flag = 1;
                scanf("%d", &move);
                move = move - 1;
                if (!checkValidity(ptr_gb, move, ptr_gb->rows, ptr_playersPid->player2Token)) {
                    printf("<F4Client> Colonna non valida, riprova: ");
                } else {
                    flag = 0;       // ho inserito il token nella colonna desiderata
                }
            } while(flag);
            printBoard(ptr_gb);     // visualizzo la giocata appena eseguita
            /* Libero il server che verifica se qualcuno ha vinto */
            semOp(semid, (unsigned short) 0, 1);
            semOp(semid, (unsigned short) 2, -1);   // attendo il mio turno
        }

        /* Verifico se qualcuno ha vinto prima di fare la giocata */
        printf("<F4Client> Verifica vincitore...\n");
        if (ptr_winCheck->player1Win || ptr_winCheck->player2Win || ptr_winCheck->full) {
            printBoard(ptr_gb);
            printf("<F4Client> Partita terminata!\n");
            endGame = true;
        } else {
            printf("<F4Client> Nessun vincitore trovato. Esegui la tua mossa!\n");
        }

    } while (!endGame);

    if (ptr_winCheck->player1Win) {
        printf("<F4Client> %s ha vinto la partita! %s ha perso!\n", ptr_playersPid->player1Name, ptr_playersPid->player2Name);
    } else if (ptr_winCheck->player2Win) {
        printf("<F4Client> %s ha vinto la partita! %s ha perso!\n", ptr_playersPid->player2Name, ptr_playersPid->player1Name);
    } else if (ptr_winCheck->full) {
        printf("Partita finita in parità. Campo di gioco pieno.\n");
    }
    semOp(semid, (unsigned short) 0, 1); // libero il server
    return 0;
}
