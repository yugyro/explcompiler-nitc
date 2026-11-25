%{
    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "exptree.h"
    #include "symbol.h"
    #include "typetable.h"
    #include "codegen.h"
    #include "classtable.h"

    int yylex(void);
    int yyerror(const char *s);

    FILE * output;
    extern int yylineNum; 
    extern int localBinding ;
    int flabel = 1;
    bool isPointer = false ;

    struct Gsymbol * Ghead  = NULL ;
    struct Lsymbol * Lhead  = NULL ; 
    struct ParamStruct * paramHead = NULL ;
    struct Gsymbol * currentFunc = NULL ;  //Current function for return type checking 

    struct TypeTable * currentType ;
    struct TypeTable * paramType ;
    struct Classtable * paramClassType = NULL;
    struct TypeTable * currLocalType ;
    struct Classtable * currLocalClassType = NULL;

    struct TypeTable * TTableHead = NULL ;
    struct TypeTable * currTupleType = NULL ;

    /* Stage 7 front-end state */
    struct Classtable * CTableHead = NULL;
    struct Classtable * currentClass = NULL;
    struct MemberFunc * currentMethod = NULL;
    struct Classtable * currentClassType = NULL;
    int hasClassDefinitions = 0;
    int methodFlabel = 100;
    int seenMethodDecl = 0;
    char *pendingMemberName = NULL;

    static void stage7Error(const char *msg) {
        fprintf(stderr, "Error: %s at line %d\n", msg, yylineNum);
        exit(1);
    }

    static int checkMethodArgs(struct MemberFunc *m, struct tnode *args) {
        struct ParamStruct *p = m ? m->paramList : NULL;
        struct tnode *a = args;
        while (p && a) {
            if (!((a->nodetype == NULL_NODE && (isUserDefinedType(p->type) || p->Ctype)) ||
                  (p->Ctype ? (p->Ctype == a->Ctype) : (p->type == a->type && p->isPtr == a->isPtr)))) {
                fprintf(stderr, "Error: Argument type mismatch in method '%s' at line %d\n", m->name, yylineNum);
                exit(1);
            }
            p = p->next; a = a->argList;
        }
        if (p || a) {
            fprintf(stderr, "Error: Argument count mismatch in method '%s' at line %d\n", m->name, yylineNum);
            exit(1);
        }
        return 1;
    }

    static struct tnode *makeMethodCall(struct MemberFunc *m, struct tnode *args, struct tnode *receiver) {
        if (!m) stage7Error("Method is not declared in this class");
        checkMethodArgs(m, args);
        struct tnode *n = createTreeNode(0, NO_VAL, m->type, strdup(m->name), METHOD_CALL_NODE, NULL, NULL, NULL, NULL);
        n->argList = args;
        n->left = receiver;
        n->val = m->flabel;
        return n;
    }

    static struct tnode *makeSelfReceiver(void) {
        struct tnode *self = createTreeNode(0, NO_VAL, T_VOID, strdup("self"), ID_NODE, NULL, NULL, NULL, NULL);
        self->Lentry = findLocalSymbol(Lhead, "self");
        if (!self->Lentry) stage7Error("Internal error: self is missing from method local table");
        return self;
    }

    static struct tnode *makeSelfFieldReceiver(struct ClassField *f) {
        if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
        struct tnode *receiver = createTreeNode(0, f->fieldIndex, f->type,
                                                  strdup(f->name), SELF_FIELD_NODE,
                                                  NULL, NULL, NULL, NULL);
        receiver->Ctype = f->ctype;
        receiver->Lentry = findLocalSymbol(Lhead, "self");
        if (!receiver->Lentry) stage7Error("Internal error: self is missing from method local table");
        return receiver;
    }

    static int sameMethodSignature(struct MemberFunc *m, struct TypeTable *ret, struct ParamStruct *p) {
        if (!m || m->type != ret) return 0;
        struct ParamStruct *q = m->paramList;
        while (q && p) {
            if (q->type != p->type || q->Ctype != p->Ctype || q->isPtr != p->isPtr) return 0;
            q = q->next; p = p->next;
        }
        return q == NULL && p == NULL;
    }


    // For user-defined type definitions (Stage 6)
    struct Fieldlist * currFieldHead = NULL ;
    int currFieldIndex = 0 ;
%}


