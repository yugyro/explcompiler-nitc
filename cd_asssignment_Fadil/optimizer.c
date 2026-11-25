
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "tac.h"

#define MAXN 2000

typedef struct {
    int  isconst;      /* 1 for constant leaf               */
    long val;          /* value of constant leaf            */
    char op[10];       /* operator ("" for leaves)          */
    int  l, r;         /* children (indices into nodes[])   */
    char holder[32];   /* variable that holds the value     */
} Node;

static Node nodes[MAXN];
static int  nn;

static char names[MAXN][32];
static int  nameNode[MAXN];
static int  nnames;

static Quad *out;
static int   nout;
static int   nfold, ncse;

static int is_num(const char *s)
{
    if (*s == '-') s++;
    if (!*s) return 0;
    for (; *s; s++)
        if (!isdigit((unsigned char)*s)) return 0;
    return 1;
}

static int is_temp(const char *s)
{
    int i;
    if (s[0] != 't' || !s[1]) return 0;
    for (i = 1; s[i]; i++)
        if (!isdigit((unsigned char)s[i])) return 0;
    return 1;
}

static void reset_dag(void)
{
    nn = 0;
    nnames = 0;
}

static int const_node(long v)
{
    int i;
    for (i = 0; i < nn; i++)
        if (nodes[i].isconst && nodes[i].val == v) return i;
    nodes[nn].isconst = 1;
    nodes[nn].val = v;
    nodes[nn].op[0] = 0;
    nodes[nn].holder[0] = 0;
    nodes[nn].l = nodes[nn].r = -1;
    return nn++;
}

static int find_name(const char *s)
{
    int i;
    for (i = 0; i < nnames; i++)
        if (!strcmp(names[i], s)) return i;
    return -1;
}

/* node currently representing the value of operand s */
static int get_node(const char *s)
{
    int k;
    if (is_num(s)) return const_node(atol(s));
    k = find_name(s);
    if (k >= 0) return nameNode[k];
    /* new leaf for a variable used before being assigned in this block */
    nodes[nn].isconst = 0;
    nodes[nn].op[0] = 0;
    nodes[nn].l = nodes[nn].r = -1;
    strcpy(nodes[nn].holder, s);
    strcpy(names[nnames], s);
    nameNode[nnames++] = nn;
    return nn++;
}

/* attach name to node n (name gets a new value) */
static void set_name(const char *name, int n)
{
    int k = find_name(name), i, j;
    if (k >= 0) nameNode[k] = n;
    else {
        strcpy(names[nnames], name);
        nameNode[nnames++] = n;
    }
    /* nodes that were held by 'name' lose their holder, unless
       another name still holds the same value */
    for (i = 0; i < nn; i++) {
        if (i != n && !strcmp(nodes[i].holder, name)) {
            nodes[i].holder[0] = 0;
            for (j = 0; j < nnames; j++)
                if (nameNode[j] == i && strcmp(names[j], name)) {
                    strcpy(nodes[i].holder, names[j]);
                    break;
                }
        }
    }
}

/* text to use for a node when generating an operand */
static void operand(int n, char *buf)
{
    if (nodes[n].isconst) sprintf(buf, "%ld", nodes[n].val);
    else strcpy(buf, nodes[n].holder);
}

static int find_interior(const char *op, int l, int r)
{
    int i;
    for (i = 0; i < nn; i++)
        if (!nodes[i].isconst && nodes[i].holder[0] &&
            !strcmp(nodes[i].op, op) && nodes[i].l == l && nodes[i].r == r)
            return i;
    return -1;
}

static void put(const char *op, const char *a1, const char *a2, const char *res)
{
    strcpy(out[nout].op, op);
    strcpy(out[nout].a1, a1);
    strcpy(out[nout].a2, a2);
    strcpy(out[nout].res, res);
    nout++;
}

/* evaluate constant expression; returns 0 if it cannot be folded */
static int eval(const char *op, long a, long b, long *v)
{
    if      (!strcmp(op, "+"))  *v = a + b;
    else if (!strcmp(op, "-"))  *v = a - b;
    else if (!strcmp(op, "*"))  *v = a * b;
    else if (!strcmp(op, "/"))  { if (b == 0) return 0; *v = a / b; }
    else if (!strcmp(op, "%"))  { if (b == 0) return 0; *v = a % b; }
    else if (!strcmp(op, "<"))  *v = a < b;
    else if (!strcmp(op, ">"))  *v = a > b;
    else if (!strcmp(op, "<=")) *v = a <= b;
    else if (!strcmp(op, ">=")) *v = a >= b;
    else if (!strcmp(op, "==")) *v = a == b;
    else if (!strcmp(op, "!=")) *v = a != b;
    else if (!strcmp(op, "&&")) *v = a && b;
    else if (!strcmp(op, "||")) *v = a || b;
    else return 0;
    return 1;
}

