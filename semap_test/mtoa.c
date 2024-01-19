#include <stdio.h>
#include <stdlib.h>

#define ROW 5
#define COL 5

int main(int argc, char *argv[]) {

    int ** boardMatrix; 

    boardMatrix = (int**) malloc(sizeof(int*) * ROW);
    for (int i = 0; i < ROW; i++) {
        boardMatrix[i] = (int*) malloc(sizeof(int) * COL);
    }
    
    for(int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            boardMatrix[i][j] = i+j;
        }
    }

    printf("Dimensione matrice: %ld\n", sizeof(char) * COL * ROW);

    int * array;

    array = (int*) malloc(sizeof(int)*COL*ROW);

    for (int i = 0; i < ROW*COL; i++) {
        array[i] = 0;
    }
   

    printf("Dimensione array: %ld\n", sizeof(int)*COL*ROW);

    int row, col, counter=0;
    do {
        printf("Inserire una cella su cui posizionare il token.\nRiga: ");
        scanf("%d", &row);
        if (row > ROW || row < 0) {
            printf("Errore! Questa riga non esiste!\n");
            exit(0);
        }
        printf("Colonna: ");
        scanf("%d", &col);
        if (col > COL || col < 0) {
            printf("Errore! Questa colonna non esiste!\n");
            exit(0);
        }

        if (array[(row*COL)+col] == 0) {
            boardMatrix[row][col] = 'x';
            array[(row*COL)+col] = 1;
            counter++;
        } else {
            printf("Non puoi giocare in questa casella, e' gia' occupata!\n");
        }



        for(int i = 0; i < ROW; i++) {
            for (int j = 0; j < COL; j++) {
                if (j == 0) {
                    printf("| %c |", boardMatrix[i][j]);
                } else {
                    printf(" %c |", boardMatrix[i][j]);
                }
            }
            printf("\n");
        }

        for(int i = 0; i < ROW; i++) {
            for (int j = 0; j < COL; j++) {
                printf("%3d, ", array[i+j]);
            }
        }
        printf("\n");

    } while (counter<=25);

    return 0;

}