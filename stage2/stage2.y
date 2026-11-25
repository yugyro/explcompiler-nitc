%{
    #include <stdio.h>
    #include "tree.h"
    #include "codegen.h"
    #include <stdlib.h>

    int yylex();
    void yyerror(char *s);
    void print(int);
    tnode * root;
    FILE * intermediate;
    extern FILE * yyin;
%}


%token NUM ID BEGIN_T END_T READ WRITE 
%left '+'
%left '*'
%left '/'
%left '-'

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
            ;
inputstmt:  READ '(' ID ')'  ';'         {$$=createTree(0,READ_T,NULL,$3,NULL);}
            ;
outputstmt: WRITE '(' ID ')' ';'         {$$=createTree(0,WRITE_T,NULL,$3,NULL);}
            ;
assignstmt: ID '=' E ';'                 {$$=createTree(0,ASGN_T,NULL,$1,$3);}
            ;     
E : E '+' E    { $$ = createTree(0, PLUS_T, NULL, $1, $3); }
  | E '-' E    { $$ = createTree(0, MINUS_T, NULL, $1, $3); }
  | E '*' E    { $$ = createTree(0, MUL_T, NULL, $1, $3); }
  | E '/' E    { $$ = createTree(0, DIV_T, NULL, $1, $3); }
  | '(' E ')'  { $$ = $2; }
  | NUM        { $$ = $1; }
  | ID         { $$ = $1; }
  ;
%%

void printTree(struct tnode *t, int level)
{
    if (t == NULL)
        return;
    for (int i = 0; i < level; i++)
        if(i == level - 1)
            printf("|-- ");
        else
            printf("|   ");
    switch (t->nodetype)
    {
        case CONNECTOR_T:
            printf("CONNECTOR\n");
            break;

        case PLUS_T:
            printf("+\n");
            break;

        case MINUS_T:
            printf("-\n");
            break;

        case MUL_T:
            printf("*\n");
            break;

        case DIV_T:
            printf("/\n");
            break;

        case ASGN_T:
            printf("=\n");
            break;

        case READ_T:
            printf("READ\n");
            break;

        case WRITE_T:
            printf("WRITE\n");
            break;

        case ID_T:
            printf("ID: %s\n", t->varname);
            break;

        case NUM_T:
            printf("NUM: %d\n", t->val);
            break;
    }
    printTree(t->left, level + 1);
    printTree(t->right, level + 1);
}
int main(int argc, char *argv[]) {
    FILE *target_file = fopen("targetcode.xsm", "w");
    fprintf(target_file, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n", 0,2056,0,0,0,0,0,0);
    fprintf(target_file,"ADD SP,26\n");
    fprintf(target_file, "BRKP\n");
    initialize();

    if (argc > 1) {
        yyin = fopen(argv[1], "r");
        if (!yyin) { perror("fopen failed"); return 1; }
    }

    if (yyparse() == 0) {
        codegen(root, target_file);
        //evaluate(root);
        printf("AST\n");
        printTree(root,0);
        /* Exit sequence */
        fprintf(target_file, "MOV R2, \"Exit\"\n");
        fprintf(target_file, "PUSH R2\nPUSH R2\nPUSH R2\nPUSH R2\nPUSH R2\n");
        fprintf(target_file, "CALL 0\n");
    }
    fclose(target_file);
    return 0;
}

void yyerror(char * s) {printf("%s\n",s);}

/* 
yacc -d stage2.y
lex stage2.l
gcc y.tab.c lex.yy.c tree.c codegen.c -o compiler
bash ./xsm -l library.lib -e workdir/stage2/targetcode.xsm  
*/
