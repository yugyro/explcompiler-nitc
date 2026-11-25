#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "tac.h"

int nerrors = 0, lineno = 1, in_local = 0;
Ast *program = NULL;

void semerr(const char *fmt, ...) {
    va_list ap;
    fprintf(stderr, "line %d: error: ", lineno);
    va_start(ap, fmt); vfprintf(stderr, fmt, ap); va_end(ap);
    fputc('\n', stderr);
    nerrors++;
}

/* ---------------- symbol table ---------------- */
static Sym syms[2000];
static int nsyms;

Sym *sym_lookup(const char *name) {
    int i;
    for (i = 0; i < nsyms; i++) if (!syms[i].global && !strcmp(syms[i].name, name)) return &syms[i];
    for (i = 0; i < nsyms; i++) if (syms[i].global && !strcmp(syms[i].name, name)) return &syms[i];
    return NULL;
}
int sym_exists_any(const char *name) {
    int i;
    for (i = 0; i < nsyms; i++) if (!strcmp(syms[i].name, name)) return 1;
    return 0;
}
Sym *sym_declare(const char *name, int type, int isArray, int size, int global) {
    int i;
    for (i = 0; i < nsyms; i++)
        if (syms[i].global == global && !strcmp(syms[i].name, name)) {
            semerr("'%s' is declared more than once", name);
            return NULL;
        }
    if (isArray && size <= 0) { semerr("array '%s' must have positive size", name); return NULL; }
    Sym *s = &syms[nsyms++];
    strncpy(s->name, name, 63); s->name[63] = 0;
    s->type = type; s->isArray = isArray; s->size = size; s->global = global;
    return s;
}

/* ---------------- AST builders with type checking ---------------- */
static Ast *newast(int kind, int type) {
    Ast *a = calloc(1, sizeof *a);
    a->kind = kind; a->type = type;
    return a;
}
Ast *mk_int(long v) { Ast *a = newast(E_INT, T_INT); a->ival = v; return a; }
Ast *mk_str(char *s) { Ast *a = newast(E_STR, T_STR); a->sval = s; return a; }

Ast *mk_var(char *name) {
    Sym *s = sym_lookup(name);
    Ast *a = newast(E_VAR, T_ERR);
    a->sval = name;
    if (!s) semerr("undeclared variable '%s'", name);
    else if (s->isArray) semerr("array '%s' used without an index", name);
    else a->type = s->type;
    return a;
}
Ast *mk_arr(char *name, Ast *idx) {
    Sym *s = sym_lookup(name);
    Ast *a = newast(E_ARR, T_ERR);
    a->sval = name; a->a = idx;
    if (!s) semerr("undeclared array '%s'", name);
    else if (!s->isArray) semerr("'%s' is not an array", name);
    else a->type = s->type;
    if (idx->type != T_INT && idx->type != T_ERR) semerr("index of '%s' must be an integer", name);
    return a;
}
Ast *mk_bin(int op, Ast *l, Ast *r) {
    Ast *a = newast(E_BIN, T_ERR);
    a->op = op; a->a = l; a->b = r;
    if (l->type == T_ERR || r->type == T_ERR) return a;
    if (l->type != T_INT || r->type != T_INT) semerr("operands of '%c' must be integers", op);
    else a->type = T_INT;
    return a;
}
Ast *mk_neg(Ast *e) {
    Ast *a = newast(E_NEG, T_ERR);
    a->a = e;
    if (e->type == T_ERR) return a;
    if (e->type != T_INT) semerr("operand of unary '-' must be an integer"); else a->type = T_INT;
    return a;
}
Ast *mk_rel(int rel, Ast *l, Ast *r) {
    Ast *a = newast(E_REL, T_ERR);
    a->op = rel; a->a = l; a->b = r;
    if (l->type == T_ERR || r->type == T_ERR) return a;
    if (l->type != r->type || (l->type != T_INT && l->type != T_STR))
        semerr("relational operator needs two integers or two strings");
    else a->type = T_BOOL;
    return a;
}
Ast *mk_logic(int kind, Ast *l, Ast *r) {
    Ast *a = newast(kind, T_ERR);
    a->a = l; a->b = r;
    if (l->type == T_ERR || r->type == T_ERR) return a;
    if (l->type != T_BOOL || r->type != T_BOOL) semerr("'and'/'or' need logical operands");
    else a->type = T_BOOL;
    return a;
}
Ast *mk_not(Ast *e) {
    Ast *a = newast(E_NOT, T_ERR);
    a->a = e;
    if (e->type == T_ERR) return a;
    if (e->type != T_BOOL) semerr("'not' needs a logical operand"); else a->type = T_BOOL;
    return a;
}
Ast *mk_assign(Ast *lv, Ast *rhs) {
    Ast *a = newast(S_ASSIGN, T_ERR);
    a->a = lv; a->b = rhs;
    if (lv->type != T_ERR && rhs->type != T_ERR && lv->type != rhs->type)
        semerr("type mismatch in assignment");
    return a;
}
Ast *mk_if(Ast *cond, Ast *t, Ast *e) {
    Ast *a = newast(S_IF, T_ERR);
    a->a = cond; a->b = t; a->c = e;
    if (cond->type != T_ERR && cond->type != T_BOOL) semerr("'if' condition must be a logical expression");
    return a;
}
Ast *mk_while(Ast *cond, Ast *body) {
    Ast *a = newast(S_WHILE, T_ERR);
    a->a = cond; a->b = body;
    if (cond->type != T_ERR && cond->type != T_BOOL) semerr("'while' condition must be a logical expression");
    return a;
}
Ast *mk_read(Ast *lv) { Ast *a = newast(S_READ, T_ERR); a->a = lv; return a; }
Ast *mk_write(Ast *e) {
    Ast *a = newast(S_WRITE, T_ERR);
    a->a = e;
    if (e->type != T_ERR && e->type != T_INT && e->type != T_STR) semerr("write() needs an integer or string");
    return a;
}
Ast *mk_simple(int kind) { return newast(kind, T_ERR); }
Ast *mk_ret(Ast *e) {
    Ast *a = newast(S_RET, T_ERR);
    a->a = e;
    if (e->type != T_ERR && e->type != T_INT) semerr("main must return an integer");
    return a;
}
Ast *ast_append(Ast *list, Ast *s) {
    Ast *p;
    if (!list) return s;
    for (p = list; p->next; p = p->next) ;
    p->next = s;
    return list;
}
