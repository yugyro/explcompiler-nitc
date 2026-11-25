#include "exptree.h"
#include "symbol.h"
#include "typetable.h"

struct tnode * createTreeNode(int isPtr,int val, struct TypeTable * type, char* varname, int nodetype, struct Gsymbol * Gentry,struct tnode *l, struct tnode * m,struct tnode *r) {
    struct tnode * temp = (struct tnode * ) malloc(sizeof(struct tnode));

    temp->left = l ;
    temp->right = r ;
    temp->mid = m ;

    temp->isPtr = isPtr ;
    temp->Gentry = Gentry ;
    temp->type = type ;
    temp->nodetype = nodetype ;

    temp->val = val ; 
    temp->varname = varname ;
    temp->argList = NULL ;
    temp->Lentry = NULL ;

    return temp ;
}

struct tnode * connectNode(struct tnode * left, struct tnode * right) {

    return createTreeNode(0,NO_VAL,T_VOID,NULL,CONNECT_NODE,NULL,left,NULL,right);
}

struct tnode * ipOpNode(int nodetype ,struct tnode * left) {
    //Will only have left side ID present since its read or write node
    return createTreeNode(0,NO_VAL,T_VOID,NULL,nodetype,NULL,left,NULL,NULL);
}

struct tnode * loopNode(int nodetype, struct tnode * left, struct tnode * right) {

    return createTreeNode(0,NO_VAL,T_VOID,NULL,nodetype,NULL,left,NULL,right);
}

struct tnode * flowControlNode(int nodetype) {
    //Continue and Break Stmt node
    return createTreeNode(0,NO_VAL,T_VOID,NULL,nodetype,NULL,NULL,NULL,NULL);
}

int isArithmeticOp(int nodeType) {
    return (nodeType == ADD_NODE || nodeType == MINUS_NODE ||
            nodeType == MUL_NODE || nodeType == DIV_NODE);
}

int isRelop(int nodeType) {
    return (nodeType == LT_NODE || nodeType == LE_NODE ||
            nodeType == GT_NODE || nodeType == GE_NODE ||
            nodeType == EQ_NODE || nodeType == NE_NODE );
}

int isLogical(int nodeType) {
    return (nodeType == AND_NODE || nodeType == OR_NODE ||
            nodeType == NOT_NODE) ;
}

/* Check if a type is a user-defined type (has fields and is not a base type) */
int isUserDefinedType(struct TypeTable * type) {
    if(type == NULL) return 0;
    if(type == T_INT || type == T_STR || type == T_BOOL || 
       type == T_VOID || type == T_INT_PTR || type == T_STR_PTR) return 0;
    if(type->fields != NULL) return 1;
    return 0;
}

struct tnode * exprNode(int nodeType, struct tnode * left , struct tnode * right) {
    if(isLogical(nodeType)) {
        // Handle NOT, which only has a left operand
        if (right == NULL) { 
            if (left->type != T_BOOL) {
                printf("Type Mismatch: NOT operator requires a boolean operand\n");
                exit(1);
            }
        }
        // Handle AND, OR
        else if(left->type != T_BOOL || right->type != T_BOOL) {
            printf("Type Mismatch: Logical operators (AND, OR) require boolean operands\n");
            exit(1);
        }
        return createTreeNode(0,NO_VAL, T_BOOL, NULL, nodeType, NULL, left, NULL, right);
    }

    if(isArithmeticOp(nodeType)) {
        if(left->type != T_INT || right->type != T_INT) {
            printf("Type Mismatch: Arithmetic operators require integer operands\n");
            exit(1);
        }
        return createTreeNode(0,NO_VAL, T_INT, NULL, nodeType, NULL, left, NULL, right);
    }

    if(isRelop(nodeType)) {
        // Allow user-defined type comparisons with == and != (for null checks)
        if (left->type == T_INT && right->type == T_INT) {
            // int-to-int comparison is fine for all relops
        } 
        else if (left->type == T_STR && right->type == T_STR) {
            // str-to-str comparison is only fine for == and !=
            if (nodeType != EQ_NODE && nodeType != NE_NODE) {
                printf("Type Mismatch: Relational operators (<, >, <=, >=) are not valid for strings.\n");
                exit(1);
            }
        }
        // Allow comparing user-defined type with null
        else if ((isUserDefinedType(left->type) && right->nodetype == NULL_NODE) ||
                 (left->nodetype == NULL_NODE && isUserDefinedType(right->type))) {
            if (nodeType != EQ_NODE && nodeType != NE_NODE) {
                printf("Type Mismatch: Only == and != comparisons allowed with null\n");
                exit(1);
            }
        }
        // Allow comparing two variables of the same user-defined type
        else if (isUserDefinedType(left->type) && isUserDefinedType(right->type) && left->type == right->type) {
            if (nodeType != EQ_NODE && nodeType != NE_NODE) {
                printf("Type Mismatch: Only == and != comparisons allowed for user-defined types\n");
                exit(1);
            }
        }
        else {
            // Any other combination is an error
            printf("Type Mismatch: Cannot compare %s with %s\n", left->type->name, right->type->name);
            exit(1);
        }
        
        // If we survived the checks, the types are compatible
        return createTreeNode(0,NO_VAL, T_BOOL, NULL, nodeType, NULL, left, NULL, right);
    }

