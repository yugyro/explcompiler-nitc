/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    PLUS = 258,                    /* PLUS  */
    MINUS = 259,                   /* MINUS  */
    STAR = 260,                    /* STAR  */
    DIV = 261,                     /* DIV  */
    AMPERSAND = 262,               /* AMPERSAND  */
    LE = 263,                      /* LE  */
    LT = 264,                      /* LT  */
    GT = 265,                      /* GT  */
    GE = 266,                      /* GE  */
    EQ = 267,                      /* EQ  */
    NE = 268,                      /* NE  */
    AND = 269,                     /* AND  */
    OR = 270,                      /* OR  */
    NOT = 271,                     /* NOT  */
    ASSIGN = 272,                  /* ASSIGN  */
    IF = 273,                      /* IF  */
    ELSE = 274,                    /* ELSE  */
    WHILE = 275,                   /* WHILE  */
    REPEAT = 276,                  /* REPEAT  */
    UNTIL = 277,                   /* UNTIL  */
    DO = 278,                      /* DO  */
    BREAK = 279,                   /* BREAK  */
    CONTINUE = 280,                /* CONTINUE  */
    ENDIF = 281,                   /* ENDIF  */
    ENDWHILE = 282,                /* ENDWHILE  */
    DECL = 283,                    /* DECL  */
    ENDDECL = 284,                 /* ENDDECL  */
    BEG = 285,                     /* BEG  */
    END = 286,                     /* END  */
    INT = 287,                     /* INT  */
    STR = 288,                     /* STR  */
    SEMICOLON = 289,               /* SEMICOLON  */
    THEN = 290,                    /* THEN  */
    READ = 291,                    /* READ  */
    WRITE = 292,                   /* WRITE  */
    EXIT_PR = 293,                 /* EXIT_PR  */
    MAIN = 294,                    /* MAIN  */
    RETURN = 295,                  /* RETURN  */
    TUPLE = 296,                   /* TUPLE  */
    DOT = 297,                     /* DOT  */
    TYPE = 298,                    /* TYPE  */
    ENDTYPE = 299,                 /* ENDTYPE  */
    NULL_VAL = 300,                /* NULL_VAL  */
    ALLOC = 301,                   /* ALLOC  */
    INITIALIZE = 302,              /* INITIALIZE  */
    FREE = 303,                    /* FREE  */
    CLASS = 304,                   /* CLASS  */
    ENDCLASS = 305,                /* ENDCLASS  */
    SELF = 306,                    /* SELF  */
    NEW = 307,                     /* NEW  */
    NUM = 308,                     /* NUM  */
    STR_LITERAL = 309,             /* STR_LITERAL  */
    ID = 310,                      /* ID  */
    FIELD_BASE = 311               /* FIELD_BASE  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define PLUS 258
#define MINUS 259
#define STAR 260
#define DIV 261
#define AMPERSAND 262
#define LE 263
#define LT 264
#define GT 265
#define GE 266
#define EQ 267
#define NE 268
#define AND 269
#define OR 270
#define NOT 271
#define ASSIGN 272
#define IF 273
#define ELSE 274
#define WHILE 275
#define REPEAT 276
#define UNTIL 277
#define DO 278
#define BREAK 279
#define CONTINUE 280
#define ENDIF 281
#define ENDWHILE 282
#define DECL 283
#define ENDDECL 284
#define BEG 285
#define END 286
#define INT 287
#define STR 288
#define SEMICOLON 289
#define THEN 290
#define READ 291
#define WRITE 292
#define EXIT_PR 293
#define MAIN 294
#define RETURN 295
#define TUPLE 296
#define DOT 297
#define TYPE 298
#define ENDTYPE 299
#define NULL_VAL 300
#define ALLOC 301
#define INITIALIZE 302
#define FREE 303
#define CLASS 304
#define ENDCLASS 305
#define SELF 306
#define NEW 307
#define NUM 308
#define STR_LITERAL 309
#define ID 310
#define FIELD_BASE 311

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 112 "parser.y"

    struct ParamStruct * paramList ;
    struct tnode * node;
    int number ;
    char * string ;
    struct Fieldlist * flist ;
    struct TypeTable * type ;

#line 188 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
