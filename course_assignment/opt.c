/* Local optimisation of each basic block with a DAG (ALSU 8.5):
 *   - constant folding (done while DAG nodes are created)
 *   - common subexpression elimination (an existing node with the same
 *     operator and children is reused instead of creating a new one)
 * Array stores kill all earlier loads of that array (ALSU 8.5.5).
 * The block is then regenerated from the DAG (ALSU 8.5.4).            */
#include <stdlib.h>
#include <string.h>
#include "tac.h"

#define MAXNODES 40000
enum { NK_VAR, NK_CONST, NK_OP, NK_LOAD, NK_STORE, NK_READ, NK_WRITE, NK_BKPT };

typedef struct Node {
    int idx, kind, bop;
    struct Node *c[3];
    Addr leaf;                 /* NK_VAR: initial value of that name; NK_CONST: the constant */
    int killed, needed;
    int nl, cap; char **lab; int *labtemp;   /* identifiers attached to this node */
    char primary[256];         /* name that holds the node's value after regeneration */
} Node;

int opt_nfold = 0, opt_ncse = 0, opt_nblocks = 0;

static Node *nodes[MAXNODES]; static int nn;
static struct { char name[256]; Node *n; } vmap[MAXNODES]; static int nv;
static Quad *out; static int nout;
static FILE *logf; static int blockno;

static void emit(Quad q) { out[nout++] = q; }

/* ---------- DAG construction helpers ---------- */
static Node *newnode(int kind) {
    if (nn >= MAXNODES) { fprintf(stderr, "block too large\n"); exit(1); }
    Node *n = calloc(1, sizeof *n);
    n->kind = kind; n->idx = nn;
    nodes[nn++] = n;
    return n;
}
static Node *mapget(const char *name) {
    int i; for (i = 0; i < nv; i++) if (!strcmp(vmap[i].name, name)) return vmap[i].n;
    return NULL;
}
static void mapset(const char *name, Node *n) {
    int i;
    for (i = 0; i < nv; i++) if (!strcmp(vmap[i].name, name)) { vmap[i].n = n; return; }
    strcpy(vmap[nv].name, name); vmap[nv++].n = n;
}
static Node *var_node(Addr a) {
    Node *n = mapget(a.s);
    if (n) return n;
    n = newnode(NK_VAR); n->leaf = a; mapset(a.s, n);
    return n;
}
static Node *const_node(Addr a) {
    int i;
    for (i = 0; i < nn; i++) {
        Node *n = nodes[i];
        if (n->kind == NK_CONST && n->leaf.k == a.k &&
            (a.k == A_INT ? n->leaf.v == a.v : !strcmp(n->leaf.s, a.s))) return n;
    }
    Node *n = newnode(NK_CONST); n->leaf = a;
    return n;
}
static Node *val_node(Addr a) { return (a.k == A_INT || a.k == A_STR) ? const_node(a) : var_node(a); }

static void addlab(Node *n, const char *s, int temp) {
    if (n->nl == n->cap) {
        n->cap = n->cap ? n->cap * 2 : 4;
        n->lab = realloc(n->lab, n->cap * sizeof(char *));
        n->labtemp = realloc(n->labtemp, n->cap * sizeof(int));
    }
    n->lab[n->nl] = strdup(s); n->labtemp[n->nl++] = temp;
}
static void remlab(Node *n, const char *s) {
    int i;
    for (i = 0; i < n->nl; i++)
        if (!strcmp(n->lab[i], s)) {
            free(n->lab[i]);
            for (; i + 1 < n->nl; i++) { n->lab[i] = n->lab[i + 1]; n->labtemp[i] = n->labtemp[i + 1]; }
            n->nl--; return;
        }
}
/* attach identifier x to node n (removing it from its previous node) */
static void attach(Addr x, Node *n) {
    Node *old = mapget(x.s);
    if (old == n) return;
    if (old) remlab(old, x.s);
    mapset(x.s, n);
    if (n->kind == NK_VAR && !strcmp(n->leaf.s, x.s)) return;   /* back to its own initial value */
    addlab(n, x.s, x.k == A_TEMP);
}
static int has_user_label(Node *n) { int i; for (i = 0; i < n->nl; i++) if (!n->labtemp[i]) return 1; return 0; }

/* ---------- constant folding ---------- */
static int fold_bin(int op, long a, long b, long *r) {
    switch (op) {
    case '+': *r = a + b; return 1;
    case '-': *r = a - b; return 1;
    case '*': *r = a * b; return 1;
    case '/': if (!b) return 0; *r = a / b; return 1;
    case '%': if (!b) return 0; *r = a % b; return 1;
    }
    return 0;
}
static int eval_rel(int rel, int c) {           /* c = comparison result (<0, 0, >0) */
    switch (rel) { case R_LT: return c < 0; case R_GT: return c > 0; case R_LE: return c <= 0;
                   case R_GE: return c >= 0; case R_EQ: return c == 0; default: return c != 0; }
}

