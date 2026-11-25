#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#define TYPE_INT_VAR 1
#define TYPE_STR_VAR 2

typedef struct Gsymbol {
    char *name;
    int type;
    int size;      // total words: rows*cols for 2D, N for 1D, 1 for scalar
    int cols;      // number of columns; 1 if not a 2D array
    int binding;
    struct Gsymbol *next;
} Gsymbol;

struct Gsymbol *Lookup(char *name);
void Install(char *name, int type, int size, int cols);
void printSymtab();
int totalDeclaredWords();

#endif