#include <stdlib.h>
#include <stdbool.h>

struct shared_board {
    char board[100][100];
    int rows;
    int cols;
};

struct shared_pid {
    bool first;
    char player1Name[100];
    pid_t player1;
    char player1Token;
    char player2Name[100];
    pid_t player2;
    char player2Token;
};

struct winning {
    int playerLeft;     // == pid giocatore -> perde per abbandono
    bool player1Win;    // giocatore 1 ha vinto =1
    bool player2Win;    // giocatore 2 ha vinto =1
    bool full;          // la matrice per il campo da gioco è piena
                        // la partita finisce pari
    bool end;
};