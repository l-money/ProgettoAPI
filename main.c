#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

int current_size = 1;
char **testo;

/*Ricordati sta roba per non intasare la memoria*/
void svuota_pila() {}

void print(int from, int to) {
    from--;
    to--;
    for (int i = from; i <= to; i++) {
        if (i >= current_size) {
            printf(".\n");
        } else {
            printf("%s\n", testo[i]);
        }
    }
}

void save_backup(char** bkp, char cmd, int from, int to){
    printf("Salvataggio:\n");
    printf("operazione:\t");
    switch(cmd){
        case 'c':
            printf("c\n");
            break;
        case 'd':
            printf("d\n");
            break;
    }
    printf("Old status from row %d to row %d\n", from, to);
    for(int i=0; i<(to-from); i++){
        printf("%d  %s\n", from+i, bkp[i]);
    }
}

void compress(int empty) {
    for (int i = empty; i < current_size; i++) {
        testo[i] = testo[i + 1];
    }
}

void delete(int from, int to) {
    printf("%d > %d\n", from, current_size);
    if (from > current_size) {
        return;
    }
    from--;
    char **bkp;
    bkp = malloc((to - from + 1) * sizeof(char *));
    int j = 0;
    if (to > current_size) {
        to = current_size;
    }
    for (int i = from; i < to; i++, j++) {
        printf("riga: %d\tbkp: %d\n", i, j);
        bkp[j] = malloc(1024 * sizeof(char));
        strcpy(bkp[j], testo[from]);
        free(testo[from]);
        compress(from);
        testo[current_size - 1] = NULL;
        current_size--;
    }
    save_backup(bkp, 'd', from, to);
}

void change(int from, int to) {
    bool overwrite = false;
    if (to > current_size) {
        testo = realloc(testo, to * sizeof(char *));
        current_size = to;
    }
    from--;
    char f[10];
    gets(f);
    char **bkp;
    bkp = malloc((to - from + 1) * sizeof(char *));
    int j=0;
    for (int i = from; i <= to; i++, j++) {
        char c[1024];
        gets(c);
        if (strcmp(c, ".\0") == 0 || i == to) {
            break;
        } else {
            if (testo[i] == NULL) {
                testo[i] = malloc(1024 * sizeof(char));
            }else{
                bkp[j] = malloc(1024 * sizeof(char));
                strcpy(bkp[j], testo[i]);
                overwrite = true;
            }
            strcpy(testo[i], c);
        }
    }
    if(overwrite){
        save_backup(bkp, 'c', from, to);
    }else{
        free(bkp);
    }
}

bool interpreta(char *input) {
    int length = (int) strlen(input);
    char cmd;
    cmd = input[length - 1];
    input[length - 1] = '\0';
    char *token = strtok(input, ",");
    int from = atoi(token);
    int to;
    if (token != NULL) {
        token = strtok(NULL, ",");
        to = atoi(token);
    } else {
        to = atoi(token);
    }
    printf("From: %d\tTo: %d\tCmd: %c\n", from, to, cmd);
    switch (cmd) {
        case 'c':
            change(from, to);
            break;
        case 'p':
            print(from, to);
            break;
        case 'd':
            delete(from, to);
            break;
        case 'u':
            break;
        case 'r':
            break;
        default:
            return false;
    }
    return true;
}

int main() {
    testo = malloc(sizeof(char *));
    char *input;
    while (true) {
        scanf("%ms", &input);
        //printf("%s\n", input);
        if (input[0] == 'q') {
            break;
        }
        interpreta(input);
    }
    return 0;
}
