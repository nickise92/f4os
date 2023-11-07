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

int main(int argc, char * argv[]) {
    
    // Inizializzare il nome dell'utente: verifica che il numero di argomenti
    // sia corretto
    if (argc < 2) {
        printf("Errore! Inserire il nome utente per avviare una sessione di gioco\n");
        exit(EXIT_FAILURE);
    }
    
    char * playerName = argv[1];

    printf("Nome utente: %s", playerName);

    printf("\n");
    return 0;   
}