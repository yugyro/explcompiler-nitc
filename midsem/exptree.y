%{
    #include <stdio.h>
    #include <ctype.h>
%}

%union{
    struct tnode * node;
    int number;
    char * string;
}

%token BEG END EXIT_PR READ WRITE IF THEN ELSE CONTINUE WHILE BREAK REPEAT UNTIL ENDIF ENDWHILE DO INT STR DECL ENDDECL 
%token LT LE EQ NE GT GE SEMICOLON ASSIGN PLUS MINUS STAR DIV MOD AMPERSAND 

%token <number> NUM
%token <node>  ID
%token <string> STR_LITERAL

%type 

program: Declarations BEG stmtList END SEMICOLON
        | Declarations BEG END SEMICOLON
        ;

Declarations: DECL DeclList ENDDECL
        |  DECL ENDDECL
        ;

DeclList: DeclList Decl 
          | Decl 
        ;

Decl: Type VarList
        ;

Type: INT|STR
        ;

VarList: Varlist , ID
        | ID [NUM]
        | ID [NUM] [NUM]
        | STAR ID
        | AMPERSAND ID
        | VarList, ID [NUM]
        | VarList, ID [NUM] [NUM]
        | VarList, ID *
        | ID
        ;

stmtList: stmt stmtList
          | stmt 
        ;

stmt: ipStmt
      |opStmt
      |
