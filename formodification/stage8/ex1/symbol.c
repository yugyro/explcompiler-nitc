#include "symbol.h"
#include "exptree.h"
#include "typetable.h"
#include "classtable.h"


int bindAddress = STACK_BASE ;
int localBinding = -1;

struct Lsymbol * createLSTFromParams(struct ParamStruct * paramList) {
    struct Lsymbol * Lhead = NULL ;
    struct ParamStruct * temp = paramList ;
    int count = countParameterList(paramList);
    int i = 1;

    int bindingOffset = -3 ;

    while(temp) {
        int paramSize ;

        /*
         * Class-valued parameters are passed as one-word object references.
         * Their TypeTable pointer is NULL, so do not dereference temp->type.
         */
        if(temp->Ctype != NULL) {
            paramSize = 2; /* object pointer + VFT pointer */
        } else if(temp->isPtr == 1 || temp->type == NULL) {
            paramSize = 1;
        } else {
            paramSize = temp->type->size;
        }

        Lhead = insertToLocalTable(Lhead, temp->name, temp->type, temp->isPtr, paramSize);

        struct Lsymbol * newEntry = findLocalSymbol(Lhead, temp->name);

        if(newEntry != NULL) {
            newEntry->Ctype = temp->Ctype;
            int wordsAfter = 0;
            for (struct ParamStruct *q = temp->next; q; q = q->next) {
                wordsAfter += (q->Ctype != NULL) ? 2 :
                              ((q->isPtr || q->type == NULL) ? 1 : q->type->size);
            }
            newEntry->binding = -3 - wordsAfter;
        }
        temp = temp->next ;
        i++ ;
    }
    localBinding = 1;
    return Lhead ;
}

struct ParamStruct * createParamStructNode(char * name, struct TypeTable * type,bool isPtr, struct Classtable * Ctype) {
    struct ParamStruct * node = (struct ParamStruct *) malloc(sizeof(struct ParamStruct));

    node->name = name ;
    node->type = type ;
    node->Ctype = Ctype;
    node->isPtr = isPtr ;
    node->next = NULL ;

    return node ;
}

struct ParamStruct * insertToParamList(struct ParamStruct * head,char * name, struct TypeTable * type, bool isPtr, struct Classtable * Ctype) {
    struct ParamStruct * node = createParamStructNode(name,type,isPtr,Ctype);

    struct ParamStruct * temp = head ;
    if(temp == NULL) {
        head = node ;
        return head ;
    }
    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;

    return head ;
}

struct ParamStruct * findinParamList(struct ParamStruct * head, char * name) {
    struct ParamStruct * temp = head ;

    while(temp) {
        if(strcmp(temp->name,name) == 0) {
            return temp ;
        }
        temp = temp->next ;
    }

    return NULL ;
}

struct Gsymbol* createSymbolNode(char* name, struct TypeTable * type,bool isPtr, int size1, int flabel) {
    struct Gsymbol * node = (struct Gsymbol *) malloc(sizeof(struct Gsymbol));

    node->name = name ;
    node->type = type ;
    node->Ctype = NULL;
    node->size1 = size1 ;
    node->next = NULL ;
    node->isPtr = isPtr ;
    node->binding = bindAddress ;
    bindAddress += size1 ;
    node->flabel = flabel ;
    return node ;
}

struct Lsymbol *createLocalSymbolNode(char * name ,struct TypeTable * type, bool isPtr,int size) {
    struct Lsymbol * node = (struct Lsymbol *) malloc(sizeof(struct Lsymbol));

    node->name = name ;
    node->type = type ;
    node->Ctype = NULL;
    node->next = NULL ;
    node->isPtr = isPtr ;
    node->size = size ;
    node->binding = localBinding;
    localBinding += size ;
    return node ;
}

struct Lsymbol * findLocalSymbol(struct Lsymbol * head, char * name) {
    if(!head) {
        return NULL ;
    }
    struct Lsymbol * temp = head ;

    while(temp) {
        if(strcmp(temp->name,name) == 0) {
            return temp ;
        }
        temp = temp->next ;
    }
    return NULL ;
}


struct Gsymbol * find(struct Gsymbol * head, char * name) {
    if(!head) return NULL;

    struct Gsymbol * temp = head;
    while(temp) {
        if(strcmp(temp->name,name) == 0) {
            return temp ;
        }
        temp = temp->next ;
    }

