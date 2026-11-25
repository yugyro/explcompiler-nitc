#ifndef TAC_H
#define TAC_H
#include <stdio.h>

#define MAXQ 20000

enum { T_ERR = 0, T_INT, T_STR, T_BOOL };
enum { R_LT, R_GT, R_LE, R_GE, R_EQ, R_NE };

/* ---------------- symbol table / diagnostics ---------------- */
typedef struct Sym {
    char name[64];
    int type, isArray, size, global;
} Sym;

extern int nerrors, lineno, in_local;
void semerr(const char *fmt, ...);
Sym *sym_lookup(const char *name);                 /* local scope first, then global */
Sym *sym_declare(const char *name, int type, int isArray, int size, int global);
int  sym_exists_any(const char *name);

/* ---------------- AST ---------------- */
enum { E_INT, E_STR, E_VAR, E_ARR, E_BIN, E_NEG, E_REL, E_AND, E_OR, E_NOT,
       S_ASSIGN, S_IF, S_WHILE, S_READ, S_WRITE, S_BREAK, S_CONT, S_BKPT, S_RET };

typedef struct Ast {
    int kind, type, op;
    long ival;
    char *sval;
    struct Ast *a, *b, *c, *next;
} Ast;

extern Ast *program;
Ast *mk_int(long v);          Ast *mk_str(char *s);
Ast *mk_var(char *name);      Ast *mk_arr(char *name, Ast *idx);
Ast *mk_bin(int op, Ast *l, Ast *r);  Ast *mk_neg(Ast *e);
Ast *mk_rel(int rel, Ast *l, Ast *r);
Ast *mk_logic(int kind, Ast *l, Ast *r);  Ast *mk_not(Ast *e);
Ast *mk_assign(Ast *lv, Ast *rhs);
Ast *mk_if(Ast *cond, Ast *thenl, Ast *elsel);
Ast *mk_while(Ast *cond, Ast *body);
Ast *mk_read(Ast *lv);        Ast *mk_write(Ast *e);
Ast *mk_simple(int kind);     Ast *mk_ret(Ast *e);
Ast *ast_append(Ast *list, Ast *s);

/* ---------------- three-address code ---------------- */
enum { A_NONE, A_VAR, A_TEMP, A_INT, A_STR };
typedef struct { int k; char s[256]; long v; } Addr;

enum { Q_BIN, Q_UN, Q_COPY, Q_LABEL, Q_GOTO, Q_IFGOTO,
       Q_LOAD,   /* res = a1[a2]   */
       Q_STORE,  /* a1[a2] = a3    */
       Q_READ, Q_WRITE, Q_BKPT, Q_RET };

typedef struct {
    int op;
    Addr res, a1, a2, a3;
    int bop;     /* + - * / %  (Q_BIN) */
    int rel;     /* relop (Q_IFGOTO)   */
    int label;   /* Q_LABEL, Q_GOTO, Q_IFGOTO */
} Quad;

extern Quad code[MAXQ];
extern int  ncode, elem_width, tmpcnt;

Addr mkvar(const char *n);
Addr mkint(long v);
Addr mkstr(const char *s);
Addr newtemp(void);
void gen_program(Ast *body);
void quad_str(const Quad *q, char *buf);
void print_code(FILE *f, const Quad *c, int n);

/* ---------------- optimizer ---------------- */
extern int opt_nfold, opt_ncse, opt_nblocks;
int optimize(const Quad *in, int n, Quad *out, FILE *log);   /* returns new length */
#endif
