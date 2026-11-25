#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tree.h"

void typeError(char *msg){
    fprintf(stderr, "Type Error: %s\n", msg);
    exit(1);
}

struct tnode* createTree(int val, int nodetype, char* varname, struct tnode *l, struct tnode *r){
    tnode *node = (tnode*) malloc(sizeof(tnode));
    node->val = val;
    node->nodetype = nodetype;
    node->varname = varname;
    node->left = l;
    node->right = r;
    node->third = NULL;
    node->type = TYPE_INT;
    node->entry=NULL;

    switch(nodetype){
        case NUM_T:
            node->type = TYPE_INT;
            break;
        case ID_T:
            break;
        
        case STRCONST_T:
            node->type = TYPE_STR;
            break;

        case PLUS_T: case MINUS_T: case MUL_T: case DIV_T:
            if(l->type != TYPE_INT || r->type != TYPE_INT)
                typeError("arithmetic operator requires integer operands");
            node->type = TYPE_INT;
            break;

        case LT_T: case GT_T: case LE_T: case GE_T: case NE_T: case EQREL_T:
            if(l->type != TYPE_INT || r->type != TYPE_INT)
                typeError("relational operator requires integer operands");
            node->type = TYPE_BOOL;
            break;

        case WRITE_T:
            if(l->type != TYPE_INT)
                typeError("write requires an integer-valued expression");
            node->type = -1;
            break;

        case WHILE_T:
            if(l->type != TYPE_BOOL)
                typeError("while-guard must be a boolean expression");
            node->type = -1;
            break;

        default:
            node->type = -1;   /* CONNECTOR_T, READ_T, BREAK_T, CONTINUE_T */
            break;
    }
    return node;
}
struct tnode * resolveVar(struct tnode * idnode){
    Gsymbol * entry=Lookup(idnode->varname);
    if(!entry)
    {
        fprintf(stderr,"Variable %s called without declaring",idnode->varname);
        exit(1);
    }
    idnode->entry=entry;
    idnode->type=(entry->type==TYPE_INT_VAR)?TYPE_INT:TYPE_STR;
    return idnode;
}
struct tnode * resolveScalar(struct tnode * idnode){
    resolveVar(idnode);
    if(idnode->entry->size>1)
    {
        typeError("array variable used without index");
    }
    return idnode;
}

struct tnode* createArrAccess(struct tnode *idnode, struct tnode *rowExpr, struct tnode *colExpr){
    resolveVar(idnode);
    if(idnode->entry->size <= 1)
        typeError("indexing a non-array variable");
    if(rowExpr->type != TYPE_INT || (colExpr && colExpr->type != TYPE_INT))
        typeError("array index must be an integer expression");
    if((idnode->entry->cols > 1) != (colExpr != NULL))
        typeError("array dimension mismatch");

    tnode *node = (tnode*) malloc(sizeof(tnode));
    node->val = 0;
    node->nodetype = ARRACC_T;
    node->varname = idnode->varname;
    node->left = rowExpr;
    node->right = colExpr;     // NULL for 1D
    node->third = NULL;
    node->entry = idnode->entry;
    node->type = (idnode->entry->type == TYPE_STR_VAR) ? TYPE_STR : TYPE_INT;
    return node;
}
struct tnode* createAssign(struct tnode *lvalue, struct tnode *rhs){
    if(rhs->type != lvalue->type)
        typeError("assignment type mismatch between variable and expression");

    tnode *node = (tnode*) malloc(sizeof(tnode));
    node->val = 0;
    node->nodetype = ASGN_T;
    node->varname = NULL;
    node->left = lvalue;
    node->right = rhs;
    node->third = NULL;
    node->entry = NULL;
    node->type = -1;
    return node;
}
struct tnode* createIf(struct tnode *cond, struct tnode *thenPart, struct tnode *elsePart){
    if(cond->type != TYPE_BOOL)
        typeError("if-guard must be a boolean expression");

    tnode *node = (tnode*) malloc(sizeof(tnode));
    node->val = 0;
    node->nodetype = IF_T;
    node->varname = NULL;
    node->left = cond;
    node->right = thenPart;
    node->third = elsePart;
    node->type = -1;
    node->entry=NULL;
    return node;
}