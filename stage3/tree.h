#ifndef TREE_H
#define TREE_H

typedef struct tnode {
    int val;
    int type;          // data type: TYPE_INT or TYPE_BOOL (statements: -1, typeless)
    char *varname;
    int nodetype;       // kind of node
    struct tnode *left, *right, *third;   // 'third' used only by IF_T (else-branch)
} tnode;

#define YYSTYPE tnode*

/* data types */
#define TYPE_INT  1
#define TYPE_BOOL 0

/* node kinds */
#define NUM_T        1
#define ID_T         2
#define PLUS_T       3
#define MINUS_T      4
#define MUL_T        5
#define DIV_T        6
#define ASGN_T       7
#define READ_T       8
#define WRITE_T      9
#define CONNECTOR_T  10
#define IF_T         11
#define WHILE_T      12
#define LT_T         13
#define GT_T         14
#define LE_T         15
#define GE_T         16
#define NE_T         17
#define EQREL_T      18
#define BREAK_T      19
#define CONTINUE_T   20

struct tnode* createTree(int val, int nodetype, char* varname, struct tnode *l, struct tnode *r);
struct tnode* createIf(struct tnode *cond, struct tnode *thenPart, struct tnode *elsePart);
void typeError(char *msg);

#endif