static int commutative(const char *op)
{
    return !strcmp(op, "+") || !strcmp(op, "*") || !strcmp(op, "==") ||
           !strcmp(op, "!=") || !strcmp(op, "&&") || !strcmp(op, "||");
}

/* handle  res = a1 op a2   (a2 is "" for uminus) */
static void do_operation(Quad *q)
{
    int l = get_node(q->a1);
    int r = (q->a2[0]) ? get_node(q->a2) : -1;
    int n;
    long v;
    char s1[32], s2[32], s3[32];
    int unary = !strcmp(q->op, "uminus");

    /* ---- constant folding ---- */
    if (nodes[l].isconst && (unary || nodes[r].isconst)) {
        int ok = 1;
        if (unary) v = -nodes[l].val;
        else ok = eval(q->op, nodes[l].val, nodes[r].val, &v);
        if (ok) {
            nfold++;
            n = const_node(v);
            set_name(q->res, n);
            if (!is_temp(q->res)) {          /* temps just become constants */
                sprintf(s1, "%ld", v);
                put("=", s1, "", q->res);
            }
            return;
        }
    }

    /* put children in a fixed order for commutative operators */
    if (!unary && commutative(q->op) && l > r) { int t = l; l = r; r = t; }

    /* ---- common subexpression ---- */
    n = find_interior(q->op, l, r);
    if (n >= 0) {
        ncse++;
        if (strcmp(nodes[n].holder, q->res) != 0 && !is_temp(q->res)) {
            strcpy(s1, nodes[n].holder);
            set_name(q->res, n);
            put("=", s1, "", q->res);
        } else
            set_name(q->res, n);
        return;
    }

    /* ---- new node ---- */
    operand(l, s1);
    if (r >= 0) operand(r, s2); else s2[0] = 0;
    strcpy(s3, q->res);
    nodes[nn].isconst = 0;
    strcpy(nodes[nn].op, q->op);
    nodes[nn].l = l;
    nodes[nn].r = r;
    strcpy(nodes[nn].holder, q->res);
    n = nn++;
    set_name(q->res, n);
    put(q->op, s1, s2, s3);
}

void optimize(Quad *in, int n, Quad *outcode, int *nout_p, int *folds, int *cses)
{
    int i, newblock = 1;
    char s[32];

    out = outcode;
    nout = nfold = ncse = 0;

    for (i = 0; i < n; i++) {
        Quad *q = &in[i];

        /* a label starts a new basic block; so does the instruction
           after a jump */
        if (!strcmp(q->op, "label")) newblock = 1;
        if (newblock) { reset_dag(); newblock = 0; }

        if (!strcmp(q->op, "label")) {
            put("label", "", "", q->res);
        }
        else if (!strcmp(q->op, "goto")) {
            put("goto", "", "", q->res);
            newblock = 1;
        }
        else if (!strcmp(q->op, "ifFalse")) {
            operand(get_node(q->a1), s);
            put("ifFalse", s, "", q->res);
            newblock = 1;
        }
        else if (!strcmp(q->op, "read")) {
            /* variable gets a fresh unknown value */
            nodes[nn].isconst = 0;
            nodes[nn].op[0] = 0;
            nodes[nn].l = nodes[nn].r = -1;
            strcpy(nodes[nn].holder, q->res);
            set_name(q->res, nn);
            nn++;
            put("read", "", "", q->res);
        }
        else if (!strcmp(q->op, "write")) {
            operand(get_node(q->a1), s);
            put("write", s, "", "");
        }
        else if (!strcmp(q->op, "=")) {
            int k = get_node(q->a1);
            operand(k, s);
            if (strcmp(s, q->res) != 0 || nodes[k].isconst) {
                set_name(q->res, k);
                put("=", s, "", q->res);
            } else
                set_name(q->res, k);
        }
        else {
            do_operation(q);
        }
    }
    *nout_p = nout;
    *folds = nfold;
    *cses = ncse;
}
