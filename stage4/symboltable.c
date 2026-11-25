#include <stdio.h>
#include <stdlib.h>
#include "symboltable.h"
#include <string.h>

int addrcounter = 4096;
Gsymbol *head = NULL;

struct Gsymbol *Lookup(char *name){
    struct Gsymbol *curr = head;
    while(curr){
        if(strcmp(curr->name, name) == 0)
            return curr;
        curr = curr->next;
    }
    return NULL;
}

void Install(char *name, int type, int size, int cols){
    Gsymbol *check = Lookup(name);
    if(check){
        fprintf(stderr, "Semantic Error, variable %s declared more than once", name);
        exit(1);
    }
    Gsymbol *newvar = (Gsymbol *)malloc(sizeof(Gsymbol));
    newvar->binding = addrcounter;
    addrcounter += size;
    newvar->name = name;
    newvar->size = size;
    newvar->cols = cols;
    newvar->type = type;
    newvar->next = head;
    head = newvar;
}

void printSymtab(){
    printf("---- Symbol Table ----\n");
    for(struct Gsymbol *s = head; s != NULL; s = s->next)
        printf("%-10s type=%d size=%d cols=%d addr=%d\n", s->name, s->type, s->size, s->cols, s->binding);
    printf("-----------------------\n");
}

int totalDeclaredWords(){
    return addrcounter - 4096;
}