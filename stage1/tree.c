#include <stdio.h>
#include "tree.h"
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

struct tnode * makeLeafNode(int n){
    struct tnode * leaf = (struct tnode *)malloc(sizeof(struct tnode));
    leaf->val=n;
    leaf->left=NULL;
    leaf->right=NULL;
    leaf->op=NULL;
    return leaf;
}

struct tnode * makeOperatorNode(char * op,struct tnode* l,struct tnode * r){
    tnode * opNode= (tnode*)malloc(sizeof(tnode));
    opNode->left=l;
    opNode->right=r;
    opNode->op=op;
    return opNode;
}

