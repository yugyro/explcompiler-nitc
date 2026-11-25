#ifndef CLASSTABLE_H
#define CLASSTABLE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "typetable.h"

struct ParamStruct;
struct Classtable;

struct ClassField {
    char *name;
    int fieldIndex;
    struct TypeTable *type;
    struct Classtable *ctype;
    struct ClassField *next;
};

struct MemberFunc {
    char *name;
    struct TypeTable *type;
    struct ParamStruct *paramList;
    int funcPosition;
    int flabel;
    int isDefined;
    struct MemberFunc *next;
};

struct Classtable {
    char *name;
    struct ClassField *fields;
    struct MemberFunc *methods;
    struct Classtable *parent;
    int classIndex;
    int fieldCount;
    int methodCount;
    struct Classtable *next;
};

struct Classtable *CInstall(struct Classtable *head, char *name, char *parentName);
struct Classtable *CLookup(struct Classtable *head, const char *name);
struct ClassField *Class_Finstall(struct Classtable *c, char *typeName, char *fieldName,
                                  struct TypeTable *type, struct Classtable *ctype);
struct MemberFunc *Class_Minstall(struct Classtable *c, char *name, struct TypeTable *type,
                                  struct ParamStruct *params, int flabel);
struct ClassField *Class_Flookup(struct Classtable *c, const char *name);
struct MemberFunc *Class_Mlookup(struct Classtable *c, const char *name);
int Class_CheckArgs(struct MemberFunc *method, struct ParamStruct *actual);
void Class_PrintTable(struct Classtable *head);

#endif