%union {
    struct ParamStruct * paramList ;
    struct tnode * node;
    int number ;
    char * string ;
    struct Fieldlist * flist ;
    struct TypeTable * type ;
}

%token PLUS MINUS STAR DIV AMPERSAND
%token LE LT GT GE EQ NE AND OR NOT ASSIGN
%token IF ELSE WHILE REPEAT UNTIL DO BREAK CONTINUE ENDIF ENDWHILE 
%token DECL ENDDECL BEG END INT STR SEMICOLON THEN READ WRITE EXIT_PR
%token MAIN RETURN 
%token TUPLE DOT 
%token TYPE ENDTYPE NULL_VAL ALLOC INITIALIZE FREE
%token CLASS ENDCLASS SELF NEW

%token <number> NUM 
%token <string> STR_LITERAL
%token <node> ID 

%type <node> program stmt stmtList ipStmt opStmt assignStmt whileStmt doWhileStmt repeatStmt ifStmt
%type <node> IDENTIFIERS EXPR Ret Body ArgList
%type <paramList> ParamList Param
%type <flist> FieldDefList FieldDef 


%right ASSIGN
%left AND OR NOT 
%nonassoc NE EQ LT LE GE GT 
%left PLUS MINUS 
%left STAR DIV  
%left DOT
%left FIELD_BASE 

%%

program     :   TypeDefBlock ClassDefBlock GDeclBlock FDefBlock MainBlock { }
            ;

/* ==================== Type Definition Block (Stage 6) ==================== */

TypeDefBlock:   TYPE TypeDefList ENDTYPE     {}
            |                                      {}
            ;

TypeDefList :   TypeDefList TypeDef          {}
            |   TypeDef                      {}
            ;

TypeDef     :   ID
                {
                    /* Install a placeholder first so a type may contain a field
                       of its own type (for example, List next;). */
                    if (TLookup(TTableHead, $1->varname) != NULL) {
                        fprintf(stderr, "Error: Type '%s' redeclared at line %d\\n",
                                $1->varname, yylineNum);
                        exit(1);
                    }
                    TTableHead = TInstall(TTableHead, $1->varname, 0, NULL);
                    currTupleType = TLookup(TTableHead, $1->varname);
                }
                '{' FieldDefList '}' {
                    int fieldCount = 0;
                    struct Fieldlist *temp = $4;
                    for (; temp != NULL; temp = temp->next) {
                        fieldCount++;
                    }
                    if (fieldCount > 8) {
                        fprintf(stderr, "Error: Too many member fields in type '%s' (max 8)\\n",
                                $1->varname);
                        exit(1);
                    }
                    currTupleType->fields = $4;
                    currTupleType->size = fieldCount;
                    currTupleType = NULL;
                }
            ;

/* ==================== Stage 7 Class Definitions ==================== */

ClassDefBlock : CLASS ClassDefList ENDCLASS { }
              |                           { }
              ;

ClassDefList  : ClassDefList ClassDef { }
              | ClassDef              { }
              ;

ClassDef      : ID '{' {
                              if (TLookup(TTableHead, $1->varname)) {
                                  fprintf(stderr, "Error: Class '%s' conflicts with an existing user-defined type at line %d\n", $1->varname, yylineNum);
                                  exit(1);
                              }
                              CTableHead = CInstall(CTableHead, $1->varname, NULL);
                              currentClass = CLookup(CTableHead, $1->varname);
                              hasClassDefinitions = 1;
                              seenMethodDecl = 0;
                         }
                DECL ClassMemberList ENDDECL MethodDefList '}'
                         {
                              for (struct MemberFunc *m = currentClass->methods; m; m = m->next) {
                                  if (!m->isDefined) {
                                      fprintf(stderr, "Error: Method '%s' declared but not defined in class '%s'\n", m->name, currentClass->name);
                                      exit(1);
                                  }
                              }
                              currentClass = NULL; currentMethod = NULL;
                         }
              ;

ClassMemberList : ClassMemberList ClassMember { }
                |                            { }
                ;

ClassMember     : ClassMemberType ID {
                              pendingMemberName = strdup($2->varname);
                         } ClassMemberTail
                ;

