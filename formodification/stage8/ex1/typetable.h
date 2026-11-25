#ifndef TYPETABLE_H  
#define TYPETABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct TypeTable{
    char *name;
    int size;  
    struct Fieldlist *fields; 
    struct TypeTable *next;
};

struct Fieldlist{
    char *name;
    int fieldIndex;
    struct TypeTable *type;
    struct Fieldlist *next; 
};

extern struct TypeTable *T_INT, *T_STR, *T_BOOL, *T_VOID;
extern struct TypeTable *T_INT_PTR, *T_STR_PTR;
extern struct TypeTable *TUPLE_TYPE, * TUPLE_PTR_TYPE;

struct TypeTable * createTypeTableNode(char * name, int size, struct Fieldlist *fields );

struct Fieldlist * createFieldlistNode(char * name, struct TypeTable * type, int fieldIndex);

struct Fieldlist* FInstall(struct Fieldlist* fieldHead, char * name, struct TypeTable * type, int fieldIndex);

struct TypeTable* createTypeTable();

struct TypeTable* TLookup(struct TypeTable *head,char *name) ;

struct TypeTable* TInstall(struct TypeTable *head,char *name,int size, struct Fieldlist *fields);

struct Fieldlist* FLookup(struct TypeTable *type, char *name);

int GetSize(struct TypeTable * type);

void printTypeTable(struct TypeTable *head);

#endif