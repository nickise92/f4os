/************************************
* VR456714
* Niccolò Iselle
* 2/11/2023
*************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void printBoard(char **, int, int);

int main(int argc, char * argv[]) {
    
    // Inizializzazione del gioco
    // Creazione del campo di gioco:
    // Verifico che il programma sia lanciato con il numero corretto di
    // argomenti 
    if (argc < 3) {
        printf("Errore! Devi specificare la dimensione del campo di gioco.\n");
        printf("Uso: './F4Server <ROW> <COL> <P1> <P2>'\n");
        printf("Esempio: './F4Server 5 5 O X\n");
        exit(EXIT_FAILURE);
    }

    // Estrapoliamo in due variabili intere le righe e le colonne della matrice
    // che andremo a generare come campo di gioco.
    int row = atoi(argv[1]);
    int col = atoi(argv[2]);
    
    // Creaiamo il campo di gioco. NB: le dimensioni minime sono 5x5, quindi
    // verifichiamo che le dimensioni siano corrette prima di generare la matrice
    if (row < 5 || col < 5) {
        printf("Il campo minimo deve avere dimensioni 5x5.\n");
        exit(EXIT_FAILURE);
    }
    printf("Row = %d, Col = %d\n", row, col); // Debug
    
    // Generazione del campo
    char ** gameBoard;
    gameBoard = (char **) malloc(sizeof(char *) * row);
    for (int i = 0; i < row; i++) {
        gameBoard[i] = (char *) malloc(sizeof(char) * col);
    }
    // Inizializzazione del campo vuoto
    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            gameBoard[i][j] = ' ';
        }
    }

    printf("In attesa dei giocatori...\n");
    printBoard(gameBoard, row, col);

    // Semaforo bloccante: il server deve attendere che i client siano connessi
     


    // arbitrare la partita
    
    // determinare se qualcuno ha vinto

    // Notificare i client se non è più possibile
    // inserire gettoni
    

    printf("\n");
    return 0;
}


/* Procedura accessoria che permette di stampare a video la matrice */
void printBoard(char ** gB, int r, int c) {
    for (int i = 0; i < r; i++) {
        for (int j = 0; j < c; j++) {
            printf(" %c |", gB[i][j]);
        }
        printf("\n");
    }
}