static void logq(const char *what, const Quad *q) {
    char b[1000]; quad_str(q, b);
    fprintf(logf, "  block B%d: %-8s  %s\n", blockno, what, b);
}

static Node *do_bin(const Quad *q, Node *l, Node *r) {
    int i; long v;
    if (l->kind == NK_CONST && r->kind == NK_CONST && l->leaf.k == A_INT && r->leaf.k == A_INT) {
        if (fold_bin(q->bop, l->leaf.v, r->leaf.v, &v)) { opt_nfold++; logq("FOLD", q); return const_node(mkint(v)); }
        fprintf(logf, "  block B%d: warning: division by zero not folded\n", blockno);
    }
    for (i = 0; i < nn; i++) {
        Node *n = nodes[i];
        if (n->kind == NK_OP && n->bop == q->bop && n->c[0] == l && n->c[1] == r && !n->killed) {
            opt_ncse++; logq("CSE", q); return n;
        }
    }
    Node *n = newnode(NK_OP); n->bop = q->bop; n->c[0] = l; n->c[1] = r;
    return n;
}
static Node *do_un(const Quad *q, Node *l) {
    int i;
    if (l->kind == NK_CONST && l->leaf.k == A_INT) { opt_nfold++; logq("FOLD", q); return const_node(mkint(-l->leaf.v)); }
    for (i = 0; i < nn; i++) {
        Node *n = nodes[i];
        if (n->kind == NK_OP && n->bop == 'u' && n->c[0] == l && !n->killed) { opt_ncse++; logq("CSE", q); return n; }
    }
    Node *n = newnode(NK_OP); n->bop = 'u'; n->c[0] = l;
    return n;
}

static void build(const Quad *q) {
    Node *n, *l, *r, *v; int i;
    switch (q->op) {
    case Q_BIN:  l = val_node(q->a1); r = val_node(q->a2); attach(q->res, do_bin(q, l, r)); break;
    case Q_UN:   l = val_node(q->a1); attach(q->res, do_un(q, l)); break;
    case Q_COPY: attach(q->res, val_node(q->a1)); break;
    case Q_LOAD:
        l = var_node(q->a1); r = val_node(q->a2);
        for (i = 0; i < nn; i++) {
            n = nodes[i];
            if (n->kind == NK_LOAD && n->c[0] == l && n->c[1] == r && !n->killed) {
                opt_ncse++; logq("CSE", q); attach(q->res, n); return;
            }
        }
        n = newnode(NK_LOAD); n->c[0] = l; n->c[1] = r;
        attach(q->res, n);
        break;
    case Q_STORE:
        l = var_node(q->a1); r = val_node(q->a2); v = val_node(q->a3);
        n = newnode(NK_STORE); n->c[0] = l; n->c[1] = r; n->c[2] = v;
        for (i = 0; i < nn; i++)                       /* ALSU 8.5.5: kill loads of this array */
            if (nodes[i]->kind == NK_LOAD && nodes[i]->c[0] == l) nodes[i]->killed = 1;
        break;
    case Q_READ:  n = newnode(NK_READ); attach(q->res, n); break;
    case Q_WRITE: l = val_node(q->a1); n = newnode(NK_WRITE); n->c[0] = l; break;
    case Q_BKPT:  newnode(NK_BKPT); break;
    }
}

/* ---------- regeneration (ALSU 8.5.4, simplified) ----------
 * Rules:
 *  1. Visit DAG nodes in creation order; compute each needed node into a TEMPORARY.
 *  2. User variables are NOT touched inside the block, so every operand is still valid.
 *  3. At the end of the block, copy each node's value into the user variables attached to it.
 *  4. A variable that is overwritten in step 3 but whose OLD value is still read at the end
 *     (as a copy source or by the final jump/return) is first saved in a temporary.        */
static Addr start_addr(Node *n) {          /* operand while the block body is being generated */
    if (n->kind == NK_CONST || n->kind == NK_VAR) return n->leaf;
    return mkvar(n->primary);
}
static Addr end_addr(Node *n) {            /* operand when read at the end of the block */
    if (n->kind == NK_VAR && n->primary[0]) return mkvar(n->primary);   /* saved old value */
    return start_addr(n);
}
static void copy_q(Addr dst, Addr src) { Quad q; memset(&q, 0, sizeof q); q.op = Q_COPY; q.res = dst; q.a1 = src; emit(q); }

static void save_if_overwritten(Node *n) {
    if (n && n->kind == NK_VAR && !n->primary[0] && mapget(n->leaf.s) != n) {
        Addr t = newtemp(); strcpy(n->primary, t.s);
        copy_q(t, n->leaf);
    }
}

