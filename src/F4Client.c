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

void printBoard(char **, int, int);

int main(int argc, char * argv[]) {

    /* Verifica che il numero di argomenti al lancio del gioco sia corretto */
    if (argc < 2) {
        const char *msg = "Errore! Inserire il nome utente per avviare una sessione di gioco\n";
        errExit(msg);
    }

    char * playerName = argv[1];

    printf("Giocatore 1: %s\n", playerName);

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