    return NULL ;
}

struct Lsymbol * insertToLocalTable(struct Lsymbol * head, char * name, struct TypeTable * type, bool isPtr,int size) {
    if(findLocalSymbol(head,name) != NULL) {
        printf("Redeclaration of variable of variable %s\n",name);
        exit(1);
    }

    struct Lsymbol * temp = head ;
    struct Lsymbol * node = createLocalSymbolNode(name,type,isPtr,size);

    if(temp == NULL) {
        head = node ;
        return head ;
    }

    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;
    return head ;
}

struct Gsymbol * insertTable(struct Gsymbol * head,char * name, struct TypeTable * type,bool isPtr, int size1,int flabel) {
    if(find(head,name) != NULL) {
        printf("Redeclaration of variable of global variable %s\n",name);
        exit(1);
    } 

    struct Gsymbol * temp = head ;
    struct Gsymbol * node = createSymbolNode(name,type,isPtr,size1,flabel); 

    if(temp == NULL) {
        head = node ;
        return head ;
    } 
    
    while(temp->next) {
        temp = temp->next ;
    }
    temp->next = node ;
    return head ;
}

void printSymbolTable(struct Gsymbol * head) {
    struct Gsymbol * node = head ;

    while(node) {
        printf("Name: %s, Type: %s%s, Size: %d\n",
               node->name,
               node->Ctype ? node->Ctype->name : (node->type ? node->type->name : "<class>"),
               node->isPtr ? "*" : "",
               node->size1);
        node = node->next ;
    }
}

int countParameterList(struct ParamStruct * paramlist) {
    struct ParamStruct * temp = paramlist ;
    int count = 0;

    while(temp) {
        count++ ;
        temp = temp->next ;
    }
    return count ;
}

void printLocalSymbolTable(struct Lsymbol * head) {
    struct Lsymbol * node = head ;

    while(node) {
        printf("Name: %s, Type: %s%s ",
               node->name,
               node->Ctype ? node->Ctype->name : (node->type ? node->type->name : "<class>"),
               node->isPtr ? "*" : "");
        node = node->next ;
    }
}

void printtemp(struct ParamStruct * head) {
    struct ParamStruct * temp = head ;

    while(temp) {
        printf("Name: %s, Type: %s\n", temp->name, temp->Ctype ? temp->Ctype->name : (temp->type ? temp->type->name : "<class>"));
        temp = temp->next ;
    }
}

void insertParamListToGST(struct Gsymbol * head, char * name ,struct ParamStruct * paramList) {
    struct Gsymbol * temp = find(head,name);
    if(temp == NULL) {
        printf("Error: Cannot add param list, function '%s' not found in GST\n", name);
        exit(1);
    }
    temp->paramList = paramList ;
}

void compareParamList(struct Gsymbol * currentFunc,struct ParamStruct * currParamList) {
    struct ParamStruct * temp = currentFunc->paramList ;
    struct ParamStruct * compare = currParamList ;

    while(temp != NULL && compare != NULL ) {
        if((strcmp(temp->name,compare->name) != 0) || 
           (temp->type != compare->type) ||
           (temp->Ctype != compare->Ctype) ||
           (temp->isPtr != compare->isPtr)) {
            printf("Type/Name/Pointer mismatch in parameter...\n");
            exit(1);
        }
        temp = temp->next ;
        compare = compare->next ;
    }
    temp = currentFunc->paramList ;
    compare = currParamList ;

    while(temp != NULL && compare != NULL ) {
        if((strcmp(temp->name,compare->name) != 0) ||
           (temp->Ctype != compare->Ctype) ||
           (temp->type && compare->type && strcmp(temp->type->name,compare->type->name)!= 0)) {
            printf("Type/Name mismatch in parameter for function definition\n");
            exit(1);
        }
        temp = temp->next ;
        compare = compare->next ;
    }

    if(temp != NULL || compare != NULL) {
        printf("Too few args or Too many args\n");
        exit(1);
    }
        
}

void checkDeclAndDefn(struct Gsymbol * currentFunc,struct TypeTable * type) {
    char * funcType = currentFunc->type->name ;

    if(strcmp(funcType,type->name) != 0) {
        printf("Function Declaration return type and Definition return type does not match\n");
        exit(1);
    }
}