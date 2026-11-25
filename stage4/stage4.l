%{
    #include <stdio.h>
    #include <stdlib.h>
    #include "tree.h"
    #include "y.tab.h"

    int yyerrline=1;
%}

%%
"decl"      {return DECL;}
"enddecl"   {return ENDDECL;}
"read"      {return READ;}
"write"     {return WRITE;}
"if"        {return IF;}
"then"      {return THEN;}
"else"      {return ELSE;}
"endif"     {return ENDIF;}
"while"     {return WHILE;}
"do"        {return DO;}
"endwhile"  {return ENDWHILE;}
"break"     {return BREAK;}
"continue"  {return CONTINUE;}
"int"       {return INT;}
"str"       {return STR;}

"<="        {return LE;}
">="        {return GE;}
"=="        {return EQOP;}
"!="        {return NE;}
"<"         {return '<';}
">"         {return '>';}
"="         {return '=';}
"["         {return '[';}
"]"         {return ']';}
","         {return ',';}

\"[^\"]*\"  {
                char *s = strdup(yytext + 1);
                s[strlen(s) - 1] = '\0';
                yylval = createTree(0, STRCONST_T, s, NULL, NULL);
                return STRCONST;
            }

[0-9]+      {yylval=createTree(atoi(yytext),NUM_T,NULL,NULL,NULL);return NUM;}
[a-z][a-z0-9]*       {yylval=createTree(0,ID_T,strdup(yytext),NULL,NULL);return ID;}

"+"         {return '+';}
"-"         {return '-';}
"*"         {return '*';}
"/"         {return '/';}
"("         {return '(';}
")"         {return ')';}
";"         {return ';';}

"\n"        {yyerrline++;}
[\t]     ;
.           ;
%%

int yywrap(){return 1;}