    printf("Unknown Expression in exprNode\n");
    exit(1) ;
}

struct tnode * assignNode(struct tnode * left, struct tnode * right) {
    if(left->nodetype == DEREF_NODE) {
        if(left->type == T_INT) {
            if(right->type != T_INT) {
                printf("Type Mismatch\n");
                exit(1);
            }
        } else if(left->type == T_STR) {
            if(right->type != T_STR) {
                printf("Type Mismatch\n");
                exit(1);
            }
        }
    }

    // Assignment to user-defined type variable
    if(isUserDefinedType(left->type)) {
        // Allow: udt_var = alloc();
        if(right->nodetype == ALLOC_NODE) {
            right->type = left->type; // Set alloc's type to the left side's type
        }
        // Allow: udt_var = null;
        else if(right->nodetype == NULL_NODE) {
            // null can be assigned to any user-defined type
        }
        // Allow: udt_var = another_udt_var (same type) or function returning same type
        else if(right->type != left->type) {
            printf("Type Mismatch: Cannot assign %s to %s\n", right->type->name, left->type->name);
            exit(1);
        }
        return createTreeNode(0,NO_VAL,T_VOID,NULL,ASSIGN_NODE,NULL,left,NULL,right);
    }

    if(left->nodetype == ID_NODE || left->nodetype == ARR_NODE || left->nodetype == FIELD_NODE) {
        if(left->type == T_INT) {
            // Allow assigning initialize() which returns int
            if(right->nodetype == INIT_NODE) {
                // initialize returns int, OK
            } else if(right->nodetype == FREE_NODE) {
                // free returns int, OK  
            } else if(right->type != T_INT) {
                printf("Type Mismatch: Cannot assign non-integer to integer variable\n");
                exit(1);
            }
        }
        if(left->type == T_STR) {
            if(right->type != T_STR) {
                printf("Type Mismatch: Cannot assign non-string to string variable\n");
                exit(1);
            }
        }
    }
    return createTreeNode(0,NO_VAL,T_VOID,NULL,ASSIGN_NODE,NULL,left,NULL,right);
}


struct tnode * exitNode(int nodeType) {
    return createTreeNode(0,NO_VAL,T_VOID,NULL,EXIT_NODE,NULL,NULL,NULL,NULL);
}

struct tnode * ifelseNode(int nodeType ,struct tnode * left, struct tnode * mid ,struct tnode * right) {
    return  createTreeNode(0,NO_VAL,T_VOID,NULL,nodeType,NULL,left,mid,right);
}

struct tnode * setTypeId(struct Gsymbol * Ghead ,struct Lsymbol * Lhead ,struct tnode * idNode,struct tnode * leftExprNode, struct tnode * rightExprNode) {
    struct Lsymbol * Ltemp = findLocalSymbol(Lhead,idNode->varname);

    //Check Local Symbol Table first
    if(Ltemp != NULL) {
        idNode->left = leftExprNode ;
        idNode->right = rightExprNode ;
        idNode->type = Ltemp->type ;
        idNode->Lentry = Ltemp ;
        idNode->isPtr = Ltemp->isPtr ;
        return idNode ;
    }
    struct Gsymbol * Gtemp = find(Ghead , idNode->varname) ;

    //Check global symbol table
    if(Gtemp == NULL) {
        printf("Id '%s' not declared\n", idNode->varname);
        exit(1);
    }

    idNode->left = leftExprNode ;
    idNode->right = rightExprNode ;
    idNode->type = Gtemp->type ;
    idNode->Gentry = Gtemp ;
    idNode->isPtr = Gtemp->isPtr ;

    return idNode ;
}

struct tnode * pointerNode(struct Gsymbol * head,int nodeType,struct tnode * node, struct tnode * left, struct tnode * right) {
    struct Gsymbol * symbolNode = find(head, node->varname);

    struct tnode * temp;

