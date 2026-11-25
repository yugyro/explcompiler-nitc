/* TAC generation following ALSU Ch.6: expressions (Fig 6.19-style temporaries),
 * jumping code for booleans (Sec 6.6.1, label 0 = "fall through"),
 * and the standard translations of if / if-else / while.            */
#include <stdlib.h>
#include <string.h>
#include "tac.h"

Quad code[MAXQ];
int ncode = 0, elem_width = 1, tmpcnt = 0;
static int labcnt = 0;
static const char *relstr[] = { "<", ">", "<=", ">=", "==", "!=" };

Addr mkvar(const char *n) { Addr a; memset(&a, 0, sizeof a); a.k = A_VAR; strncpy(a.s, n, 255); return a; }
Addr mkint(long v)        { Addr a; memset(&a, 0, sizeof a); a.k = A_INT; a.v = v; return a; }
Addr mkstr(const char *s) { Addr a; memset(&a, 0, sizeof a); a.k = A_STR; strncpy(a.s, s, 255); return a; }
Addr newtemp(void) {
    Addr a; memset(&a, 0, sizeof a); a.k = A_TEMP;
    do { sprintf(a.s, "t%d", ++tmpcnt); } while (sym_exists_any(a.s));   /* never clash with a user name */
    return a;
}

static Quad *newq(int op) {
    if (ncode >= MAXQ) { fprintf(stderr, "program too large\n"); exit(1); }
    Quad *q = &code[ncode++];
    memset(q, 0, sizeof *q); q->op = op;
    return q;
}
static int newlabel(void) { return ++labcnt; }
static void e_label(int l)  { newq(Q_LABEL)->label = l; }
static void e_goto(int l)   { newq(Q_GOTO)->label = l; }
static void e_if(Addr x, int rel, Addr y, int l) {
    Quad *q = newq(Q_IFGOTO); q->a1 = x; q->a2 = y; q->rel = rel; q->label = l;
}
static int negate(int r) {
    switch (r) { case R_LT: return R_GE; case R_GE: return R_LT; case R_GT: return R_LE;
                 case R_LE: return R_GT; case R_EQ: return R_NE; default: return R_EQ; }
}

/* ---------------- expressions ---------------- */
static Addr gen_expr(Ast *e);

static Addr gen_index(Ast *idx) {              /* element offset; scaled when -w N > 1 */
    Addr i = gen_expr(idx);
    if (elem_width != 1) {
        Addr t = newtemp();
        Quad *q = newq(Q_BIN); q->res = t; q->a1 = i; q->a2 = mkint(elem_width); q->bop = '*';
        return t;
    }
    return i;
}

static Addr gen_expr(Ast *e) {
    Quad *q; Addr a, b, t;
    switch (e->kind) {
    case E_INT: return mkint(e->ival);
    case E_STR: return mkstr(e->sval);
    case E_VAR: return mkvar(e->sval);
    case E_ARR:
        a = gen_index(e->a); t = newtemp();
        q = newq(Q_LOAD); q->res = t; q->a1 = mkvar(e->sval); q->a2 = a;
        return t;
    case E_BIN:
        a = gen_expr(e->a); b = gen_expr(e->b); t = newtemp();
        q = newq(Q_BIN); q->res = t; q->a1 = a; q->a2 = b; q->bop = e->op;
        return t;
    case E_NEG:
        a = gen_expr(e->a); t = newtemp();
        q = newq(Q_UN); q->res = t; q->a1 = a;
        return t;
    }
    fprintf(stderr, "internal error: gen_expr\n"); exit(1);
}

/* ---------------- booleans: jumping code ---------------- */
static void gen_jump(Ast *b, int t, int f) {
    Addr x, y;
    switch (b->kind) {
    case E_REL:
        x = gen_expr(b->a); y = gen_expr(b->b);
        if (t && f)  { e_if(x, b->op, y, t); e_goto(f); }
        else if (t)  e_if(x, b->op, y, t);
        else if (f)  e_if(x, negate(b->op), y, f);
        break;
    case E_AND: {
        int f1 = f ? f : newlabel();
        gen_jump(b->a, 0, f1);
        gen_jump(b->b, t, f);
        if (!f) e_label(f1);
        break; }
    case E_OR: {
        int t1 = t ? t : newlabel();
        gen_jump(b->a, t1, 0);
        gen_jump(b->b, t, f);
        if (!t) e_label(t1);
        break; }
    case E_NOT:
        gen_jump(b->a, f, t);
        break;
    default:
        fprintf(stderr, "internal error: gen_jump\n"); exit(1);
    }
}