ClassMemberType : INT { currentType = T_INT; currentClassType = NULL; }
                | STR { currentType = T_STR; currentClassType = NULL; }
                | ID  {
                          currentType = TLookup(TTableHead, $1->varname);
                          currentClassType = NULL;
                          if (!currentType) currentClassType = CLookup(CTableHead, $1->varname);
                          if (currentClassType == currentClass) stage7Error("A class cannot contain a field of its own class type in Stage 7");
                          if (!currentType && !currentClassType) {
                              fprintf(stderr, "Error: Undefined member type '%s' at line %d\n", $1->varname, yylineNum);
                              exit(1);
                          }
                      }
                ;

ClassMemberTail : SEMICOLON {
                              if (seenMethodDecl) stage7Error("Class fields must be declared before methods");
                              Class_Finstall(currentClass,
                                  currentClassType ? currentClassType->name : (currentType ? currentType->name : ""),
                                  pendingMemberName, currentType, currentClassType);
                         }
                | '(' ParamList ')' SEMICOLON {
                              if (currentClassType != NULL || currentType == NULL)
                                  stage7Error("Class methods must have a non-class return type");
                              seenMethodDecl = 1;
                              Class_Minstall(currentClass, pendingMemberName, currentType, $2, methodFlabel++);
                         }
                ;

MethodDefList : MethodDefList MethodDef { }
              | MethodDef                { }
              |                          { }
              ;

MethodDef     : GType ID '(' ParamList ')' '{' {
                         if (currentClassType) stage7Error("Class methods cannot return class objects in Stage 7 Task 1");
                         struct MemberFunc *m = Class_Mlookup(currentClass, $2->varname);
                         if (!m) {
                             fprintf(stderr, "Error: Definition of undeclared method '%s' in class '%s' at line %d\n", $2->varname, currentClass->name, yylineNum);
                             exit(1);
                         }
                         if (!sameMethodSignature(m, currentType, $4)) {
                             fprintf(stderr, "Error: Signature mismatch for method '%s' in class '%s' at line %d\n", $2->varname, currentClass->name, yylineNum);
                             exit(1);
                         }
                         if (m->isDefined) stage7Error("Method defined more than once");
                         m->isDefined = 1;
                         currentMethod = m;
                         currentFunc = createSymbolNode($2->varname, currentType, false, 0, m->flabel);
                         currentFunc->paramList = $4;
                         Lhead = createLSTFromParams($4);
                         /* The caller pushes self before the explicit arguments. */
                         Lhead = insertToLocalTable(Lhead, "self", T_VOID, false, 1);
                         findLocalSymbol(Lhead, "self")->binding = -countParameterList($4) - 3;
                         localBinding = 1;
                     }
                LdeclBlock Body '}' {
                         currentFunc->bodyAST = $9;
                         currentFunc->Lentry = Lhead;
                         funcCodeGen(currentFunc, output);
                         currentFunc = NULL;
                         currentMethod = NULL;
                         Lhead = NULL;
                     }
              ;

/* ==================== Main Block ==================== */

MainBlock   :   INT MAIN '(' ')' '{'            {   
                                                    Ghead = insertTable(Ghead,"F0",T_INT,false,0,0); 
                                                    currentFunc = find(Ghead, "F0");
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", "main");
                                                        exit(1);
                                                    }
                                                }
                LdeclBlock Body '}' {
                                        currentFunc->bodyAST = $8;
                                        currentFunc->Lentry = Lhead;

                                        printf("\nAST:\n");
                                        printAST(currentFunc->bodyAST, 0);

                                        funcCodeGen(currentFunc, output);
                                        currentFunc = NULL;
                                        Lhead = NULL;
                                    }
            ;

/* ==================== Global Declaration Block ==================== */

GDeclBlock  :   DECL GDeclList ENDDECL      {}
            |   DECL ENDDECL                {}
            |                                {}
            ;

GDeclList   :   GDeclList GDecl             {}
            |   GDecl                       {}
            ;

GDecl       :   GType GidList SEMICOLON     {}
            |   GTupleDecl SEMICOLON        {}
            ;

