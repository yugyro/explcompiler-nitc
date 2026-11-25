//count number of integers and floating point numbers

%{
    #include <stdio.h>
    int numint=0;
    int numfloat=0;
%}

digits [0-9]
decimal [.]
sign [-]

%option noyywrap

%%
{sign}{digits}{digits}* {numint++;}
{sign}{digits}*{decimal}{digits}{digits}* {numfloat++;}
%%

int main(){
    yylex();
    return 1;
}