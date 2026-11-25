%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
%}

%option noyywrap
%option yylineno

digit   [0-9]
letter  [a-zA-Z]

%%
"decl"        { return DECL; }
"enddecl"     { return ENDDECL; }
"begin"       { return T_BEGIN; }
"end"         { return T_END; }
"int"         { return INT; }
"read"        { return READ; }
"write"       { return WRITE; }
"if"          { return IF; }
"then"        { return THEN; }
"else"        { return ELSE; }
"endif"       { return ENDIF; }
"while"       { return WHILE; }
"do"          { return DO; }
"endwhile"    { return ENDWHILE; }

{letter}({letter}|{digit})*  { yylval.s = strdup(yytext); return ID; }
{digit}+                     { yylval.s = strdup(yytext); return NUM; }

"<="          { return LE; }
">="          { return GE; }
"=="          { return EQ; }
"!="          { return NE; }
"&&"          { return AND; }
"||"          { return OR; }
[-+*/%<>=(),;] { return yytext[0]; }

"//".*        { }
[ \t\r\n]+    { }
.             { printf("Lexical error: unknown character '%s' at line %d\n", yytext, yylineno); }
%%
