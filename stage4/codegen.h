#ifndef CODEGEN_H
#define CODEGEN_H
#include "tree.h"
#include <stdio.h>


void initialize();
int getreg();
int freereg(int i);
int getLabel();

int codegen(struct tnode *t, FILE *target_file);
int evaluate(tnode *t);

#endif