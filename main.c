#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
/*Provo a committare*/
/*Non c'è la riga 0*/
typedef struct stack {
    int from;
    int to;
    int content_length; //provo sta cosa
    char event;
    char **bkp;
    struct stack *prew;
} stack;

int current_size = 0;
char **testo = NULL;
int undo_size = 0;
int redo_size = 0;
stack *undo_stack = NULL;
stack *redo_stack = NULL;
char *token = NULL, input[20];
char c[1024];
bool changed = false;

void free_stack(stack *s) {
    /*for (int i = 0; i < s->to - s->from; i++) {
        free(s->bkp[i]);
    }
    free(s->bkp);*/
    free(s);
}

/*Ricordati sta roba per non intasare la memoria*/
void empty_redo_stack() {
    stack *s;
    while (redo_stack != NULL) {
        s = redo_stack->prew;
        free(redo_stack->bkp);
        free(redo_stack);
        redo_stack = s;
    }
    redo_size = 0;
}

/*@DEBUG
 * stampa i dettagli di una specificata stack (undo o redo)
 * mostra gli indici di posizione nel testo
 * mostra il testo allo stato di riferimento*/
void print_stack(stack *toPrint) {
    int counter = 0;
    if (toPrint == NULL) {
        printf("Empty stack!\n");
        return;
    }
    do {
        printf("#%d\t", counter);
        printf("From: %d\tTo: %d\n", toPrint->from, toPrint->to);
        printf("Operation: %c\n", toPrint->event);
        for (int i = 0; i < (toPrint->to - toPrint->from); i++) {
            if (toPrint->bkp[i] == NULL) {
                printf(".\n");
            } else {
                printf("%s\n", toPrint->bkp[i]);
            }
        }
        toPrint = toPrint->prew;
        counter++;
    } while (toPrint != NULL);
}

/*Stampa lo stato del file di testo*/
void print(int from, int to) {
    from--;
    to--;
    //printf("from %d\tto %d\tsize %d\n", from, to, current_size);
    if (testo == NULL) {
        for (int i = from; i <= to; i++) {
            printf(".\n");
        }
        return;
    }
    for (int i = from; i <= to; i++) {
        if (i >= current_size || i < 0 || testo[i] == NULL) {
            printf(".\n");
        } else {
            printf("%s", testo[i]);
            //printf("%d\n", i);
        }
    }
}

/*Acquisisce una copia del file di testo tra le righe specificate
 * Questo metodo server per implementare il salvataggio per undo e redo
 * viene chiamato da delete, change, undo e redo*/
char **dump_backup(int from, int to) {
    char **bkp;
    //printf("from: %d\tTo: %d\n", from, to);
    if(to>current_size){
        to=current_size;
    }
    bkp = malloc((to - from) * sizeof(char *));
    int j = 0;
    for (int i = from; i < to /*&& i < current_size*/; i++) {
        if (i < current_size) {
            bkp[j] = testo[i];
        } else {
            bkp[j] = NULL;
        }
        j++;
    }

    return bkp;
}

/*Ad una modifica o cancellazione salva lo stato delle righe intaccate
 * dall'operazione all'interno dell'apposita struttura dati per l'undo*/
void save_backup_undo(char **bkp, char cmd, int from, int to, int oldsize) {
    //printf("Salvataggio:\n");
    //printf("operazione:\t");
    /*if (current_size == 0) {
        from = 0;
        to = 0;
    }*/
    stack *new_stat = malloc(sizeof(stack));
    new_stat->prew = undo_stack;
    new_stat->from = from;
    new_stat->to = to;
    new_stat->event = cmd;
    new_stat->bkp = bkp;
    new_stat->content_length = oldsize;
    undo_size++;
    undo_stack = new_stat;
    //printf("Old status from row %d to row %d\n", from, to);
    /*for (int i = 0; i < (to - from); i++) {
        printf("%d  %s\n", from+i, bkp[i]);
    }*/
    /*Da togliere*/
    //free(bkp);
}

