/** Dati due array di pari dimensione si definisca l'operazione ps(v1, v2) come:
 *      v1[0] * v2[0] + v1[1] * v2[1] + ... + v1[n] * v2[n] 
 * Realizzare un'applicazione C basta su memoria condivisa e piu' processi
 * (1 padre e tanti figli del processo padre) per parallelizzare il calcolo 
 * sopra descritto.
 * Ogni processo figlio dovra' svolgere il conteggio v1[x] * v2[x] a lui assegnato,
 * mentre il processo padre svolbera' la somma finale e stampera' il risultato
 * complessivo.
*/

#include <stdio.h>
#include <stdlib.h>

#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/wait.h>

#define ARR_SIZE 10

int main(int argc, char *argv[]) {

    /* Puo essere anche un array direttamente */
    struct shared {
        int data[ARR_SIZE];
    };

    int sum = 0;

    int v1[ARR_SIZE] = {1,2,3,4,5,6,7,8,9,10};;
    int v2[ARR_SIZE] = {1,2,3,4,5,6,7,8,9,10};;

    /* Creazione della Shared Memory, necessitiamo di una chiave e di una dimensione */
    // Lasciamo al S.O. la scelta della chiave
    int shmid = shmget(IPC_PRIVATE, sizeof(struct shared), IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1) {
        printf("shmget failed\n");
        exit(1);
    }

    // Linkiamo la memoria condivisa ad un puntatore
    /* shmat 'attacca' la memoria ad un identificativo */
    struct shared *ptr_sh = shmat(shmid, 0, 0);


    /* Creiamo i processi figli */
    pid_t pid;
    for (int i = 0; i < ARR_SIZE; i++) {
        pid = fork();

        if (pid == -1) {
            printf("child %d not created\n", i);
        } else if (pid == 0) {
            // Il figlio esegue il calcolo e poi ritorna al padre e muore.
            ptr_sh->data[i] = v1[i] * v2[i];
            printf("<Child %d> %d * %d = %d\n", getpid(), v1[i], v2[i], ptr_sh->data[i]);

            exit(0);
        }
    }

    int status = 0;
    // Attesa che tutti i processi figli abbiano finito
    while( (pid = wait(&status)) != -1);

    for (int i = 0; i < ARR_SIZE; i++) {
        sum += ptr_sh->data[i];
    }

    /* Stacchiamo la memoria condivisa (detach) e poi la eliminiamo */
    if (shmdt(ptr_sh) == -1) {
        printf("shmdt failed");
        exit(1);
    }

    printf("<Parent> Result: %d\n", sum);

    return 0;
}