static void regenerate(const Quad *term, int termKind, Node *t0, Node *t1) {
    int i, k;

    /* which nodes are needed: side effects, nodes with user variables attached, operands of those */
    if (t0) t0->needed = 1;
    if (t1) t1->needed = 1;
    for (i = nn - 1; i >= 0; i--) {
        Node *n = nodes[i];
        if (n->kind == NK_STORE || n->kind == NK_READ || n->kind == NK_WRITE || n->kind == NK_BKPT) n->needed = 1;
        else if ((n->kind == NK_OP || n->kind == NK_LOAD) && has_user_label(n)) n->needed = 1;
        if (n->needed) for (k = 0; k < 3; k++) if (n->c[k]) n->c[k]->needed = 1;
    }

    /* step 1: body, in creation order */
    for (i = 0; i < nn; i++) {
        Node *n = nodes[i];
        Quad q; memset(&q, 0, sizeof q);
        if (!n->needed) continue;
        switch (n->kind) {
        case NK_OP: case NK_LOAD: case NK_READ:
            for (k = 0; k < n->nl; k++) if (n->labtemp[k]) break;       /* reuse an original temp name */
            strcpy(n->primary, k < n->nl ? n->lab[k] : newtemp().s);
            q.res = mkvar(n->primary);
            if (n->kind == NK_READ) q.op = Q_READ;
            else if (n->kind == NK_LOAD) { q.op = Q_LOAD; q.a1 = start_addr(n->c[0]); q.a2 = start_addr(n->c[1]); }
            else if (n->bop == 'u') { q.op = Q_UN; q.a1 = start_addr(n->c[0]); }
            else { q.op = Q_BIN; q.bop = n->bop; q.a1 = start_addr(n->c[0]); q.a2 = start_addr(n->c[1]); }
            emit(q); break;
        case NK_STORE:
            q.op = Q_STORE; q.a1 = start_addr(n->c[0]); q.a2 = start_addr(n->c[1]); q.a3 = start_addr(n->c[2]);
            emit(q); break;
        case NK_WRITE: q.op = Q_WRITE; q.a1 = start_addr(n->c[0]); emit(q); break;
        case NK_BKPT:  q.op = Q_BKPT; emit(q); break;
        }
    }

    /* step 4: save old values that are still needed after the variables are overwritten */
    save_if_overwritten(t0);
    save_if_overwritten(t1);
    for (i = 0; i < nn; i++) if (nodes[i]->kind == NK_VAR && has_user_label(nodes[i])) save_if_overwritten(nodes[i]);

    /* step 3: final values of the user variables */
    for (i = 0; i < nn; i++) {
        Node *n = nodes[i];
        if (!n->needed && n->kind != NK_CONST && n->kind != NK_VAR) continue;
        for (k = 0; k < n->nl; k++)
            if (!n->labtemp[k]) copy_q(mkvar(n->lab[k]), end_addr(n));
    }

    /* the block's final jump / return */
    if (termKind == Q_GOTO) { Quad q; memset(&q, 0, sizeof q); q.op = Q_GOTO; q.label = term->label; emit(q); }
    else if (termKind == Q_IFGOTO) { Quad q = *term; q.a1 = end_addr(t0); q.a2 = end_addr(t1); emit(q); }
    else if (termKind == Q_RET)    { Quad q = *term; q.a1 = end_addr(t0); emit(q); }
}

static void process_block(const Quad *q, int s, int e) {
    int i = s, te = e, termKind = 0;
    const Quad *term = NULL;
    Node *t0 = NULL, *t1 = NULL;
    nn = 0; nv = 0; blockno++;

    if (q[i].op == Q_LABEL) { emit(q[i]); i++; }
    if (te > i && (q[te - 1].op == Q_GOTO || q[te - 1].op == Q_IFGOTO || q[te - 1].op == Q_RET)) term = &q[--te];
    for (; i < te; i++) build(&q[i]);

    if (term) {
        termKind = term->op;
        if (term->op == Q_IFGOTO) {
            Node *l = val_node(term->a1), *r = val_node(term->a2);
            if (l->kind == NK_CONST && r->kind == NK_CONST && l->leaf.k == r->leaf.k) {
                int c = l->leaf.k == A_INT ? (l->leaf.v < r->leaf.v ? -1 : l->leaf.v > r->leaf.v)
                                           : strcmp(l->leaf.s, r->leaf.s);
                opt_nfold++; logq("FOLD", term);
                termKind = eval_rel(term->rel, c) ? Q_GOTO : 0;   /* always taken -> goto; never taken -> removed */
            } else { t0 = l; t1 = r; }
        } else if (term->op == Q_RET) {
            t0 = val_node(term->a1);
        }
    }
    regenerate(term, termKind, t0, t1);
}

int optimize(const Quad *in, int n, Quad *o, FILE *log) {
    int i, s;
    char *leader = calloc(n + 2, 1);
    out = o; nout = 0; logf = log; blockno = 0;
    if (n == 0) return 0;
    leader[0] = 1;
    for (i = 0; i < n; i++) {
        if (in[i].op == Q_LABEL) leader[i] = 1;
        if (in[i].op == Q_GOTO || in[i].op == Q_IFGOTO || in[i].op == Q_RET) leader[i + 1] = 1;
    }
    for (s = 0; s < n; ) {
        for (i = s + 1; i < n && !leader[i]; i++) ;
        process_block(in, s, i);
        s = i;
    }
    opt_nblocks = blockno;
    free(leader);
    return nout;
}