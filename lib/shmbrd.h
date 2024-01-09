#include <stdlib.h>

struct shared_board {
    char board[100][100];
};

struct shared_pid {
    char player1Name[100];
    pid_t player1;
    char player1Token;
    char player2Name[100];
    pid_t player2;
    char player2Token;
};