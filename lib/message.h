/**********************************
 * VR456714,
 * Niccolò Iselle
 * 2023-11-16
 */
#include <stdlib.h>

struct message {
    long mtype;
    pid_t pid;
    int row;
    int col;
    char token;
    char content[100];
};
