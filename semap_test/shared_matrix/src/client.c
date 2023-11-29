#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/msg.h>

#include "structdef.h"


int main(int argc, char *argv[]) {

    // Message queue key for receiving from client
    key_t serverKey = 100;


    // Shared memory key
    key_t shmKey = 5070;

    if (argc < 2) {
        printf("Errore, argomento mancante!\n");
        exit(0);
    }

    int msqSrv = msgget(serverKey, S_IRUSR | S_IWUSR);
    if (msqSrv == -1) {
        perror("msgget failed");
        exit(1);
    }

    int msqCli = msgget(serverKey, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (msqCli == -1) {
        perror("msgget failed");
        exit(1);
    }

    struct message msg;
    msg.mtype = 1;

    strcpy(msg.content, argv[1]);

    size_t mSize = sizeof(struct message) - sizeof(long);
    printf("<Client> Connecting to the server...\n");

    if (msgsnd(msqSrv, &msg, mSize, 0) == -1) {
        perror("msgsend failed");
        exit(1);
    }

    printf("<Client> Printing board...\n");
    size_t boardSize = sizeof(struct shared);

    int shmid = shmget(shmKey, boardSize, S_IWUSR | S_IRUSR);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    struct shared * ptr_gb = shmat(shmid, 0, 0);

    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (j == 0) {
                printf("| %c |", ptr_gb->board[i][j]);
            } else {
                printf(" %c |", ptr_gb->board[i][j]);
            }
        }
        printf("\n");
    }

    return 0;
}