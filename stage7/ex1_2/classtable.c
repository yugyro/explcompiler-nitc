#include "classtable.h"
#include "symbol.h"

struct Classtable *CLookup(struct Classtable *head, const char *name) {
    for (struct Classtable *c = head; c; c = c->next)
        if (strcmp(c->name, name) == 0) return c;
    return NULL;
}

struct Classtable *CInstall(struct Classtable *head, char *name, char *parentName) {
    if (CLookup(head, name)) {
        fprintf(stderr, "Error: Class '%s' redeclared\n", name);
        exit(1);
    }
    if (parentName && *parentName) {
        fprintf(stderr, "Error: Inheritance is not supported in Stage 7 (class '%s')\n", name);
        exit(1);
    }
    struct Classtable *c = calloc(1, sizeof(*c));
    if (!c) { perror("calloc"); exit(1); }
    c->name = strdup(name);
    c->parent = NULL;
    c->classIndex = 0;
    if (!head) return c;
    for (struct Classtable *it = head; it; it = it->next) c->classIndex++;
    struct Classtable *tail = head;
    while (tail->next) tail = tail->next;
    tail->next = c;
    return head;
}

struct ClassField *Class_Flookup(struct Classtable *c, const char *name) {
    if (!c) return NULL;
    for (struct ClassField *f = c->fields; f; f = f->next)
        if (strcmp(f->name, name) == 0) return f;
    return NULL;
}

struct MemberFunc *Class_Mlookup(struct Classtable *c, const char *name) {
    if (!c) return NULL;
    for (struct MemberFunc *m = c->methods; m; m = m->next)
        if (strcmp(m->name, name) == 0) return m;
    return NULL;
}

struct ClassField *Class_Finstall(struct Classtable *c, char *typeName, char *fieldName,
                                  struct TypeTable *type, struct Classtable *ctype) {
    (void)typeName;
    if (!c) { fprintf(stderr, "Internal error: no current class for field '%s'\n", fieldName); exit(1); }
    if (Class_Flookup(c, fieldName) || Class_Mlookup(c, fieldName)) {
        fprintf(stderr, "Error: Member '%s' redeclared in class '%s'\n", fieldName, c->name);
        exit(1);
    }
    if (c->fieldCount >= 8) {
        fprintf(stderr, "Error: Class '%s' exceeds the limit of 8 fields\n", c->name);
        exit(1);
    }
    struct ClassField *f = calloc(1, sizeof(*f));
    if (!f) { perror("calloc"); exit(1); }
    f->name = strdup(fieldName);
    f->fieldIndex = c->fieldCount++;
    f->type = type;
    f->ctype = ctype;
    if (!c->fields) c->fields = f;
    else { struct ClassField *t = c->fields; while (t->next) t = t->next; t->next = f; }
    return f;
}

struct MemberFunc *Class_Minstall(struct Classtable *c, char *name, struct TypeTable *type,
                                  struct ParamStruct *params, int flabel) {
    if (!c) { fprintf(stderr, "Internal error: no current class for method '%s'\n", name); exit(1); }
    if (Class_Mlookup(c, name) || Class_Flookup(c, name)) {
        fprintf(stderr, "Error: Member '%s' redeclared in class '%s' (overloading is not supported)\n", name, c->name);
        exit(1);
    }
    if (c->methodCount >= 8) {
        fprintf(stderr, "Error: Class '%s' exceeds the limit of 8 methods\n", c->name);
        exit(1);
    }
    struct MemberFunc *m = calloc(1, sizeof(*m));
    if (!m) { perror("calloc"); exit(1); }
    m->name = strdup(name);
    m->type = type;
    m->paramList = params;
    m->funcPosition = c->methodCount++;
    m->flabel = flabel;
    if (!c->methods) c->methods = m;
    else { struct MemberFunc *t = c->methods; while (t->next) t = t->next; t->next = m; }
    return m;
}

int Class_CheckArgs(struct MemberFunc *method, struct ParamStruct *actual) {
    if (!method) return 0;
    struct ParamStruct *e = method->paramList, *a = actual;
    while (e && a) {
        if (e->Ctype ? (e->Ctype != a->Ctype) : (e->type != a->type || e->isPtr != a->isPtr)) {
            fprintf(stderr, "Error: Argument type mismatch in method '%s'\n", method->name);
            exit(1);
        }
        e = e->next; a = a->next;
    }
    if (e || a) {
        fprintf(stderr, "Error: Argument count mismatch in method '%s'\n", method->name);
        exit(1);
    }
    return 1;
}

void Class_PrintTable(struct Classtable *head) {
    printf("\n========== CLASS TABLE ==========\n");
    for (struct Classtable *c = head; c; c = c->next) {
        printf("Class: %s | Parent: %s | Fields: %d | Methods: %d\n", c->name,
               c->parent ? c->parent->name : "NULL", c->fieldCount, c->methodCount);
        for (struct ClassField *f = c->fields; f; f = f->next)
            printf("  Field[%d]: %s : %s%s\n", f->fieldIndex, f->name,
                   f->type ? f->type->name : (f->ctype ? f->ctype->name : "<unknown>"),
                   f->ctype ? " (class)" : "");
        for (struct MemberFunc *m = c->methods; m; m = m->next)
            printf("  Method[%d]: %s -> %s (label F%d)\n", m->funcPosition, m->name,
                   m->type ? m->type->name : "<unknown>", m->flabel);
    }
}
