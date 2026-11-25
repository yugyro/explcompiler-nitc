//1.1 to count the number of lines, words, and characters in the input.

%{

#include <stdio.h>
#define NEWLINE 1;
#define WHITESPACE 2;
#define WORD 3;
int characters=0;
int words=0;
int lines=0;
%}

low [a-z]
upp [A-Z]
number [0-9]

%option noyywrap

%%

{low|upp|number}{low/upp/number}* {words++; characters+=yyleng();}
"\n" {lines++;}

%%

int main{
    yylex();
    printf("lines:%d words:%d characters:%d\n",lines,words,characters);
    return 1;
}



