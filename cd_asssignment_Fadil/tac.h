/* tac.h - Three Address Code definitions
 * CS3002E Compiler Design - Course Project 2
 */
#ifndef TAC_H
#define TAC_H
#include <stdio.h>

#define MAXCODE 2000

/* One three-address instruction (quadruple):  res = a1 op a2
 *   op = "+", "-", "*", "/", "%", "<", ">", "<=", ">=", "==", "!=", "&&", "||"
 *        "uminus"  : res = - a1
 *        "="       : res = a1
 *        "ifFalse" : ifFalse a1 goto res
 *        "goto"    : goto res
 *        "label"   : res:
 *        "read"    : read res
 *        "write"   : write a1
 */
typedef struct {
    char op[10];
    char a1[32];
    char a2[32];
    char res[32];
} Quad;

/* code generation (tac.c) */
extern Quad code[MAXCODE];
extern int  ncode;
char *newtemp(void);
int   newlabel(void);
void  emit(const char *op, const char *a1, const char *a2, const char *res);
void  print_code(Quad *q, int n, FILE *fp);

/* optimizer (optimizer.c) */
void optimize(Quad *in, int n, Quad *out, int *nout, int *folds, int *cses);

#endif
