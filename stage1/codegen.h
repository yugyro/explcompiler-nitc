#ifndef CODEGEN
#define CODEGEN
#include "tree.h"
#include <stdio.h>


int getreg();
int freereg(int i);
void initialize();
int codegen(struct tnode * t,FILE * target_file);
#endif