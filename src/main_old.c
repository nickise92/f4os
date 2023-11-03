#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int toInteger(char * a) {
    int result = 0;
    int len = strlen(a);

    for (int i = 0; i < len; i++) {
        /* Conversione dell'argomento (char *) in 
         * numero intero 
        */
       result *= 10;
       result += a[i] - 48;

    }

    return result;
}

// Stampa il campo di gioco
void printBoard( char ** brd, int _row, int _col) {
    for (int i = 0; i < _row; i++) {
        printf("|");
        for (int j = 0; j < _col; j++) {
            printf(" %c |", brd[i][j]);
        }
        printf("\n");
    }
}

void updateGameBoard( char ** brd, int _row, int _col, int xpos, int ypos, char _player) {
    if (brd[xpos][ypos] != ' ') {
        brd[xpos][ypos] = _player;
    }
    printBoard(brd, _row, _col);
}

int main (int argc, char * argv[]) {

    /* Variabili */
    int col, row;           // colonne e righe del campo
    char ** gameBoard;      // campo di gioco, necessita di un semaforo per l'accesso tra server e clients
    char *p1, *p2;          // simboli dei due giocatori
    char *xpos, *ypos;      // stringa su cui salvare la mossa del giocatore nel formato 'x y'
    int x, y;               // posizione x,y in formato intero per definire la mossa


    p1 = argv[3];
    p2 = argv[4];
    // Gestione argomenti mancanti quando viene lanciato il programma
    if (argc < 5) {
        printf("Errore, argomenti non sufficienti per lanciare il programma.");
        printf("\nFunzionamento:\n./main <row> <col> <p1> <p2>\nEsempio: ./main 5 5 O X\n");
        exit(EXIT_FAILURE);
    }
    
    //printf("Result = %d\n", toInteger(argv[1]));
    // Conversione ad interi delle dimensioni del campo
    row = toInteger(argv[1]);
    col = toInteger(argv[2]);


    // Generazione del campo:
    gameBoard = (char **) malloc (sizeof(char *) * row);
    for (int i = 0; i < row; i++) {
        gameBoard[i] = (char *) malloc (sizeof(char) * col);
    }
    printf("Player 1: '%s';\nPlayer 2: '%s';\n", p1, p2 );
    // Inizializzazione del campo a caratteri vuoti:
    for (int i = 0; i < row; i ++) {
        for (int j = 0; j < col; j++) {
            gameBoard[i][j] = ' ';
        }
    }
    // printBoard(gameBoard, row, col);
    
    // Creazione di un processo figlio che genera una mossa casuale
    // il padre attende la mossa per poi visualizzarla sullo schermo.
    pid_t pid = fork();
    

    if (pid == -1) {
        exit(EXIT_FAILURE);
    }
    
    if (pid == 0) { // Child process
        printBoard(gameBoard, row, col);
        // Mossa del giocatore
        printf("Inserisci la riga: ");
        scanf("%s", xpos);
        printf("Inserisci la colonna: ");
        scanf("%s", ypos);    
    
        printf("Selezionata casella %s, %s\n", xpos, ypos);
        
    }
    
    pid_t waitChild = wait(NULL);
    if (waitChild == -1) {
        exit(EXIT_FAILURE);
    }


    // Mossa del giocatore 1
    // int x, y;
    // int flag = 0;
    // do {
    //     printf("Inserisci la riga: ");
    //     scanf("%s", xpos);
    //     printf("Inserisci la colonna: ");
    //     scanf("%s", ypos);
    //     x = toInteger(xpos);
    //     y = toInteger(ypos);
    //     if (x > row && y > col) {
    //         printf("Errore! Il punto selezionato e' fuori dallo spazio di gioco!");
    //         flag = 1;
    //     }
    // } while (flag);
       
    printBoard(gameBoard, row, col);
    // updateGameBoard(gameBoard,row, col, x, y, p1[0]);    // dev'essere fatta dal client (quindi dal figlio)
    printf("Mossa eseguita, il figlio ha terminato.\n");

    return 0;
}