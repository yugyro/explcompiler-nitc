#include "classtable.h"
#include "symbol.h"

struct Classtable *CLookup(struct Classtable *head, const char *name) {
    for (struct Classtable *c = head; c; c = c->next)
        if (strcmp(c->name, name) == 0) return c;
    return NULL;
}

static struct ParamStruct *copyParams(struct ParamStruct *src) {
    struct ParamStruct *head = NULL, *tail = NULL;
    for (; src; src = src->next) {
        struct ParamStruct *p = calloc(1, sizeof(*p));
        if (!p) { perror("calloc"); exit(1); }
        p->name = strdup(src->name);
        p->type = src->type;
        p->Ctype = src->Ctype;
        p->isPtr = src->isPtr;
        if (!head) head = p; else tail->next = p;
        tail = p;
    }
    return head;
}

int Class_IsSubtype(struct Classtable *child, struct Classtable *parent) {
    for (struct Classtable *c = child; c; c = c->parent)
        if (c == parent) return 1;
    return 0;
}

struct Classtable *CInstall(struct Classtable *head, char *name, char *parentName) {
    if (CLookup(head, name)) {
        fprintf(stderr, "Error: Class '%s' redeclared\\n", name);
        exit(1);
    }
    struct Classtable *parent = NULL;
    if (parentName && *parentName) {
        parent = CLookup(head, parentName);
        if (!parent) {
            fprintf(stderr, "Error: Parent class '%s' must be defined before class '%s'\\n",
                    parentName, name);
            exit(1);
        }
    }
    struct Classtable *cl = calloc(1, sizeof(*cl));
    if (!cl) { perror("calloc"); exit(1); }
    cl->name = strdup(name);
    cl->parent = parent;
    cl->classIndex = 0;
    if (head) {
        for (struct Classtable *it = head; it; it = it->next) cl->classIndex++;
        struct Classtable *tail = head;
        while (tail->next) tail = tail->next;
        tail->next = cl;
    } else {
        head = cl;
    }

    /* Stage 8 Task 1: copy inherited fields and methods into the child table.
       Field/method indices and inherited method labels remain stable. */
    if (parent) {
        for (struct ClassField *pf = parent->fields; pf; pf = pf->next) {
            struct ClassField *f = calloc(1, sizeof(*f));
            if (!f) { perror("calloc"); exit(1); }
            f->name = strdup(pf->name);
            f->fieldIndex = pf->fieldIndex;
            f->type = pf->type;
            f->ctype = pf->ctype;
            if (!cl->fields) cl->fields = f;
            else { struct ClassField *t = cl->fields; while (t->next) t = t->next; t->next = f; }
            cl->fieldCount++;
        }
        for (struct MemberFunc *pm = parent->methods; pm; pm = pm->next) {
            struct MemberFunc *m = calloc(1, sizeof(*m));
            if (!m) { perror("calloc"); exit(1); }
            m->name = strdup(pm->name);
            m->type = pm->type;
            m->paramList = copyParams(pm->paramList);
            m->funcPosition = pm->funcPosition;
            m->flabel = pm->flabel;
            m->isDefined = 1; /* inherited method needs no child definition */
            m->inherited = 1;
            if (!cl->methods) cl->methods = m;
            else { struct MemberFunc *t = cl->methods; while (t->next) t = t->next; t->next = m; }
            cl->methodCount++;
        }
    }
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

static int sameParams(struct ParamStruct *a, struct ParamStruct *b) {
    while (a && b) {
        if (a->type != b->type || a->Ctype != b->Ctype || a->isPtr != b->isPtr)
            return 0;
        a = a->next; b = b->next;
    }
    return a == NULL && b == NULL;
}

struct MemberFunc *Class_MlookupSignature(struct Classtable *c, const char *name,
                                          struct TypeTable *ret, struct ParamStruct *params) {
    if (!c) return NULL;
    for (struct MemberFunc *m = c->methods; m; m = m->next)
        if (strcmp(m->name, name) == 0 && m->type == ret &&
            sameParams(m->paramList, params))
            return m;
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
    if (!c) { fprintf(stderr, "Internal error: no current class for method '%s'\\n", name); exit(1); }
    if (Class_Flookup(c, name)) {
        fprintf(stderr, "Error: Method '%s' conflicts with a field in class '%s'\\n", name, c->name);
        exit(1);
    }

    for (struct MemberFunc *existing = c->methods; existing; existing = existing->next) {
        if (strcmp(existing->name, name) != 0 ||
            !sameParams(existing->paramList, params))
            continue;
        if (existing->inherited) {
            if (existing->type != type) {
                fprintf(stderr, "Error: Override of '%s' in class '%s' must preserve return type\\n",
                        name, c->name);
                exit(1);
            }
            existing->type = type;
            existing->paramList = copyParams(params);
            existing->flabel = flabel;
            existing->isDefined = 0;
            existing->inherited = 0;
            return existing;
        }
        fprintf(stderr, "Error: Method '%s' with the same signature redeclared in class '%s'\\n",
                name, c->name);
        exit(1);
    }

    if (c->methodCount >= 8) {
        fprintf(stderr, "Error: Class '%s' exceeds the limit of 8 methods (including overloads)\\n", c->name);
        exit(1);
    }
    struct MemberFunc *m = calloc(1, sizeof(*m));
    if (!m) { perror("calloc"); exit(1); }
    m->name = strdup(name);
    m->type = type;
    m->paramList = copyParams(params);
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
        for (struct MemberFunc *m = c->methods; m; m = m->next) {
            printf("  Method[%d]: %s(", m->funcPosition, m->name);
            for (struct ParamStruct *p = m->paramList; p; p = p->next) {
                if (p != m->paramList) printf(", ");
                if (p->Ctype) printf("%s", p->Ctype->name);
                else printf("%s", p->type ? p->type->name : "<unknown>");
                if (p->isPtr) printf("*");
            }
            printf(") -> %s (label F%d)%s \n",
                   m->type ? m->type->name : "<unknown>", m->flabel,
                   m->inherited ? " [inherited]" : "");
        }
    }
}