/*In caso di restore da un'operazione di delete è necessario
 * shiftare le righe non cancellate nelle posizioni precedenti
 * per far spazio a quelle eliminate che devono essere reinserite*/
void shifta(int from, int new_size, int old_size) {
    old_size--;
    new_size--;
    for (int i = 0; old_size - i >= from; i++) {
        /*strcpy(testo[new_size - i], testo[old_size - i]);*/
        testo[new_size - i] = testo[old_size - i];
    }
}

/*Reverta il testo allo stato specificato dallo stato
 * fornito come argomento*/
void restore_backup(stack *stato) {
    int old_size = current_size;
    //printf("to: %d\tsize: %d\n", stato->to, current_size);
    if (stato->to > current_size) {
        testo = realloc(testo, stato->to * sizeof(char *));
        current_size = stato->to;
        //printf("old size: %d\tNew size: %d\n", old_size, current_size);
    } else if (stato->event == 'd') {
        current_size += stato->to - stato->from;
        testo = realloc(testo, current_size * sizeof(char *));
        shifta(stato->from, current_size, old_size);
    }
    int j = 0;
    int vuote = 0;
    for (int i = stato->from; i < stato->content_length; i++) {
        /*Se non va fare strcpy*/
        if (stato->bkp[j] == NULL) {
            vuote++;
            testo[i] = NULL;
        } else {
            //printf("%s\t<-%s\n", testo[i], stato->bkp[j]);
            testo[i] = stato->bkp[j];
        }
        j++;
    }
    current_size -= vuote;
    free_stack(stato);
    undo_size--;
    testo = realloc(testo, current_size * sizeof(char *));
}

/*Quando viene chiamato undo salva lo stato delle righe intaccate
 * dall'operazione all'interno dell'apposita struttura dati per il redo*/
void save_backup_redo(int from, int to, char cmd, int oldsize) {
    stack *new_stat = malloc(sizeof(stack));
    new_stat->prew = redo_stack;
    new_stat->from = from;
    new_stat->to = to;
    new_stat->event = cmd;
    new_stat->bkp = dump_backup(from, to);
    new_stat->content_length = oldsize;
    redo_size++;
    redo_stack = new_stat;
}

/*Esegue l'undo di un numero operazioni pari al valore steps
 * per ogni step esegue le seguenti operazioni:
 * - salva lo stato attuale nella stack redo (solo le righe interessate)
 * - modifica le righe interessate come descritto nell'elemento della stack undo
 * - per più undo consecutivi passa allo stato precedente*/
