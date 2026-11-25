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

    switch(nodetype){
        case NUM_T:
        case ID_T:
            node->type = TYPE_INT;
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

        case ASGN_T:
            if(r->type != TYPE_INT)
                typeError("assignment requires an integer-valued expression");
            node->type = -1;
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
    return node;
}