GTupleDecl  :   TUPLE ID '(' FieldDefList ')'   {
                                                    int size = 0;
                                                    struct Fieldlist * temp = $4 ;
                                                    while(temp) {
                                                        size += GetSize(temp->type);
                                                        temp = temp->next ;
                                                    }
                                                    TTableHead = TInstall(TTableHead,$2->varname,size,$4);
                                                    currTupleType = TLookup(TTableHead, $2->varname);

                                                    if(currTupleType == NULL) {
                                                        printf("Tuple Type installation failed\n");
                                                        exit(1);
                                                    }
                                                }
                GTupleIdList                    {   currTupleType = NULL ;}
            ;

GTupleIdList:  GTupleIdList ',' GTupleId       {}
            |  GTupleId                        {}
            ;

GTupleId    :   ID                          {    Ghead = insertTable(Ghead, ($1)->varname, currTupleType, false, currTupleType->size, -1);}
            |   STAR ID                     {    Ghead = insertTable(Ghead, ($2)->varname, currTupleType, true, currTupleType->size, -1);}
            ;

FieldDefList
    : FieldDefList ',' FieldDef
      {
          struct Fieldlist *temp = $1;

          while (temp->next)
              temp = temp->next;

          $3->fieldIndex = temp->fieldIndex + 1;
          temp->next = $3;
          $$ = $1;
      }
    | FieldDefList SEMICOLON FieldDef
      {
          struct Fieldlist *temp = $1;

          while (temp->next)
              temp = temp->next;

          $3->fieldIndex = temp->fieldIndex + 1;
          temp->next = $3;
          $$ = $1;
      }
    | FieldDefList SEMICOLON
      {
          $$ = $1;
      }
    | FieldDef
      {
          $1->fieldIndex = 0;
          $$ = $1;
      }
    ;

FieldDef    :   FieldType ID                {   $$ = createFieldlistNode(($2)->varname,currentType,0); }
            ;

/* FieldType allows user-defined types (looked up in TTableHead) in addition to int/str */
FieldType   :   INT                         {   currentType = T_INT ;}
            |   STR                         {   currentType = T_STR ;}
            |   ID                          {   
                                                struct TypeTable * udt = TLookup(TTableHead, $1->varname);
                                                if(udt == NULL) {
                                                    printf("Error: Undefined type '%s' in field definition at line %d\n", $1->varname, yylineNum);
                                                    exit(1);
                                                }
                                                currentType = udt ;
                                            }
            ;

GidList     :   GidList ',' Gid             {}
            |   Gid                         {}
            ;

