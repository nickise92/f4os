#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>

#include "structdef.h"

int msqSrv = -1;
int msqCli = -1;

int main(int argc, char *argv[]) {

    // Message queue key for receiving from client
    key_t serverKey = 100;


    // Shared memory key
    key_t shmKey = 5070;

    int row = 5;
    int col = 5;

    size_t boardSize = sizeof(struct shared);
    int shmid = shmget(shmKey, boardSize, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }


    struct shared * ptr_gb = shmat(shmid, NULL, 0);

    for (int i = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            ptr_gb->board[i][j] = 'x';
        }
    }

    // Attendo connessione del client
    printf("<Server> In attesa della connessione del client...\n");

    msqSrv = msgget(serverKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqSrv == -1) {
        perror("msgget failed");
        exit(1);
    }

    struct message msg;
    msg.mtype = 1;

    size_t mSize = sizeof(struct message) - sizeof(long);
    if (msgrcv(msqSrv, &msg, mSize, 0, 0) == -1) {
        perror("msgrcv failed");
        exit(1);
    }

    printf("<Server> Client connected. User: %s\n", msg.content);



    while(1);
    return 0;
}