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
%}


%token NUM
%left '+'
%left '*'
%left '/'
%left '-'

%%
S:E {root=$1;
    int res=codegen(root,intermediate);
    printf("Result is in reg num %d\n",res);
    print(res);
    }
;
E:  E '+' E {$$=makeOperatorNode("+",$1,$3);}
    | E '*' E {$$=makeOperatorNode("*",$1,$3);}
    | E '/' E {$$=makeOperatorNode("/",$1,$3);}
    | E '-' E {$$=makeOperatorNode("-",$1,$3);}
    | NUM {$$=$1;}
    | '(' E ')' {$$=$2;}
    ;
%%

void print(int r) {
    fprintf(intermediate, "MOV R2, \"Write\"\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "MOV R2, -2\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "PUSH R%d\n", r);
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "CALL 0\n");
    fprintf(intermediate, "POP R3\n");
    fprintf(intermediate, "POP R1\n");
    fprintf(intermediate, "POP R1\n");
    fprintf(intermediate, "POP R1\n");
    fprintf(intermediate, "POP R1\n");
    fprintf(intermediate, "MOV R2, \"Exit\"\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "PUSH R2\n");
    fprintf(intermediate, "CALL 0\n");
    exit(1);
}

int main(int argc,char * argv[]){
    FILE * target_file=fopen("targetcode.xsm","w");
    intermediate=target_file;
    fprintf(target_file, "%d\n%d\n%d\n%d\n%d\n%d\n%d\n%d\n",0,2056,0,0,0,0,0,0);
    fprintf(target_file,"BRKP\n");
    yyparse();
    fclose(target_file);
    return 0;
}

void yyerror(char * s) {printf("%s\n",s);}