    if (nodeType == DEREF_NODE) { // *ptr
        if (node->isPtr == 0) {
            printf("Error: Cannot dereference a non-pointer variable '%s'\n", node->varname);
            exit(1);
        }

        struct TypeTable * pointeeType;
        if (node->type == T_INT_PTR) {
            pointeeType = T_INT;
        } else if (node->type == T_STR_PTR) {
            pointeeType = T_STR;
        } else {
            pointeeType = node->type; // fallback, shouldn't normally happen
        }

        // The result *ptr is NOT a pointer, and carries the pointee's type
        temp = createTreeNode(0, NO_VAL, pointeeType, node->varname, DEREF_NODE, symbolNode, NULL, NULL, NULL);
    }
    else if (nodeType == ADDR_NODE) { // &var
    if (node->isPtr == 1) {
        printf("Error: Cannot take the address of a pointer '%s'\n", node->varname);
        exit(1);
    }

    struct TypeTable * ptrType;
    if (node->type == T_INT) {
        ptrType = T_INT_PTR;
    } else if (node->type == T_STR) {
        ptrType = T_STR_PTR;
    } else {
        ptrType = node->type; // fallback, shouldn't normally happen
    }

    // The result &var IS a pointer
    temp = createTreeNode(1, NO_VAL, ptrType, node->varname, ADDR_NODE, symbolNode, NULL, NULL, NULL);
}

    temp->left = node;
    return temp;
}
struct tnode * returnNode(struct Gsymbol * funcEntry, int nodetype, struct tnode * node) {

    if(funcEntry == NULL) {
        printf("Error: Return statement outside function\n");
        exit(1) ;
    }

    // Allow returning user-defined types
    if(isUserDefinedType(node->type) && isUserDefinedType(funcEntry->type)) {
        if(node->type != funcEntry->type) {
            printf("Error: Return type mismatch in function '%s'.\n", funcEntry->name);
            exit(1);
        }
    }
    // Allow returning null for user-defined return type
    else if(node->nodetype == NULL_NODE && isUserDefinedType(funcEntry->type)) {
        // OK
    }
    // Type check: return expression type must match function return type
    else if(node->type != funcEntry->type) {
        printf("Error: Return type mismatch in function '%s'. Expected %s, got %s\n", 
               funcEntry->name, funcEntry->type->name, node->type->name);
        exit(1);
    }
    
    return createTreeNode(0,NO_VAL,node->type,NULL,RETURN_NODE,funcEntry,node,NULL,NULL);
}

struct tnode * funcNode(struct Gsymbol * Ghead ,struct tnode * node, struct tnode * arglist) {

    struct Gsymbol * funcEntry = find(Ghead,node->varname);

    if(funcEntry == NULL) {
        printf("Error : Function '%s' not declared and called\n", node->varname);
        exit(1) ;
    }

    struct ParamStruct * paramHead = funcEntry->paramList ;
    struct tnode * argHead = arglist ;

    // Type check arguments against parameters
    while(argHead != NULL &&  paramHead != NULL) {
        // Allow null to be passed where a user-defined type is expected
        if(argHead->nodetype == NULL_NODE && isUserDefinedType(paramHead->type)) {
            // OK
        }
        else if(argHead->type != paramHead->type ) {
            printf("Type mismatch for argument in function call to '%s'\n", funcEntry->name);
            exit(1);
        }
        paramHead = paramHead->next ;
        argHead = argHead->argList ;
    }

    if(paramHead != NULL) {
        printf("Too few arguments in function call\n");
        exit(1);
    }

    if(argHead != NULL) {
        printf("Too many arguments in function call\n");
        exit(1);
    }

    struct tnode * callNode =  createTreeNode(0,NO_VAL,funcEntry->type,funcEntry->name,FUNC_CALL_NODE,funcEntry,NULL,NULL,NULL);

    node->argList = arglist ;
    callNode->left = node ;

    return callNode ;
}

struct tnode * appendArg(struct tnode * argList, struct tnode * newArg) {
    newArg->argList = NULL ;

    if(argList == NULL) {
        return newArg ;
    }

    struct tnode * temp = argList ;

    while(temp->argList) {
        temp = temp->argList ;
    }
    temp->argList = newArg ;

    return argList ;
}

struct tnode * fieldNode(struct Fieldlist * field,int fieldIndex, struct TypeTable * type, char * name, int nodetype, struct tnode * rightExprNode) {
    if(field->type->fields == NULL) {
        perror("Field access ('.') on a non-tuple type");
        exit(1);
    }

    if(field == NULL) {
        printf("Field '%s' not found in tuple type '%s'", field->name, field->type->name);
        exit(1);
    }
    return createTreeNode(0,fieldIndex, type, name, nodetype, NULL, NULL, NULL, rightExprNode);
}

void printAST(struct tnode *root, int depth)
{
    if (root == NULL)
        return;

    for (int i = 0; i < depth; i++)
        printf("  ");

    printf("Node type: %d", root->nodetype);

    if (root->varname != NULL)
        printf(", Name: %s", root->varname);

    if (root->type != NULL)
        printf(", Type: %s", root->type->name);

    if (root->nodetype == NUM_NODE)
        printf(", Value: %d", root->val);

    printf("\n");

    printAST(root->left, depth + 1);
    printAST(root->mid, depth + 1);
    printAST(root->right, depth + 1);
}