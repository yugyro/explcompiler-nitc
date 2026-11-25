%{
    #include <stdio.h>
    #include "tree.h"
    #include "codegen.h"
    #include <stdlib.h>
    #include "symboltable.h"

    int yylex();
    void yyerror(char *s);
    tnode *root;
    extern FILE *yyin;
    int currentDecltype;
%}

%token NUM ID STRCONST READ WRITE DECL ENDDECL
%token IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token LE GE EQOP NE
%token INT STR

%left '<' '>' LE GE NE EQOP
%left '+' '-'
%left '*' '/'
%left '[' ']'

%%
Program:    Declarations slist                   {root=$2;}
            ;

Declarations: DECL decllist ENDDECL 
            | DECL ENDDECL
            ;
            
decllist:   decllist decla
            | decla
            ;

decla:       Type Varlist ';'
            ;

Type:       INT                         {currentDecltype=TYPE_INT_VAR;}
            | STR                       {currentDecltype=TYPE_STR_VAR;}
            ;

Varlist:    Varlist ',' Varitem                 
            | Varitem
            ;

Varitem:    ID '[' NUM ']'                  {Install($1->varname,currentDecltype,$3->val,1);}
          | ID '[' NUM ']' '[' NUM ']'      {Install($1->varname,currentDecltype,$3->val * $6->val,$6->val);}
          | ID                              {Install($1->varname,currentDecltype,1,1);}
          ;


slist:      slist stmt                  {$$=createTree(0,CONNECTOR_T,NULL,$1,$2);}
            | stmt                      {$$=$1;}
            ;

stmt:       inputstmt
            | outputstmt
            | assignstmt
            | ifstmt
            | whilestmt
            | BREAK ';'                 {$$=createTree(0,BREAK_T,NULL,NULL,NULL);}
            | CONTINUE ';'              {$$=createTree(0,CONTINUE_T,NULL,NULL,NULL);}
            ;

inputstmt:  READ '(' Lvalue ')' ';'              {$$=createTree(0,READ_T,NULL,$3,NULL);}
            ;

outputstmt: WRITE '(' E ')' ';'                  {$$=createTree(0,WRITE_T,NULL,$3,NULL);}
            ;

assignstmt: Lvalue '=' E ';'                     {$$=createAssign($1,$3);}
            ;

Lvalue:     ID                       {$$=resolveScalar($1);}
          | ID '[' E ']'             {$$=createArrAccess($1,$3,NULL);}
          | ID '[' E ']' '[' E ']'   {$$=createArrAccess($1,$3,$6);}
          ;

ifstmt:     IF '(' E ')' THEN slist ELSE slist ENDIF ';'  {$$=createIf($3,$6,$8);}
            | IF '(' E ')' THEN slist ENDIF ';'           {$$=createIf($3,$6,NULL);}
            ;

whilestmt:  WHILE '(' E ')' DO slist ENDWHILE ';'  {$$=createTree(0,WHILE_T,NULL,$3,$6);}
            ;

E : E '+' E   { $$ = createTree(0, PLUS_T, NULL, $1, $3); }
  | E '-' E   { $$ = createTree(0, MINUS_T, NULL, $1, $3); }
  | E '*' E   { $$ = createTree(0, MUL_T, NULL, $1, $3); }
  | E '/' E   { $$ = createTree(0, DIV_T, NULL, $1, $3); }
  | E '<' E   { $$ = createTree(0, LT_T, NULL, $1, $3); }
  | E '>' E   { $$ = createTree(0, GT_T, NULL, $1, $3); }
  | E LE E    { $$ = createTree(0, LE_T, NULL, $1, $3); }
  | E GE E    { $$ = createTree(0, GE_T, NULL, $1, $3); }
  | E NE E    { $$ = createTree(0, NE_T, NULL, $1, $3); }
  | E EQOP E  { $$ = createTree(0, EQREL_T, NULL, $1, $3); }
  | '(' E ')' { $$ = $2; }
  | NUM       { $$ = $1; }
  | STRCONST       { $$ = $1; }
  | Lvalue    {$$=$1;}
  ;
%%

extern int yyerrline;
void yyerror(char *s) { printf("Line %d,%s\n",yyerrline,s); }

int main(int argc, char *argv[]) {
    FILE *intermediate = fopen("intermediate.xsm", "w");
    fprintf(intermediate, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 0,2056,0,0,0,0,0,0);
    fprintf(intermediate, "BRKP\n");
    initialize();

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { perror("fopen failed"); return 1; }
    }

    if (yyparse() == 0) {
        fprintf(intermediate, "ADD SP,%d\n",totalDeclaredWords());
        printSymtab();
        codegen(root, intermediate);
        fprintf(intermediate, "MOV R2, \"Exit\"\n");
        fprintf(intermediate, "PUSH R2\nPUSH R2\nPUSH R2\nPUSH R2\nPUSH R2\n");
        fprintf(intermediate, "CALL 0\n");
        printf("Wrote intermediate.xsm\n");
    }
    fclose(intermediate);
    return 0;
}