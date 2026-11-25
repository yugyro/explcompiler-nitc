%{
#include <stdio.h>
#include <stdlib.h>
#include "tac.h"
int  yylex(void);
void yyerror(const char *s);
static int curtype;
extern FILE *yyin;
static Quad opt_code[MAXQ * 3];
%}

%union { long num; char *str; struct Ast *ast; }

%token <num> NUM
%token <str> ID STRING
%token K_DECL K_ENDDECL K_BEGIN K_END K_IF K_THEN K_ELSE K_ENDIF K_WHILE K_DO K_ENDWHILE
%token K_READ K_WRITE K_RETURN K_BREAK K_CONTINUE K_BREAKPOINT K_INT K_STR K_MAIN K_UNSUP
%token LT GT LE GE EQ NE

%left  OR
%left  AND
%right NOT
%left  LT GT LE GE EQ NE
%left  '+' '-'
%left  '*' '/' '%'
%right UMINUS

%type <ast> expr lvalue stmt stmt_list ret_stmt

%%
program   : opt_decl main_def ;

opt_decl  : /* empty */
          | K_DECL decl_list K_ENDDECL ;

decl_list : decl_list decl | decl ;
decl      : type_spec var_list ';' ;
type_spec : K_INT { curtype = T_INT; } | K_STR { curtype = T_STR; } ;
var_list  : var_list ',' var | var ;
var       : ID                { sym_declare($1, curtype, 0, 0, !in_local); }
          | ID '[' NUM ']'    { if (in_local) semerr("arrays can only be declared globally");
                                else sym_declare($1, curtype, 1, (int)$3, 1); }
          ;

main_def  : K_INT K_MAIN '(' ')' '{' { in_local = 1; } opt_decl
            K_BEGIN stmt_list ret_stmt K_END '}'
            { program = ast_append($9, $10); }
          ;

stmt_list : /* empty */        { $$ = NULL; }
          | stmt_list stmt     { $$ = ast_append($1, $2); }
          ;

stmt : lvalue '=' expr ';'                                   { $$ = mk_assign($1, $3); }
     | K_IF expr K_THEN stmt_list K_ENDIF ';'                { $$ = mk_if($2, $4, NULL); }
     | K_IF expr K_THEN stmt_list K_ELSE stmt_list K_ENDIF ';' { $$ = mk_if($2, $4, $6); }
     | K_WHILE expr K_DO stmt_list K_ENDWHILE ';'            { $$ = mk_while($2, $4); }
     | K_READ '(' lvalue ')' ';'                             { $$ = mk_read($3); }
     | K_WRITE '(' expr ')' ';'                              { $$ = mk_write($3); }
     | K_BREAK ';'                                           { $$ = mk_simple(S_BREAK); }
     | K_CONTINUE ';'                                        { $$ = mk_simple(S_CONT); }
     | K_BREAKPOINT ';'                                      { $$ = mk_simple(S_BKPT); }
     ;

ret_stmt : K_RETURN expr ';'   { $$ = mk_ret($2); } ;

lvalue : ID                    { $$ = mk_var($1); }
       | ID '[' expr ']'       { $$ = mk_arr($1, $3); }
       ;

expr : NUM                     { $$ = mk_int($1); }
     | STRING                  { $$ = mk_str($1); }
     | ID                      { $$ = mk_var($1); }
     | ID '[' expr ']'         { $$ = mk_arr($1, $3); }
     | expr '+' expr           { $$ = mk_bin('+', $1, $3); }
     | expr '-' expr           { $$ = mk_bin('-', $1, $3); }
     | expr '*' expr           { $$ = mk_bin('*', $1, $3); }
     | expr '/' expr           { $$ = mk_bin('/', $1, $3); }
     | expr '%' expr           { $$ = mk_bin('%', $1, $3); }
     | '-' expr %prec UMINUS   { $$ = mk_neg($2); }
     | '(' expr ')'            { $$ = $2; }
     | expr LT expr            { $$ = mk_rel(R_LT, $1, $3); }
     | expr GT expr            { $$ = mk_rel(R_GT, $1, $3); }
     | expr LE expr            { $$ = mk_rel(R_LE, $1, $3); }
     | expr GE expr            { $$ = mk_rel(R_GE, $1, $3); }
     | expr EQ expr            { $$ = mk_rel(R_EQ, $1, $3); }
     | expr NE expr            { $$ = mk_rel(R_NE, $1, $3); }
     | expr AND expr           { $$ = mk_logic(E_AND, $1, $3); }
     | expr OR expr            { $$ = mk_logic(E_OR, $1, $3); }
     | NOT expr                { $$ = mk_not($2); }
     ;
%%
void yyerror(const char *s) {
    fprintf(stderr, "line %d: %s\n", lineno, s);
    nerrors++;
}

/* ---------------- driver ----------------
 * usage: ./expl2tac source.txt
 * output.txt            generated TAC
 * optimized_output.txt  optimized TAC
 * stats.txt             optimization log + counts                    */


int main(int argc, char **argv) {
    yyin = fopen(argv[1], "r");
    yyparse();
    gen_program(program);

    FILE *f = fopen("output.txt", "w");
    print_code(f, code, ncode);
    fclose(f);

    FILE *st = fopen("stats.txt", "w");
    fprintf(st, "Optimization log\n----------------\n");
    int nopt = optimize(code, ncode, opt_code, st);

    f = fopen("optimized_output.txt", "w");
    print_code(f, opt_code, nopt);
    fclose(f);

    fprintf(st, "\nNumber of constant foldings : %d\n", opt_nfold);
    fprintf(st, "Number of CSE eliminations  : %d\n", opt_ncse);
    fclose(st);
    return 0;
}