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


#include "errExit.h"

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
        errExit(msg);
    }

    // Allochiamo la matrice che funge da campo di gioco in uno spazio di memoria condivisa
    key_t keyShmGameBoard = 123456;
    size_t gameBoardSize = sizeof(char *) * row;
    int shmBoardID = shmget(keyShmGameBoard, gameBoardSize, IPC_CREAT | IPC_EXCL);
    if ( shmBoardID == -1 ){
        errExit("Creation of shared memory failed!");
    }

    char ** gameBoard;
    gameBoard = (char **) malloc (sizeof(char *) * row);
    for (int i = 0; i < col; i++) {
        gameBoard[i] = (char *) malloc (sizeof(char) * col);
    }




}