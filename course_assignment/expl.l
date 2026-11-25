%{
#include <string.h>
#include <stdlib.h>
#include "tac.h"
#include "y.tab.h"
int yywrap(void) { return 1; }
%}
%x COMMENT
letter [a-zA-Z]
digit  [0-9]
%%
"/*"                { BEGIN(COMMENT); }
<COMMENT>"*/"       { BEGIN(INITIAL); }
<COMMENT>\n         { lineno++; }
<COMMENT>.          { }
"//".*              { }
\n                  { lineno++; }
[ \t\r]+            { }

"decl"              return K_DECL;
"enddecl"           return K_ENDDECL;
"begin"             return K_BEGIN;
"end"               return K_END;
"if"                return K_IF;
"then"              return K_THEN;
"else"              return K_ELSE;
"endif"             return K_ENDIF;
"while"             return K_WHILE;
"do"                return K_DO;
"endwhile"          return K_ENDWHILE;
"read"              return K_READ;
"write"             return K_WRITE;
"return"            return K_RETURN;
"break"             return K_BREAK;
"continue"          return K_CONTINUE;
"breakpoint"        return K_BREAKPOINT;
"int"               return K_INT;
"str"               return K_STR;
"main"              return K_MAIN;
"and"|"AND"         return AND;
"or"|"OR"           return OR;
"not"|"NOT"         return NOT;
"type"|"endtype"|"NULL"|"alloc"|"free"|"initialize"  {
        fprintf(stderr, "line %d: error: '%s' is not supported in this subset of ExpL\n", lineno, yytext);
        nerrors++; return K_UNSUP; }

"<="                return LE;
">="                return GE;
"=="                return EQ;
"!="                return NE;
"<"                 return LT;
">"                 return GT;

{letter}({letter}|{digit})*  { yylval.str = strdup(yytext); return ID; }
{digit}+                     { yylval.num = atol(yytext); return NUM; }
\"[^\"\n]*\"                 { yylval.str = strdup(yytext + 1); yylval.str[yyleng - 2] = 0; return STRING; }
.                            { return yytext[0]; }
%%