Gid         :   ID                          {
                                                Ghead = insertTable(Ghead,($1)->varname,currentType,false,1,-1);
                                                if (currentClassType) find(Ghead,($1)->varname)->Ctype = currentClassType;
                                            }
            |   ID '[' NUM ']'              {
                                                if (currentClassType) stage7Error("Arrays of class objects are not supported");
                                                Ghead = insertTable(Ghead,($1)->varname,currentType,false,$3,-1);
                                            }
            |   ID '(' ParamList ')'        {
                                                if (currentClassType) stage7Error("Functions cannot return class objects in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,($1)->varname,currentType,false,0,flabel++);
                                                insertParamListToGST(Ghead,($1)->varname,$3);
                                            }
            |   STAR ID                     {
                                                if (currentClassType) stage7Error("Pointers to class objects are not supported in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,($2)->varname,currentType == T_INT ? T_INT_PTR : T_STR_PTR,true,1,-1);
                                            }
            ;

FDefBlock : FDefBlock Fdef {}
          |    {}
          ;

Body        :   BEG stmtList Ret END SEMICOLON  {
                                                    $$ = connectNode($2,$3);
                                                }
            |   BEG Ret END SEMICOLON           {
                                                    $$ = $2;
                                                }
            |   BEG stmtList Ret END            {
                                                    $$ = connectNode($2,$3);
                                                }
            |   BEG Ret END                    {
                                                    $$ = $2;
                                                }
            ;

ParamList   :   ParamList ',' Param         {   $$ = insertToParamList($1,($3)->name,($3)->type,($3)->isPtr,($3)->Ctype) ; }
            |   Param                       {   $$ = insertToParamList(NULL,($1)->name,($1)->type,($1)->isPtr,($1)->Ctype) ;}
            |                               {   $$ = NULL ;}
            ;

Param       :   ParType ID                  {   $$ = createParamStructNode(strdup($2->varname),paramType,false,paramClassType); }
            |   ParType STAR ID             {   $$ = createParamStructNode(strdup($3->varname),paramType == T_INT ? T_INT_PTR : T_STR_PTR,true,NULL); }
            ;

Ret         :   RETURN EXPR SEMICOLON       {   
                                                $$ = returnNode(currentFunc,RETURN_NODE,$2); 
                                            }

LdeclBlock  :   DECL LDeclList ENDDECL      {   localBinding = 1; }
            |   DECL ENDDECL                {}
            ;

LDeclList   :   LDeclList LDecl             {}
            |   LDecl                       {}
            ;

LDecl       :   LType IdList SEMICOLON      {}
            |   LTupleDecl SEMICOLON        {}  
            ;

LTupleDecl  :   TUPLE ID '(' FieldDefList ')'   {
                                                    int size = 0;
                                                    struct Fieldlist *temp = $4;
                                                    while(temp) {
                                                        size += GetSize(temp->type);
                                                        temp = temp->next;
                                                    }
                                                    TTableHead = TInstall(TTableHead, $2->varname, size, $4);
                                                    currTupleType = TLookup(TTableHead, $2->varname);
                                                    if (currTupleType == NULL) {
                                                        yyerror("Tuple type installation failed");
                                                        exit(1);
                                                    }
                                                }
                LTupleIdList                    {   currTupleType = NULL ;}
            ;
    
LTupleIdList:   LTupleIdList ',' LTupleId       {}
            |   LTupleId                        {}
            ;

LTupleId    :   ID                          {   Lhead = insertToLocalTable(Lhead, ($1)->varname, currTupleType,false,currTupleType->size);}
            |   STAR ID                     {   Lhead = insertToLocalTable(Lhead, ($2)->varname, currTupleType,true,1);}
            ;


IdList      :   IdList ',' ID               {   Lhead = insertToLocalTable(Lhead,($3)->varname,currLocalType,false,currLocalClassType ? 1 : 1); if(currLocalClassType) findLocalSymbol(Lhead,($3)->varname)->Ctype=currLocalClassType;}
            |   ID                          {   Lhead = insertToLocalTable(Lhead,($1)->varname,currLocalType,false,1); if(currLocalClassType) findLocalSymbol(Lhead,($1)->varname)->Ctype=currLocalClassType;}
            ;

GType       :   INT                         { currentType = T_INT; currentClassType = NULL; }
            |   STR                         { currentType = T_STR; currentClassType = NULL; }
            |   ID                          {
                                                struct TypeTable *udt = TLookup(TTableHead, $1->varname);
                                                struct Classtable *ct = CLookup(CTableHead, $1->varname);
                                                currentType = udt;
                                                currentClassType = NULL;
                                                if (!udt && ct) currentClassType = ct;
                                                else if (!udt && !ct) {
                                                    fprintf(stderr, "Error: Undefined type '%s' at line %d\n", $1->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
            ;

ParType     :   INT                         { paramType = T_INT; paramClassType = NULL; }
            |   STR                         { paramType = T_STR; paramClassType = NULL; }
            |   ID                          {
                                                paramType = TLookup(TTableHead, $1->varname);
                                                paramClassType = NULL;
                                                if (!paramType) paramClassType = CLookup(CTableHead, $1->varname);
                                                if (!paramType && !paramClassType) {
                                                    fprintf(stderr, "Error: Undefined parameter type '%s' at line %d\n", $1->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
            ;

LType       :   INT                         { currLocalType = T_INT; currLocalClassType = NULL; }
            |   STR                         { currLocalType = T_STR; currLocalClassType = NULL; }
            |   ID                          {
                                                currLocalType = TLookup(TTableHead, $1->varname);
                                                currLocalClassType = NULL;
                                                if (!currLocalType) currLocalClassType = CLookup(CTableHead, $1->varname);
                                                if (!currLocalType && !currLocalClassType) {
                                                    fprintf(stderr, "Error: Undefined local type '%s' at line %d\n", $1->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
            ;


EXPR        :   EXPR MINUS EXPR             {   $$ = exprNode(MINUS_NODE, $1, $3); }
            |   EXPR PLUS  EXPR             {   $$ = exprNode(ADD_NODE, $1, $3); }
            |   EXPR STAR  EXPR             {   $$ = exprNode(MUL_NODE, $1, $3); }
            |   EXPR DIV   EXPR             {   $$ = exprNode(DIV_NODE, $1, $3); }
            |   EXPR AND   EXPR             {   $$ = exprNode(AND_NODE,$1, $3); }
            |   EXPR OR    EXPR             {   $$ = exprNode(OR_NODE,$1, $3); }
            |   NOT EXPR                    {   $$ = exprNode(NOT_NODE,$2, NULL);}
            |   EXPR LE    EXPR             {   $$ = exprNode(LE_NODE, $1, $3); }
            |   EXPR LT    EXPR             {   $$ = exprNode(LT_NODE, $1, $3); }
            |   EXPR GT    EXPR             {   $$ = exprNode(GT_NODE, $1, $3); }
            |   EXPR GE    EXPR             {   $$ = exprNode(GE_NODE, $1, $3); }
            |   EXPR EQ    EXPR             {   $$ = exprNode(EQ_NODE, $1, $3); }
            |   EXPR NE    EXPR             {   $$ = exprNode(NE_NODE, $1, $3); }
            |   '(' EXPR ')'                {   $$ = $2 ;}
            |   NUM                         {   $$ = createTreeNode(0,$<number>1,T_INT,NULL,NUM_NODE,NULL,NULL,NULL,NULL); }
            |   STR_LITERAL                 {   $$ = createTreeNode(0,NO_VAL,T_STR,$<string>1,STR_NODE,NULL,NULL,NULL,NULL); }
            |   IDENTIFIERS                 {   $$ = $1 ; }
            |   NULL_VAL                    {   $$ = createTreeNode(0,0,T_VOID,"null",NULL_NODE,NULL,NULL,NULL,NULL); }
            |   ALLOC '(' ')'              {   $$ = createTreeNode(0,NO_VAL,T_VOID,NULL,ALLOC_NODE,NULL,NULL,NULL,NULL); }
            |   NEW '(' ID ')'              {
                                                struct Classtable *ct = CLookup(CTableHead, $3->varname);
                                                if (!ct) { fprintf(stderr, "Error: Unknown class '%s' in new at line %d\n", $3->varname, yylineNum); exit(1); }
                                                $$ = createTreeNode(0,NO_VAL,T_VOID,strdup($3->varname),ALLOC_NODE,NULL,NULL,NULL,NULL);
                                                $$->Ctype = ct;
                                            }
            |   INITIALIZE '(' ')'         {   $$ = createTreeNode(0,NO_VAL,T_INT,NULL,INIT_NODE,NULL,NULL,NULL,NULL); }
            |   FREE '(' IDENTIFIERS ')'   {   $$ = createTreeNode(0,NO_VAL,T_INT,NULL,FREE_NODE,NULL,$3,NULL,NULL); }
            |   ID '(' ')'                 {   
                                                $$ = funcNode(Ghead,$1,NULL);
                                            }
            |   ID '(' ArgList ')'          {   $$ = funcNode(Ghead,$1,$3);}
            |   SELF DOT ID '(' ')'        {
                                                struct MemberFunc *m = Class_Mlookup(currentClass, $3->varname);
                                                $$ = makeMethodCall(m, NULL, makeSelfReceiver());
                                            }
            |   SELF DOT ID '(' ArgList ')' {
                                                struct MemberFunc *m = Class_Mlookup(currentClass, $3->varname);
                                                $$ = makeMethodCall(m, $5, makeSelfReceiver());
                                            }
            |   IDENTIFIERS DOT ID '(' ')'  {
                                                /* Receiver may be global, local, or a class-valued parameter. */
                                                struct Classtable *receiverClass = $1->Ctype;
                                                if (!receiverClass && $1->Lentry) receiverClass = $1->Lentry->Ctype;
                                                if (!receiverClass && $1->Gentry) receiverClass = $1->Gentry->Ctype;
                                                if (!receiverClass) {
                                                    struct Gsymbol *obj = find(Ghead, $1->varname);
                                                    if (obj) { receiverClass = obj->Ctype; $1->Gentry = obj; }
                                                }
                                                if (!receiverClass) stage7Error("Method call receiver is not a declared class object");
                                                $1->Ctype = receiverClass;
                                                struct MemberFunc *m = Class_Mlookup(receiverClass, $3->varname);
                                                $$ = makeMethodCall(m, NULL, $1);
                                            }
            |   IDENTIFIERS DOT ID '(' ArgList ')' {
                                                /* Receiver may be global, local, or a class-valued parameter. */
                                                struct Classtable *receiverClass = $1->Ctype;
                                                if (!receiverClass && $1->Lentry) receiverClass = $1->Lentry->Ctype;
                                                if (!receiverClass && $1->Gentry) receiverClass = $1->Gentry->Ctype;
                                                if (!receiverClass) {
                                                    struct Gsymbol *obj = find(Ghead, $1->varname);
                                                    if (obj) { receiverClass = obj->Ctype; $1->Gentry = obj; }
                                                }
                                                if (!receiverClass) stage7Error("Method call receiver is not a declared class object");
                                                $1->Ctype = receiverClass;
                                                struct MemberFunc *m = Class_Mlookup(receiverClass, $3->varname);
                                                $$ = makeMethodCall(m, $5, $1);
                                            }
            |   SELF DOT ID DOT ID '(' ')' {
                                                struct ClassField *f = Class_Flookup(currentClass, $3->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = Class_Mlookup(f->ctype, $5->varname);
                                                $$ = makeMethodCall(m, NULL, makeSelfFieldReceiver(f));
                                            }
            |   SELF DOT ID DOT ID '(' ArgList ')' {
                                                struct ClassField *f = Class_Flookup(currentClass, $3->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = Class_Mlookup(f->ctype, $5->varname);
                                                $$ = makeMethodCall(m, $7, makeSelfFieldReceiver(f));
                                            }
            ;

ArgList     :   ArgList ',' EXPR            {
                                                $$ = appendArg($1,$3);
                                            }
            |   EXPR                        {   
                                                $$ = $1;
                                                $$->argList = NULL ;
                                            }
            ;

stmtList    :   stmtList stmt                   {   $$ = connectNode($1,$2); }
            |   stmt                            {   $$ = $1 ; }
            ;

stmt        :   ipStmt                          {   $$ = $1 ; }
            |   opStmt                          {   $$ = $1 ; }
            |   assignStmt                      {   $$ = $1 ; }
            |   ifStmt                          {   $$ = $1 ; }
            |   whileStmt                       {   $$ = $1 ; }
            |   doWhileStmt                     {   $$ = $1 ; }
            |   repeatStmt                      {   $$ = $1 ; }
            |   BREAK SEMICOLON                 {   $$ = flowControlNode(BREAK_NODE); }
            |   CONTINUE SEMICOLON              {   $$ = flowControlNode(CONTINUE_NODE); }
            |   EXIT_PR SEMICOLON               {   $$ = exitNode(EXIT_NODE); } 
            ;                   

Fdef        :   GType ID '(' ParamList ')' '{'  {
                                                    if (currentClassType) stage7Error("Functions cannot return class objects in Stage 7 Task 1");
                                                    currentFunc = find(Ghead, $2->varname);
                                                    checkDeclAndDefn(currentFunc,currentType);
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", $2->varname);
                                                        exit(1);
                                                    }
                                                    compareParamList(currentFunc,$4) ;
                                                    Lhead = createLSTFromParams($4);
                                                }
                LdeclBlock Body '}'             {
                                                    currentFunc->bodyAST = $9 ;
                                                    currentFunc->Lentry = Lhead ;
                                                    
                                                    funcCodeGen(currentFunc,output);                                       
                                                    currentFunc = NULL ;
                                                    Lhead = NULL ;
                                                }
            ;

ifStmt      :   IF '(' EXPR ')' THEN stmtList ELSE stmtList ENDIF SEMICOLON     {   
                                                                                    
                                                                                    $$ = ifelseNode(IF_NODE,$3,$6,$8);
                                                                                }
            |   IF '(' EXPR ')' THEN stmtList ENDIF SEMICOLON                   {
                                                                                    
                                                                                    $$ = ifelseNode(IF_NODE,$3,$6,NULL);
                                                                                }
            ;

whileStmt   :    WHILE '(' EXPR ')' DO stmtList ENDWHILE SEMICOLON              {
                                                                                    $$ = loopNode(WHILE_NODE,$3,$6);
                                                                                }
            ;

doWhileStmt :   DO stmtList WHILE '(' EXPR ')' SEMICOLON                        {   $$ = loopNode(DOWHILE_NODE,$5,$2); }
            ;

repeatStmt  :   REPEAT stmtList UNTIL '(' EXPR ')' SEMICOLON                    {   $$ = loopNode(REPEAT_NODE,$5,$2); }
            ;

ipStmt      :   READ '(' IDENTIFIERS ')' SEMICOLON                              {   $$ = ipOpNode(READ_NODE,$3); }
            ;

opStmt      :   WRITE '(' EXPR ')' SEMICOLON                                    {   $$ = ipOpNode(WRITE_NODE,$3); }
            ;

assignStmt  :   IDENTIFIERS ASSIGN EXPR SEMICOLON                               {   $$ = assignNode($1,$3); }
            ;

IDENTIFIERS :   SELF DOT ID                                                   {
                                                                                    if (!currentClass) stage7Error("self can only be used inside a class method");
                                                                                    struct ClassField *f = Class_Flookup(currentClass, $3->varname);
                                                                                    if (!f) { fprintf(stderr, "Error: Field '%s' not found in class '%s' at line %d\n", $3->varname, currentClass->name, yylineNum); exit(1); }
                                                                                    $$ = createTreeNode(0, f->fieldIndex, f->type, strdup(f->name), SELF_FIELD_NODE, NULL, NULL, NULL, NULL);
                                                                                    $$->Ctype = f->ctype;
                                                                                    $$->Lentry = findLocalSymbol(Lhead, "self");
                                                                                    if (!$$->Lentry) stage7Error("Internal error: self is missing from method local table");
                                                                                }
            |   ID %prec FIELD_BASE                                               {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$1,NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    $$ = node;
                                                                                }
            |   ID '['  EXPR ']'                                                {   
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$1,$3,NULL);
                                                                                    node->nodetype = ARR_NODE ;
                                                                                    $$ = node;
                                                                                }
            |   STAR ID                                                         {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$2,NULL,NULL);
                                                                                    node->nodetype = ID_NODE;                              
                                                                                    $$ = pointerNode(Ghead,DEREF_NODE,node,NULL,NULL);
                                                                                }      
            |   AMPERSAND ID                                                    {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,$2,NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    $$ = pointerNode(Ghead,ADDR_NODE,node,NULL,NULL);
                                                                                }
            |   IDENTIFIERS DOT ID                                              {
                                                                                    /* Allow nested UDT field access such as self.tail.next. */
                                                                                    if ($1->Gentry && $1->Gentry->Ctype) stage7Error("Class fields are private; access fields only through self inside their class");
                                                                                    if (!$1->type || $1->type->fields == NULL) {
                                                                                        const char *baseType = $1->Ctype ? $1->Ctype->name : ($1->type ? $1->type->name : "<class>");
                                                                                        fprintf(stderr, "Error: Field access ('.') on a non-structured type '%s' at line %d\n", baseType, yylineNum);
                                                                                        exit(1);
                                                                                    }
                                                                                    struct Fieldlist *field = FLookup($1->type, $3->varname);

                                                                                    if (field == NULL) {
                                                                                        printf("Error: Field '%s' not found in type '%s' at line %d\n", $3->varname, $1->type->name, yylineNum);                                                                                   
                                                                                        exit(1);
                                                                                    }
                                                                                    $$ = createTreeNode(0,field->fieldIndex, field->type, field->name, FIELD_NODE, NULL, $1, NULL, NULL);
                                                                                }                                                              
            ;

%%

int yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineNum);
    return 1;
}

extern FILE* yyin;

int main(int argc, char** argv) {

    if(argc > 1){
        yyin = fopen(argv[1], "r");
        if(!yyin){
            printf("Error opening file\n");
            return 1;
        }
    }

    output = fopen("output.xsm", "w");

    TTableHead = createTypeTable(); // TInstall default types -> int, str, void, bool

    auxFunctions(output,ENTRY_CODE,-1,-1);
    fprintf(output,"CALL F0\n");
    yyparse();
    auxFunctions(output,EXIT_CODE,-1,-1);
    printSymbolTable(Ghead);
    if (CTableHead) Class_PrintTable(CTableHead);

    fclose(output);
    return 0;
}
