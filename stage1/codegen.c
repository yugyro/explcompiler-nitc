#include "codegen.h"
#include <stdio.h>
#include "tree.h"

#define min(a,b) (((a) < (b)) ? (a) : (b))
#define max(a,b) (((a) > (b)) ? (a) : (b))

int reg[20];
void initialize(){
    for(int i=0;i<20;i++)
    {
        reg[i]=0;
    }
}

int freereg(int num){
    if(num<0 || num>=20)
    return 0;
    reg[num]=0;
    return 1;
}

int getreg(){
    for(int i=0;i<20;i++)
    {
        if(reg[i]==0)
        {
            return i;
        }
    }
    return -1;
}

int codegen(struct tnode * t,FILE * target_file){
    int lreg=-1;
    int rreg=-1;
    if(t->left)
    {
        lreg=codegen(t->left,target_file);
    }
    if(t->right)
    {
        rreg=codegen(t->right,target_file);
    }
    if(!t->left && !t->right)
    {
        int p=getreg();
        fprintf(target_file,"MOV R%d,%d\n",p,t->val);
        reg[p]=1;
        return p;
    }
    else if(*(t->op)=='+')
    {
        fprintf(target_file,"ADD R%d,R%d\n",min(lreg,rreg),max(lreg,rreg));
        freereg(max(lreg,rreg));
    }
    else if(*(t->op)=='*')
    {
        fprintf(target_file,"MUL R%d,R%d\n",min(lreg,rreg),max(lreg,rreg));
        freereg(max(lreg,rreg));
    }
    else if(*(t->op)=='-')
    {
        fprintf(target_file,"SUB R%d,R%d\n",min(lreg,rreg),max(lreg,rreg));
        freereg(max(lreg,rreg));
    }
    else if(*(t->op)=='/')
    {
        fprintf(target_file,"DIV R%d,R%d\n",min(lreg,rreg),max(lreg,rreg));
        freereg(max(lreg,rreg));
    }
    return min(lreg,rreg);
}

