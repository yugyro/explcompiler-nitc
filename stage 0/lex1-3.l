//to list out all words of length three, starting with "A" to uppercase.
%{
    #include <stdio.h>
%}

upp [A-Z]
low [a-z]

%option noyywrap

%%

{upp|low}{upp|low}* {if(yyleng()==3 && yytext[0]=='A') printf("%s\n",yytext);}

%%

int main(){
    yylex();
    return 1;
}