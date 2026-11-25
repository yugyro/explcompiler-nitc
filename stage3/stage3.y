%{
    #include <stdio.h>
    #include "tree.h"
    #include "codegen.h"
    #include <stdlib.h>

    int yylex();
    void yyerror(char *s);
    tnode *root;
    extern FILE *yyin;
%}

%token NUM ID BEGIN_T END_T READ WRITE
%token IF THEN ELSE ENDIF WHILE DO ENDWHILE BREAK CONTINUE
%token LE GE EQOP NE

%left '<' '>' LE GE NE EQOP
%left '+' '-'
%left '*' '/'

%%
Program:    BEGIN_T slist END_T ';'     {root=$2;}
            | BEGIN_T END_T ';'         {root=NULL;}
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

inputstmt:  READ '(' ID ')' ';'                  {$$=createTree(0,READ_T,NULL,$3,NULL);}
            ;

outputstmt: WRITE '(' E ')' ';'                  {$$=createTree(0,WRITE_T,NULL,$3,NULL);}
            ;

assignstmt: ID '=' E ';'                         {$$=createTree(0,ASGN_T,NULL,$1,$3);}
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
  | ID        { $$ = $1; }
  ;
%%

void yyerror(char *s) { printf("%s\n", s); }

int main(int argc, char *argv[]) {
    FILE *intermediate = fopen("intermediate.xsm", "w");
    fprintf(intermediate, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 0,2056,0,0,0,0,0,0);
    fprintf(intermediate, "ADD SP,26\n");
    fprintf(intermediate, "BRKP\n");
    initialize();

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { perror("fopen failed"); return 1; }
    }

    if (yyparse() == 0) {
        codegen(root, intermediate);
        fprintf(intermediate, "MOV R2, \"Exit\"\n");
        fprintf(intermediate, "PUSH R2\nPUSH R2\nPUSH R2\nPUSH R2\nPUSH R2\n");
        fprintf(intermediate, "CALL 0\n");
        printf("Wrote intermediate.xsm\n");
    }
    fclose(intermediate);
    return 0;
}