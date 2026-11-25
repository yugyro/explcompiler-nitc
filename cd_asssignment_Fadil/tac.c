/* tac.c - storing and printing three address code */
#include <string.h>
#include <stdlib.h>
#include "tac.h"

Quad code[MAXCODE];
int  ncode = 0;
static int tempcount = 0, labelcount = 0;

char *newtemp(void)
{
    char buf[32];
    sprintf(buf, "t%d", ++tempcount);
    return strdup(buf);
}

int newlabel(void)
{
    return ++labelcount;
}

void emit(const char *op, const char *a1, const char *a2, const char *res)
{
    if (ncode >= MAXCODE) {
        fprintf(stderr, "too much code\n");
        exit(1);
    }
    strcpy(code[ncode].op,  op);
    strcpy(code[ncode].a1,  a1);
    strcpy(code[ncode].a2,  a2);
    strcpy(code[ncode].res, res);
    ncode++;
}

void print_code(Quad *q, int n, FILE *fp)
{
    int i;
    for (i = 0; i < n; i++) {
        if (!strcmp(q[i].op, "label"))
            fprintf(fp, "%s:\n", q[i].res);
        else if (!strcmp(q[i].op, "goto"))
            fprintf(fp, "\tgoto %s\n", q[i].res);
        else if (!strcmp(q[i].op, "ifFalse"))
            fprintf(fp, "\tifFalse %s goto %s\n", q[i].a1, q[i].res);
        else if (!strcmp(q[i].op, "read"))
            fprintf(fp, "\tread %s\n", q[i].res);
        else if (!strcmp(q[i].op, "write"))
            fprintf(fp, "\twrite %s\n", q[i].a1);
        else if (!strcmp(q[i].op, "="))
            fprintf(fp, "\t%s = %s\n", q[i].res, q[i].a1);
        else if (!strcmp(q[i].op, "uminus"))
            fprintf(fp, "\t%s = - %s\n", q[i].res, q[i].a1);
        else
            fprintf(fp, "\t%s = %s %s %s\n", q[i].res, q[i].a1, q[i].op, q[i].a2);
    }
}