/* ---------------- statements ---------------- */
static void gen_list(Ast *s, int lb, int la);

static void gen_stmt(Ast *s, int lb, int la) {   /* lb/la: loop begin / after labels (0 = not in loop) */
    Quad *q; Addr v, i, t;
    switch (s->kind) {
    case S_ASSIGN:
        v = gen_expr(s->b);
        if (s->a->kind == E_VAR) {
            q = newq(Q_COPY); q->res = mkvar(s->a->sval); q->a1 = v;
        } else {
            i = gen_index(s->a->a);
            q = newq(Q_STORE); q->a1 = mkvar(s->a->sval); q->a2 = i; q->a3 = v;
        }
        break;
    case S_IF: {
        int after = newlabel();
        if (!s->c) {
            gen_jump(s->a, 0, after);
            gen_list(s->b, lb, la);
            e_label(after);
        } else {
            int els = newlabel();
            gen_jump(s->a, 0, els);
            gen_list(s->b, lb, la);
            e_goto(after);
            e_label(els);
            gen_list(s->c, lb, la);
            e_label(after);
        }
        break; }
    case S_WHILE: {
        int begin = newlabel(), after = newlabel();
        e_label(begin);
        gen_jump(s->a, 0, after);
        gen_list(s->b, begin, after);
        e_goto(begin);
        e_label(after);
        break; }
    case S_READ:
        if (s->a->kind == E_VAR) { q = newq(Q_READ); q->res = mkvar(s->a->sval); }
        else {
            i = gen_index(s->a->a); t = newtemp();
            q = newq(Q_READ); q->res = t;
            q = newq(Q_STORE); q->a1 = mkvar(s->a->sval); q->a2 = i; q->a3 = t;
        }
        break;
    case S_WRITE:
        v = gen_expr(s->a);
        q = newq(Q_WRITE); q->a1 = v;
        break;
    case S_BREAK: if (la) e_goto(la); break;     /* no effect outside a loop */
    case S_CONT:  if (lb) e_goto(lb); break;
    case S_BKPT:  newq(Q_BKPT); break;
    case S_RET:
        v = gen_expr(s->a);
        q = newq(Q_RET); q->a1 = v;
        break;
    }
}
static void gen_list(Ast *s, int lb, int la) { for (; s; s = s->next) gen_stmt(s, lb, la); }
void gen_program(Ast *body) { gen_list(body, 0, 0); }

/* ---------------- printing ---------------- */
static void addr_str(const Addr *a, char *buf) {
    switch (a->k) {
    case A_INT: sprintf(buf, "%ld", a->v); break;
    case A_STR: sprintf(buf, "\"%s\"", a->s); break;
    default:    strcpy(buf, a->s);
    }
}
void quad_str(const Quad *q, char *buf) {
    char r[300], x[300], y[300], z[300];
    addr_str(&q->res, r); addr_str(&q->a1, x); addr_str(&q->a2, y); addr_str(&q->a3, z);
    switch (q->op) {
    case Q_BIN:    sprintf(buf, "%s = %s %c %s", r, x, q->bop, y); break;
    case Q_UN:     sprintf(buf, "%s = - %s", r, x); break;
    case Q_COPY:   sprintf(buf, "%s = %s", r, x); break;
    case Q_LABEL:  sprintf(buf, "L%d:", q->label); break;
    case Q_GOTO:   sprintf(buf, "goto L%d", q->label); break;
    case Q_IFGOTO: sprintf(buf, "if %s %s %s goto L%d", x, relstr[q->rel], y, q->label); break;
    case Q_LOAD:   sprintf(buf, "%s = %s[%s]", r, x, y); break;
    case Q_STORE:  sprintf(buf, "%s[%s] = %s", x, y, z); break;
    case Q_READ:   sprintf(buf, "read %s", r); break;
    case Q_WRITE:  sprintf(buf, "write %s", x); break;
    case Q_BKPT:   strcpy(buf, "breakpoint"); break;
    case Q_RET:    sprintf(buf, "return %s", x); break;
    }
}
void print_code(FILE *f, const Quad *c, int n) {
    char buf[1000]; int i;
    for (i = 0; i < n; i++) {
        quad_str(&c[i], buf);
        fprintf(f, c[i].op == Q_LABEL ? "%s\n" : "    %s\n", buf);
    }
}
