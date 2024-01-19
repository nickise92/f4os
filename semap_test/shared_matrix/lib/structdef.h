struct shared {
    char board[100][100];
};

struct message {
    long mtype;
    size_t size;
    char content[100];
};