#include <stdio.h>
#include <string.h>
#include "tree.h"
#include <stdlib.h>

//{0=num,1=read,2=write,3=connector,4=+,5=-,6=*,7=/,8=var}
//root->type = 1 (int)
struct tnode* createTree(int val, int type, char* c, struct tnode *l, struct tnode *r){
    tnode * root=(tnode *)malloc(sizeof(tnode));
    root->nodetype=type;
    root->right=r;
    root->left=l;
    root->val=val;
    root->type=TYPE_INT;
    root->varname=c;
    return root;
}