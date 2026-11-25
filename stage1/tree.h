#ifndef TREE_H
#define TREE_H

typedef struct tnode{
        int val;
        char * op;
        struct tnode * right;
        struct tnode * left;
    }tnode;

#define YYSTYPE tnode*

struct tnode * makeLeafNode(int n);
struct tnode * makeOperatorNode(char * op,tnode * l,tnode * r);

#endif

//yacc -d stage1.y
//lex stage1.l
//gcc y.tab.c lex.yy.c tree.c codegen.c -o compiler -ll 
//