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
#include <stdbool.h>

#include "errExit.h"
#include "message.h"
#include "shmbrd.h"
#include "semaphore.h"

/* Variabili globali */
int count_sig = 0;
struct shared_pid *ptr_playersPid;
struct shared_board *ptr_gb;
struct winning *ptr_winCheck;
int semid;
bool endGame = false;


/* Funzione di verifica vittoria o sconfitta */
void checkGameStatus() {
    /* Verifico se un giocatore ha vinto */

    // TODO: Vanno spostati nell'handler di gestione SIGINT e non qui
    // Caso di abbandono del gicatore 1:
    if (ptr_winCheck->playerLeft == ptr_playersPid->player1) {
        ptr_winCheck->player2Win = true;
        ptr_winCheck->player1Win = false;
        printf("<F4Server> %s vince per abbandono di %s.\n", ptr_playersPid->player2Name, ptr_playersPid->player1Name);
    }

    // Caso di abbandono del giocatore 2:
    if (ptr_winCheck->playerLeft == ptr_playersPid->player2) {
        ptr_winCheck->player2Win = false;
        ptr_winCheck->player1Win = true;
        printf("<F4Server> %s vince per abbandono di %s.\n", ptr_playersPid->player1Name, ptr_playersPid->player2Name);
    }

    /* Verifica di vincita */
    // Verifica colonne
    printf("<F4Server> Verifico le colonne...\n");

    int j = 0;                  // prima colonna
    int k = 1;
    int i = ptr_gb->rows - 1;   // ultima riga, parto a verificare dal basso, quindi indice piu' alto

    char token;
    int count = 1;

    /* Verifico tutte le colonne partendo dalla prima */
    bool flag = true;
    while (j < ptr_gb->cols && flag) {
        token = ptr_gb->board[i][j];    // considero il token presente nella prima casella
        while (i - k >= 0) {
            /* Controlliamo ogni riga della colonna corrente (indice j) */
            if (ptr_gb->board[i - k][j] == token) {
                count++;
                k++;        // aumentiamo k solo in questo caso
            } else {
                count = 1;                      // reset il count a 1
                i = i - k;                      // aggiorniamo la riga
                token = ptr_gb->board[i][j];    // aggiorniamo il token
                k = 1;                          // reset a k
            }

            if (count == 4 && token != ' ') {
                if (token == ptr_playersPid->player1Token) {
                    printf("<F4Server> Il vincitore è %s!\n", ptr_playersPid->player1Name);
                    ptr_winCheck->player1Win = true;
                    ptr_winCheck->player2Win = false;
                    endGame = true;
                } else if (token == ptr_playersPid->player2Token) {
                    printf("<F4Server> Il vincitore è %s!\n", ptr_playersPid->player2Name);
                    ptr_winCheck->player1Win = false;
                    ptr_winCheck->player2Win = true;
                    endGame = true;
                }

                flag = false;
            }
        }
        /* Reset indici di controllo prima di analizzare
         * la colonna successiva */
        j++;
        i = ptr_gb->rows - 1;
        k = 1;
        count = 1;
    }

    /* Verifico tutte le righe partendo dalla prima (indice i piu' alto) */
    printf("<F4Server> Verifico le righe...\n");

    // Aggiorno valori indici di controllo
    j = 0;
    k = 1;
    i = ptr_gb->rows - 1;
    count = 1;
    /* Nota: non serve reimpostare flag = true, perché se arrivo a questo punto
     * e non ho trovato colonne vincenti devo verificare le righe. Se ho già
     * trovato delle colonne vincenti, non devo entrare nel ciclo di verifica
     * delle righe perché ho già trovato un vincitore. */

    while (i >= 0 && flag) {
        token = ptr_gb->board[i][j];
        while (j + k <= ptr_gb->cols && flag) {
            /* Verifichiamo tutte le colonne nella riga corrente */
            if (ptr_gb->board[i][j+k] == token) {
                count++;
                k++;
            } else {
                count = 1;
                j = j + k;
                token = ptr_gb->board[i][j];
                k = 1;
            }

            if (count == 4 && token != ' ') {
                if (token == ptr_playersPid->player1Token) {
                    printf("<F4Server> Il vincitore è %s!\n", ptr_playersPid->player1Name);
                    ptr_winCheck->player1Win = true;
                    ptr_winCheck->player2Win = false;
                    endGame = true;
                } else if (token == ptr_playersPid->player2Token) {
                    printf("<F4Server> Il vincitore è %s!\n", ptr_playersPid->player2Name);
                    ptr_winCheck->player1Win = false;
                    ptr_winCheck->player2Win = true;
                    endGame = true;
                }

                flag = false;
            }
        }

         /* Ripristino gli indici di controllo per la
         * verifica della riga successiva */
        i--;
        j = 0;
        k = 1;
        count = 1;
    }

    /* Se il flag è ancora a true, nessuna riga e nessuna colonna sono vincenti.
     * Verifichiamo se la matrice è piena o se è ancora possibile giocare */
    for (i = ptr_gb->rows - 1 && !ptr_winCheck->full; i >= 0; i--) {
        for (j = 0; j < ptr_gb->cols; j++) {
            if (ptr_gb->board[i][j] == ' ') {
                ptr_winCheck->full = false;
            }
        }
    }

    if (ptr_winCheck->full) {
        printf("<F4Server> Il campo di gioco è pieno. La partita finisce pari.\n");
        endGame = true;
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

void sigPlayerLeft(int sig) {
    printf("<F4Server> Un giocatore ha deciso di abbandonare la partita.\n");

    /* Se il giocatore 1 ha abbandonato la partita, invio SIGUSR2 al giocatore 2 */
    if (ptr_winCheck->playerLeft == ptr_playersPid->player1) {
        if (kill(ptr_playersPid->player2, SIGUSR2) == SIG_ERR) {
            errExit("kill failed");
        }
    }

    /* Se il giocatore 2 ha abbandonato la partita, invio SIGUSR2 al giocatore 1 */
    if (ptr_winCheck->playerLeft == ptr_playersPid->player2) {
        if (kill(ptr_playersPid->player1, SIGUSR2) == SIG_ERR) {
            errExit("kill failed");
        }
    }


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
}

int main(int argc, char * argv[]) {

    /* GESTIONE SIGINT (CTRL-C) */
    if (signal(SIGINT, sigHandler) == SIG_ERR) {
        errExit("change signal SIGINT handler failed");
    }

    /* Handler abbandono di un giocatore */
    if (signal(SIGUSR1, sigPlayerLeft) == SIG_ERR) {
        errExit("change signal SIGUSR1 handler failed");
    }

    /* Chiavi per la memoria condivisa */
    key_t boardKey = 5090; // Chiave per lo spazio di memoria condivisa su cui e' presente il campo di gioco
    key_t pidKey = 6050; // Chiave per l'accesso al pid dei giocatori
    key_t winKey = 4070; // Chiave per l'accesso alla struttura winning

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
    ptr_playersPid->serverPid = getpid();

    /********************** ALLOCAZIONE MEMORIA CONDIVISA STRUTTURA WINNING **********************/
    size_t winSize = sizeof(struct winning);
    int shWinID = shmget(winKey, winSize, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shWinID == -1) {
        errExit("shmget winning failed");
    }

    ptr_winCheck = shmat(shWinID, NULL, 0);
    ptr_winCheck->player1Win = false;
    ptr_winCheck->player2Win = false;
    ptr_winCheck->full = false;
    ptr_winCheck->end = true;

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

    do {
        semOp(semid, (unsigned short) 1, 1);    // turno giocatore 1
        printf("<F4Server> Turno di %s\n", ptr_playersPid->player1Name);
        semOp(semid, (unsigned short) 0, -1);   // server attende
        printf("<F4Server> Controllo tornato al Server...\n");
        // Verifica lo stato della partita
        printf("<F4Server> Verifica lo stato della partita...\n");
        checkGameStatus();
        semOp(semid, (unsigned short) 2, 1);     // turno giocatore 2
        printf("<F4Server> Turno di %s\n", ptr_playersPid->player2Name);
        semOp(semid, (unsigned short) 0, -1);   // server attende
        printf("<F4Server> Controllo tornato al Server...\n");
        // Verifica lo stato della partita
        printf("<F4Server> Verifica lo stato della partita...\n");
        checkGameStatus();

         /* Continuo ad alternare i turni fin quando nella struttura "winning" ho
         * !true && false oppure false && !true, ovvero 1 giocatore avra' vinto e
         * l'altro no. */
    } while (!endGame);

    /********************** TERMINAZIONE PARTITA E CHIUSURA SHARED MEMORY LATO SERVER **********************/
    /* Libero il player 1 che deve comunicare che la partita e' terminata e chiudersi */
    semOp(semid, (unsigned short) 1, 1);
    /* Attendo il ritorno del controllo al Server */
    semOp(semid, (unsigned short) 0, -1);
    /* Libero il player 2 che comunica anch'esso la terminazione della partita e si chiude */
    semOp(semid, (unsigned short) 2, 1);
    /* Attendo il ritorno del controllo per eseguire la chiusura delle memorie condivise lato Server */
    semOp(semid, (unsigned short) 0, -1);

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
    if (shmdt(ptr_winCheck) == -1) {
        errExit("shmdt failed");
    } else {
        printf("<F4Server> Memoria condivisa informazioni sullo stato della partita rimossa con successo.\n");
    }

    /* Rimozione dei semafori */
    if (semctl(semid, 0, IPC_RMID, 0) == -1) {
        errExit("semctl failed");
    } else {
        printf("<F4Server> Semafori rimossi con successo.\n");
    }

    return 0;
}
