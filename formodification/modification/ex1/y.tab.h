/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2020 Free Software Foundation,
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
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

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

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

#ifndef YY_YY_Y_TAB_H_INCLUDED
# define YY_YY_Y_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    PLUS = 258,
    MINUS = 259,
    STAR = 260,
    DIV = 261,
    MOD = 262,
    AMPERSAND = 263,
    LE = 264,
    LT = 265,
    GT = 266,
    GE = 267,
    EQ = 268,
    NE = 269,
    AND = 270,
    OR = 271,
    NOT = 272,
    ASSIGN = 273,
    IF = 274,
    ELSE = 275,
    WHILE = 276,
    REPEAT = 277,
    UNTIL = 278,
    DO = 279,
    BREAK = 280,
    CONTINUE = 281,
    ENDIF = 282,
    ENDWHILE = 283,
    DECL = 284,
    ENDDECL = 285,
    BEG = 286,
    END = 287,
    INT = 288,
    STR = 289,
    SEMICOLON = 290,
    THEN = 291,
    READ = 292,
    WRITE = 293,
    EXIT_PR = 294,
    MAIN = 295,
    RETURN = 296,
    TUPLE = 297,
    DOT = 298,
    TYPE = 299,
    ENDTYPE = 300,
    NULL_VAL = 301,
    ALLOC = 302,
    INITIALIZE = 303,
    FREE = 304,
    CLASS = 305,
    ENDCLASS = 306,
    SELF = 307,
    NEW = 308,
    EXTENDS = 309,
    NUM = 310,
    STR_LITERAL = 311,
    ID = 312,
    UMINUS = 313,
    FIELD_BASE = 314
  };
#endif
/* Tokens.  */
#define PLUS 258
#define MINUS 259
#define STAR 260
#define DIV 261
#define MOD 262
#define AMPERSAND 263
#define LE 264
#define LT 265
#define GT 266
#define GE 267
#define EQ 268
#define NE 269
#define AND 270
#define OR 271
#define NOT 272
#define ASSIGN 273
#define IF 274
#define ELSE 275
#define WHILE 276
#define REPEAT 277
#define UNTIL 278
#define DO 279
#define BREAK 280
#define CONTINUE 281
#define ENDIF 282
#define ENDWHILE 283
#define DECL 284
#define ENDDECL 285
#define BEG 286
#define END 287
#define INT 288
#define STR 289
#define SEMICOLON 290
#define THEN 291
#define READ 292
#define WRITE 293
#define EXIT_PR 294
#define MAIN 295
#define RETURN 296
#define TUPLE 297
#define DOT 298
#define TYPE 299
#define ENDTYPE 300
#define NULL_VAL 301
#define ALLOC 302
#define INITIALIZE 303
#define FREE 304
#define CLASS 305
#define ENDCLASS 306
#define SELF 307
#define NEW 308
#define EXTENDS 309
#define NUM 310
#define STR_LITERAL 311
#define ID 312
#define UMINUS 313
#define FIELD_BASE 314

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 193 "parser.y"

    struct ParamStruct * paramList ;
    struct tnode * node;
    int number ;
    char * string ;
    struct Fieldlist * flist ;
    struct TypeTable * type ;

#line 184 "y.tab.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
