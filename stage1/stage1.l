%{
    #include <stdio.h>
    #include "tree.h"
    #include "y.tab.h"
%}

%%
[0-9]+ {yylval=makeLeafNode(atoi(yytext));return NUM;}
"+"     {return '+';}
"("     {return '(';}
")"     {return ')';}
"*"     {return '*';}
"/"     {return '/';}
"-"     {return '-';}

[\t \n] ;
.       ; 
%%

yywrap(){return 1;}

