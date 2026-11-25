
%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tac.h"

int  yylex(void);
void yyerror(const char *s);
extern int   yylineno;
extern FILE *yyin;

int errors = 0;

char symtab[100][32];
int  nsym = 0;

void declare(char *name)
{
    int i;
    for (i = 0; i < nsym; i++)
        if (!strcmp(symtab[i], name)) {
            printf("Error line %d: '%s' declared twice\n", yylineno, name);
            errors++;
            return;
        }
    strcpy(symtab[nsym++], name);
}

void check(char *name)
{
    int i;
    for (i = 0; i < nsym; i++)
        if (!strcmp(symtab[i], name)) return;
    printf("Error line %d: '%s' not declared\n", yylineno, name);
    errors++;
}

char *binop(const char *op, char *a, char *b)
{
    char *t = newtemp();
    emit(op, a, b, t);
    return t;
}

void emit_label(int n)
{
    char buf[32];
    sprintf(buf, "L%d", n);
    emit("label", "", "", buf);
}
void emit_goto(int n)
{
    char buf[32];
    sprintf(buf, "L%d", n);
    emit("goto", "", "", buf);
}
void emit_iffalse(char *cond, int n)
{
    char buf[32];
    sprintf(buf, "L%d", n);
    emit("ifFalse", cond, "", buf);
}
%}

%union { char *s; int n; }

%token <s> ID NUM
%token DECL ENDDECL T_BEGIN T_END INT READ WRITE
%token IF THEN ELSE ENDIF WHILE DO ENDWHILE
%token LE GE EQ NE AND OR

%type <s> expr
%type <n> if_head

%left OR
%left AND
%left EQ NE
%left '<' '>' LE GE
%left '+' '-'
%left '*' '/' '%'
%right UMINUS

%%

program : DECL decl_list ENDDECL T_BEGIN stmt_list T_END
        ;

decl_list : decl_list decl
          | /* empty */
          ;

decl : INT id_list ';'
     ;

id_list : id_list ',' ID   { declare($3); }
        | ID               { declare($1); }
        ;

stmt_list : stmt_list stmt
          | stmt
          ;

stmt : ID '=' expr ';'
         { check($1); emit("=", $3, "", $1); }
     | READ '(' ID ')' ';'
         { check($3); emit("read", "", "", $3); }
     | WRITE '(' expr ')' ';'
         { emit("write", $3, "", ""); }
     | if_head stmt_list ENDIF ';'
         { emit_label($1); }
     | if_head stmt_list ELSE
         { int l2 = newlabel(); emit_goto(l2); emit_label($1); $<n>$ = l2; }
       stmt_list ENDIF ';'
         { emit_label($<n>4); }
     | WHILE
         { $<n>$ = newlabel(); emit_label($<n>$); }
       '(' expr ')' DO
         { $<n>$ = newlabel(); emit_iffalse($4, $<n>$); }
       stmt_list ENDWHILE ';'
         { emit_goto($<n>2); emit_label($<n>7); }
     ;

if_head : IF '(' expr ')' THEN
            { $$ = newlabel(); emit_iffalse($3, $$); }
        ;

expr : expr '+' expr   { $$ = binop("+",  $1, $3); }
     | expr '-' expr   { $$ = binop("-",  $1, $3); }
     | expr '*' expr   { $$ = binop("*",  $1, $3); }
     | expr '/' expr   { $$ = binop("/",  $1, $3); }
     | expr '%' expr   { $$ = binop("%",  $1, $3); }
     | expr '<' expr   { $$ = binop("<",  $1, $3); }
     | expr '>' expr   { $$ = binop(">",  $1, $3); }
     | expr LE expr    { $$ = binop("<=", $1, $3); }
     | expr GE expr    { $$ = binop(">=", $1, $3); }
     | expr EQ expr    { $$ = binop("==", $1, $3); }
     | expr NE expr    { $$ = binop("!=", $1, $3); }
     | expr AND expr   { $$ = binop("&&", $1, $3); }
     | expr OR expr    { $$ = binop("||", $1, $3); }
     | '-' expr %prec UMINUS
         { char *t = newtemp(); emit("uminus", $2, "", t); $$ = t; }
     | '(' expr ')'    { $$ = $2; }
     | NUM             { $$ = $1; }
     | ID              { check($1); $$ = $1; }
     ;

%%

void yyerror(const char *s)
{
    printf("Syntax error at line %d: %s\n", yylineno, s);
    errors++;
}

int main(int argc, char *argv[])
{
    static Quad opt[MAXCODE];
    int nopt, folds, cses;
    FILE *fp;

    if (argc < 2) {
        printf("Usage: %s <input file>\n", argv[0]);
        return 1;
    }
    yyin = fopen(argv[1], "r");
    if (!yyin) {
        printf("Cannot open %s\n", argv[1]);
        return 1;
    }

    yyparse();
    if (errors > 0) {
        printf("%d error(s). No code generated.\n", errors);
        return 1;
    }

    /* unoptimized TAC */
    fp = fopen("tac_output.txt", "w");
    print_code(code, ncode, fp);
    fclose(fp);

    /* optimized TAC */
    optimize(code, ncode, opt, &nopt, &folds, &cses);
    fp = fopen("optimized_tac.txt", "w");
    print_code(opt, nopt, fp);
    fclose(fp);

    /* statistics */
    fp = fopen("stats.txt", "w");
    fprintf(fp, "Number of constant foldings : %d\n", folds);
    fprintf(fp, "Number of CSE eliminations  : %d\n", cses);
    fprintf(fp, "Instructions before optimization : %d\n", ncode);
    fprintf(fp, "Instructions after optimization  : %d\n", nopt);
    fclose(fp);

    printf("Done. Generated tac_output.txt, optimized_tac.txt and stats.txt\n");
    return 0;
}
