%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"
    #include "y.tab.h"
%}

%%
"begin" {return BEGIN_T;}
"end"   {return END_T;}
"read"  {return READ;}
"write" {return WRITE;}
"="     {return '=';}
[0-9]+  {yylval=createTree(atoi(yytext),NUM_T,NULL,NULL,NULL);return NUM;}
[a-z]   {yylval=createTree(0,ID_T,strdup(yytext),NULL,NULL);return ID;}
"+"     {return '+';}
"("     {return '(';}
")"     {return ')';}
"*"     {return '*';}
"/"     {return '/';}
"-"     {return '-';}
";"     {return ';';}

[\t \n] ;
.       ; 
%%

yywrap(){return 1;}