void undo(int steps) {
    if(!changed){
        return;
    }
    stack *s = undo_stack;
    stack *s2;
    for (int i = 0; /*s != NULL*/ undo_size > 0 && i < steps; i++) {
        save_backup_redo(s->from, s->to, s->event, current_size);
        s2 = s->prew;
        restore_backup(s);
        s = s2;
        /*if (current_size == 0) {
            break;
        }*/

    }
    undo_stack = s;
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
void delete(int from, int to, bool manual) {
    //printf("%d > %d\n", from, current_size);
    if(!changed){
        return;
    }
    if (manual) {
        empty_redo_stack();
    }

    int oldsize = current_size;

    char **bkp;
    if (from > current_size && manual) {
        bkp = dump_backup(from - 1, to);
        save_backup_undo(bkp, 'd', from - 1, to, current_size);
        return;
    }
    from--;
    int j = 0;
    int oldto = to;
    if (to > current_size) {
        to = current_size;
    }
    if (manual && current_size > 0)
        bkp = dump_backup(from, oldto);

    for (int i = from; i < to; i++, j++) {
        //printf("riga: %d\tbkp: %d\n", i, j);
        /*bkp[j] = malloc(1024 * sizeof(char));
        strcpy(bkp[j], testo[from]);*/
        compress(from);
        testo[current_size - 1] = NULL; //FREE?
        current_size--;

    }
    if (manual)
        save_backup_undo(bkp, 'd', from, to, oldsize);
    testo = realloc(testo, current_size * sizeof(char *));
}


/*Sostituisce il testo esistente in una riga con quello richiesto all'utente
 * se la riga non esiste ne crea una nuova
 * se la riga esiste chiama save_backup per salvare il vecchio stato
 * salva anche se la riga vecchia era vuota e la etichetta NULL
 * se manual prende l'inserimento dall'utente
 * se non manual inserisce da puntatore*/
void change(int from, int to, bool manual, char **autoins) {
    if (manual) {
        empty_redo_stack();
        changed = true;
    }
    int oldsize = current_size;
    char **bkp;
    from--;
    if (to > current_size) {
        oldsize = current_size;
        /*RESTORE*/
        if (manual)
            bkp = dump_backup(from, to); /*WARN current_size*/
        int k = current_size;
        testo = realloc(testo, to * sizeof(char *));
        current_size = to;
        /*Serve per far si che il nuovo puntatore riga venga identificato
         * come null (non occupato) dal resto del programma dopo la realloc*/
        for (; k < current_size; k++) {
            testo[k] = NULL;
        }

    } else {
        /*RESTORE*/
        oldsize = to;
        if (manual)
            bkp = dump_backup(from, to);

    }
    if (manual) {
        //char f[10];
        //fgets(f, 10, stdin);
    } else {
        to--;
    }
    int j = 0;
    for (int i = from; i <= to; i++, j++) {
//        char c[1024];
        if (manual) {
            fgets(c, 1024, stdin);
        } else {
            testo[i] = autoins[j];
            continue;
        }
        if (strcmp(c, ".\0") == 0 || i == to) {
            break;
        } else {
            int length = strlen(c) + 1;
            testo[i] = malloc(length * sizeof(char));
            strcpy(testo[i], c);
        }
    }
    /*RESTORE*/
    if (manual)
        save_backup_undo(bkp, 'c', from, to, oldsize);

}

void redo(int steps) {
    if(!changed){
        return;
    }
    stack *s = redo_stack;
    stack *s2;
    for (int i = 0; s != NULL && i < steps && redo_size > 0; i++) {

        char **bkp = dump_backup(s->from, s->to);
        /*Maybe se event è delete nella redo non è necessario salvare lo stato successivo
         * Anche perchè non esiste, per ora occupo memoria senza usarla per semplicità*/
        save_backup_undo(bkp, s->event, s->from, s->to, current_size);
        switch (s->event) {
            case 'd':
                delete(s->from + 1, s->to, false);
                break;
            case 'c':
                change(s->from + 1, s->to, false, s->bkp);
                break;
        }
        s2 = s->prew;
        free_stack(s);
        redo_size--;
        s = s2;
    }
    redo_stack = s;
}

/*Interpreta i comandi dell'utente
 * nei formati
 * - 5c
 * - 2,5d
 * e chiama le opportune funzioni*/
bool interpreta(char *input) {
    int length = (int) strlen(input);
    char cmd;
    cmd = input[length - 2];
    //printf("cmd: %c\n", cmd);
    input[length - 2] = '\0';
    token = strtok(input, ",");
    int from = atoi(token);
    //printf("from: %d\n", from);
    int to;
    token = strtok(NULL, ",");
    if (token != NULL) {
        to = atoi(token);
    }
    //printf("From: %d\tTo: %d\tCmd: %c\n", from, to, cmd);
    switch (cmd) {
        case 'c':
            change(from, to, true, NULL);
            break;
        case 'p':
            print(from, to);
            break;
        case 'd':
            delete(from, to, true);
            break;
        case 'u':
            undo(from);
            break;
        case 'r':
            redo(from);
            break;
        default:
            return false;
    }
    return true;
}

int main() {
    while (true) {
        fgets(input, 20, stdin);
        //printf("%s\n", input);
        if (input[0] == 'q') {
            break;
        } else if (input[0] == 's') {
            printf("Size: %d\n", undo_size);
            print_stack(undo_stack);
        } else if (input[0] == 'w') {
            printf("Size: %d\n", redo_size);
            print_stack(redo_stack);
        } else if (input[0] == 't') {
            printf("Text size: %d\n", current_size);
        } else {
            interpreta(input);
        }
    }
    return 0;
}
