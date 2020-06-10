#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

/*Non c'è la riga 0*/
typedef struct {
    int from;
    int to;
    char event;
    char **bkp;
    struct status *prew;
} stack;

int current_size = 0;
char **testo;
int undo_size = 0;
int redo_size = 0;
stack *undo_stack = NULL;
stack *redo_stack = NULL;

/*Ricordati sta roba per non intasare la memoria*/
void svuota_pila() {}

void print_stack(stack *toPrint) {
    do {
        printf("From: %d\tTo: %d\n", toPrint->from, toPrint->to);
        for (int i = 0; i < (toPrint->to - toPrint->from); i++) {
            printf("?? %d %d\n", toPrint->from, toPrint->to);
            printf("B %d < %d\n", i, (toPrint->to - toPrint->from));
            if (toPrint->bkp[i] == NULL) {
                printf(".\n");
            } else {
                printf("%s\n", toPrint->bkp[i]);
            }
        }
        toPrint = toPrint->prew;
    } while (toPrint != NULL);
}

/*Stampa lo stato del file di testo*/
void print(int from, int to) {
    from--;
    to--;
    for (int i = from; i <= to; i++) {
        if (i >= current_size || i < 0) {
            printf(".\n");
        } else {
            printf("%s", testo[i]);
            //printf("%d\n", i);
        }
    }
}

char **dump_backup(int from, int to) {
    char **bkp;
    bkp = malloc((to - from) * sizeof(char *));
    //printf("from: %d\tTo: %d\n", from, to);
    int j = 0;
    for (int i = from; i < to; i++) {
        if (bkp[j] == NULL) {
            bkp[j] = malloc(1024 * sizeof(char));
        }
        strcpy(bkp[j], testo[i]);
        j++;
    }
    return bkp;
}

/*Ad una modifica o cancellazione salva lo stato delle righe
 * intaccate all'interno dell'apposita struttura dati per l'undo*/
void save_backup_undo(char **bkp, char cmd, int from, int to) {
    //printf("Salvataggio:\n");
    //printf("operazione:\t");
    stack *old_stat = undo_stack;
    undo_stack = malloc(sizeof(stack));
    undo_stack->prew = old_stat;
    undo_stack->from = from;
    undo_stack->to = to;
    undo_stack->event = cmd;
    undo_stack->bkp = bkp;
    undo_size++;
    //printf("Old status from row %d to row %d\n", from, to);
    for (int i = 0; i < (to - from); i++) {
        //printf("%d  %s\n", from+i, bkp[i]);
    }
    /*Da togliere*/
    //free(bkp);
}

void restore_backup(stack *stato) {
    int j = stato->from;
    for (int i = 0; i < (stato->to - stato->from); i++) {
        free(testo[j]);
        /*Se non va fare strcpy*/
        testo[j] = stato->bkp[i];
        j++;
    }
    free(stato);
}

void save_backup_redo(int from, int to) {
    stack *old_stat = redo_stack;
    redo_stack = malloc(sizeof(stack));
    redo_stack->prew = old_stat;
    redo_stack->from = from;
    redo_stack->to = to;
    redo_stack->event = 'r';
    redo_stack->bkp = dump_backup(from, to);
    redo_size++;
}

void undo(int steps) {
    stack *s = undo_stack;
    for (int i = 0; s != NULL && i < steps; i++) {
        save_backup_redo(s->from, s->to);
        restore_backup(s);
        s = s->prew;
    }
}

void redo(int steps) {

}

/*Data la riga vuota (empty) shifta tutte le righe
 * successive in alto di 1 posizione per coprirla*/
void compress(int empty) {
    for (int i = empty; i < current_size; i++) {
        testo[i] = testo[i + 1];
    }
}

/*Cancella le righe specificate come args e chiama
 * - compress per compattare i "buchi"
 * - save_backup per salvare lo stato prima della cancellazione*/
void delete(int from, int to) {
    //printf("%d > %d\n", from, current_size);
    if (from > current_size) {
        return;
    }
    from--;
    char **bkp;
    int j = 0;
    if (to > current_size) {
        to = current_size;
    }
    bkp = dump_backup(from, to);

    for (int i = from; i < to; i++, j++) {
        //printf("riga: %d\tbkp: %d\n", i, j);
        /*bkp[j] = malloc(1024 * sizeof(char));
        strcpy(bkp[j], testo[from]);*/
        free(testo[from]);
        compress(from);
        testo[current_size - 1] = NULL; //FREE?
        current_size--;

    }
    save_backup_undo(bkp, 'd', from, to);
    testo = realloc(testo, current_size * sizeof(char *));
}

/*Sostituisce il testo esistente in una riga con quello richiesto all'utente
 * se la riga non esiste ne crea una nuova
 * se la riga esiste chiama save_backup per salvare il vecchio stato
 * salva anche se la riga vecchia era vuota e la etichetta NULL*/
void change(int from, int to) {
    char **bkp;
    from--;
    if (to > current_size) {
        bkp = dump_backup(from, current_size);
        testo = realloc(testo, to * sizeof(char *));
        current_size = to;
    } else {
        bkp = dump_backup(from, to);
    }

    char f[10];
    fgets(f, 10, stdin);
    int j = 0;
    for (int i = from; i <= to; i++, j++) {
        char c[1024];
        fgets(c, 1024, stdin);
        if (strcmp(c, ".\0") == 0 || i == to) {
            break;
        } else {
            if (testo[i] == NULL) {
                testo[i] = malloc(1024 * sizeof(char));
            }
            strcpy(testo[i], c);
        }
    }
    save_backup_undo(bkp, 'c', from, to);

}

/*Interpreta i comandi dell'utente
 * nei formati
 * - 5c
 * - 2,5d
 * e chiama le opportune funzioni*/
bool interpreta(char *input) {
    int length = (int) strlen(input);
    char cmd;
    cmd = input[length - 1];
    printf("cmd: %c\n", cmd);
    input[length - 1] = '\0';
    char *token = strtok(input, ",");
    int from = atoi(token);
    printf("from: %d\n", from);
    int to;
    token = strtok(NULL, ",");
    if (token != NULL) {
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
            printf("Undo %d\n", from);
            undo(from);
            break;
        case 'r':
            break;
        default:
            return false;
    }
    return true;
}

int main() {
    char *input;
    while (true) {
        scanf("%ms", &input);
        //printf("%s\n", input);
        if (input[0] == 'q') {
            break;
        } else if (input[0] == 's') {
            print_stack(undo_stack);
        } else {
            interpreta(input);
        }
    }
    return 0;
}
