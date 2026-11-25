#ifndef CODEGEN_H
#define CODEGEN_H
#include "tree.h"
#include <stdio.h>

#define VARADDR(c) (4096 + ((c) - 'a'))

void initialize();
int getreg();
int freereg(int i);
int getLabel();

int codegen(struct tnode *t, FILE *target_file);
int evaluate(tnode *t);

#endif