/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output, and Bison version.  */
#define YYBISON 30802

/* Bison version string.  */
#define YYBISON_VERSION "3.8.2"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* First part of user prologue.  */
#line 1 "parser.y"

    #include <stdlib.h>
    #include <stdio.h>
    #include <string.h>
    #include "exptree.h"
    #include "symbol.h"
    #include "typetable.h"
    #include "codegen.h"
    #include "classtable.h"

    int yylex(void);
    int yyerror(const char *s);

    FILE * output;
    extern int yylineNum; 
    extern int localBinding ;
    int flabel = 1;
    bool isPointer = false ;

    struct Gsymbol * Ghead  = NULL ;
    struct Lsymbol * Lhead  = NULL ; 
    struct ParamStruct * paramHead = NULL ;
    struct Gsymbol * currentFunc = NULL ;  //Current function for return type checking 

    struct TypeTable * currentType ;
    struct TypeTable * paramType ;
    struct Classtable * paramClassType = NULL;
    struct TypeTable * currLocalType ;
    struct Classtable * currLocalClassType = NULL;

    struct TypeTable * TTableHead = NULL ;
    struct TypeTable * currTupleType = NULL ;

    /* Stage 7 front-end state */
    struct Classtable * CTableHead = NULL;
    struct Classtable * currentClass = NULL;
    struct MemberFunc * currentMethod = NULL;
    struct Classtable * currentClassType = NULL;
    int hasClassDefinitions = 0;
    int methodFlabel = 100;
    int seenMethodDecl = 0;
    char *pendingMemberName = NULL;

    static void stage7Error(const char *msg) {
        fprintf(stderr, "Error: %s at line %d\n", msg, yylineNum);
        exit(1);
    }

    static int checkMethodArgs(struct MemberFunc *m, struct tnode *args) {
        struct ParamStruct *p = m ? m->paramList : NULL;
        struct tnode *a = args;
        while (p && a) {
            if (!((a->nodetype == NULL_NODE && (isUserDefinedType(p->type) || p->Ctype)) ||
                  (p->Ctype ? (p->Ctype == a->Ctype) : (p->type == a->type && p->isPtr == a->isPtr)))) {
                fprintf(stderr, "Error: Argument type mismatch in method '%s' at line %d\n", m->name, yylineNum);
                exit(1);
            }
            p = p->next; a = a->argList;
        }
        if (p || a) {
            fprintf(stderr, "Error: Argument count mismatch in method '%s' at line %d\n", m->name, yylineNum);
            exit(1);
        }
        return 1;
    }

    static struct tnode *makeMethodCall(struct MemberFunc *m, struct tnode *args, struct tnode *receiver) {
        if (!m) stage7Error("Method is not declared in this class");
        checkMethodArgs(m, args);
        struct tnode *n = createTreeNode(0, NO_VAL, m->type, strdup(m->name), METHOD_CALL_NODE, NULL, NULL, NULL, NULL);
        n->argList = args;
        n->left = receiver;
        n->val = m->flabel;
        return n;
    }

    static struct tnode *makeSelfReceiver(void) {
        struct tnode *self = createTreeNode(0, NO_VAL, T_VOID, strdup("self"), ID_NODE, NULL, NULL, NULL, NULL);
        self->Lentry = findLocalSymbol(Lhead, "self");
        if (!self->Lentry) stage7Error("Internal error: self is missing from method local table");
        return self;
    }

    static struct tnode *makeSelfFieldReceiver(struct ClassField *f) {
        if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
        struct tnode *receiver = createTreeNode(0, f->fieldIndex, f->type,
                                                  strdup(f->name), SELF_FIELD_NODE,
                                                  NULL, NULL, NULL, NULL);
        receiver->Ctype = f->ctype;
        receiver->Lentry = findLocalSymbol(Lhead, "self");
        if (!receiver->Lentry) stage7Error("Internal error: self is missing from method local table");
        return receiver;
    }

    static int sameMethodSignature(struct MemberFunc *m, struct TypeTable *ret, struct ParamStruct *p) {
        if (!m || m->type != ret) return 0;
        struct ParamStruct *q = m->paramList;
        while (q && p) {
            if (q->type != p->type || q->Ctype != p->Ctype || q->isPtr != p->isPtr) return 0;
            q = q->next; p = p->next;
        }
        return q == NULL && p == NULL;
    }


    // For user-defined type definitions (Stage 6)
    struct Fieldlist * currFieldHead = NULL ;
    int currFieldIndex = 0 ;

#line 181 "y.tab.c"

# ifndef YY_CAST
#  ifdef __cplusplus
#   define YY_CAST(Type, Val) static_cast<Type> (Val)
#   define YY_REINTERPRET_CAST(Type, Val) reinterpret_cast<Type> (Val)
#  else
#   define YY_CAST(Type, Val) ((Type) (Val))
#   define YY_REINTERPRET_CAST(Type, Val) ((Type) (Val))
#  endif
# endif
# ifndef YY_NULLPTR
#  if defined __cplusplus
#   if 201103L <= __cplusplus
#    define YY_NULLPTR nullptr
#   else
#    define YY_NULLPTR 0
#   endif
#  else
#   define YY_NULLPTR ((void*)0)
#  endif
# endif

/* Use api.header.include to #include this header
   instead of duplicating it here.  */
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

#line 355 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_Y_TAB_H_INCLUDED  */
/* Symbol kind.  */
enum yysymbol_kind_t
{
  YYSYMBOL_YYEMPTY = -2,
  YYSYMBOL_YYEOF = 0,                      /* "end of file"  */
  YYSYMBOL_YYerror = 1,                    /* error  */
  YYSYMBOL_YYUNDEF = 2,                    /* "invalid token"  */
  YYSYMBOL_PLUS = 3,                       /* PLUS  */
  YYSYMBOL_MINUS = 4,                      /* MINUS  */
  YYSYMBOL_STAR = 5,                       /* STAR  */
  YYSYMBOL_DIV = 6,                        /* DIV  */
  YYSYMBOL_AMPERSAND = 7,                  /* AMPERSAND  */
  YYSYMBOL_LE = 8,                         /* LE  */
  YYSYMBOL_LT = 9,                         /* LT  */
  YYSYMBOL_GT = 10,                        /* GT  */
  YYSYMBOL_GE = 11,                        /* GE  */
  YYSYMBOL_EQ = 12,                        /* EQ  */
  YYSYMBOL_NE = 13,                        /* NE  */
  YYSYMBOL_AND = 14,                       /* AND  */
  YYSYMBOL_OR = 15,                        /* OR  */
  YYSYMBOL_NOT = 16,                       /* NOT  */
  YYSYMBOL_ASSIGN = 17,                    /* ASSIGN  */
  YYSYMBOL_IF = 18,                        /* IF  */
  YYSYMBOL_ELSE = 19,                      /* ELSE  */
  YYSYMBOL_WHILE = 20,                     /* WHILE  */
  YYSYMBOL_REPEAT = 21,                    /* REPEAT  */
  YYSYMBOL_UNTIL = 22,                     /* UNTIL  */
  YYSYMBOL_DO = 23,                        /* DO  */
  YYSYMBOL_BREAK = 24,                     /* BREAK  */
  YYSYMBOL_CONTINUE = 25,                  /* CONTINUE  */
  YYSYMBOL_ENDIF = 26,                     /* ENDIF  */
  YYSYMBOL_ENDWHILE = 27,                  /* ENDWHILE  */
  YYSYMBOL_DECL = 28,                      /* DECL  */
  YYSYMBOL_ENDDECL = 29,                   /* ENDDECL  */
  YYSYMBOL_BEG = 30,                       /* BEG  */
  YYSYMBOL_END = 31,                       /* END  */
  YYSYMBOL_INT = 32,                       /* INT  */
  YYSYMBOL_STR = 33,                       /* STR  */
  YYSYMBOL_SEMICOLON = 34,                 /* SEMICOLON  */
  YYSYMBOL_THEN = 35,                      /* THEN  */
  YYSYMBOL_READ = 36,                      /* READ  */
  YYSYMBOL_WRITE = 37,                     /* WRITE  */
  YYSYMBOL_EXIT_PR = 38,                   /* EXIT_PR  */
  YYSYMBOL_MAIN = 39,                      /* MAIN  */
  YYSYMBOL_RETURN = 40,                    /* RETURN  */
  YYSYMBOL_TUPLE = 41,                     /* TUPLE  */
  YYSYMBOL_DOT = 42,                       /* DOT  */
  YYSYMBOL_TYPE = 43,                      /* TYPE  */
  YYSYMBOL_ENDTYPE = 44,                   /* ENDTYPE  */
  YYSYMBOL_NULL_VAL = 45,                  /* NULL_VAL  */
  YYSYMBOL_ALLOC = 46,                     /* ALLOC  */
  YYSYMBOL_INITIALIZE = 47,                /* INITIALIZE  */
  YYSYMBOL_FREE = 48,                      /* FREE  */
  YYSYMBOL_CLASS = 49,                     /* CLASS  */
  YYSYMBOL_ENDCLASS = 50,                  /* ENDCLASS  */
  YYSYMBOL_SELF = 51,                      /* SELF  */
  YYSYMBOL_NEW = 52,                       /* NEW  */
  YYSYMBOL_NUM = 53,                       /* NUM  */
  YYSYMBOL_STR_LITERAL = 54,               /* STR_LITERAL  */
  YYSYMBOL_ID = 55,                        /* ID  */
  YYSYMBOL_FIELD_BASE = 56,                /* FIELD_BASE  */
  YYSYMBOL_57_ = 57,                       /* '{'  */
  YYSYMBOL_58_ = 58,                       /* '}'  */
  YYSYMBOL_59_ = 59,                       /* '('  */
  YYSYMBOL_60_ = 60,                       /* ')'  */
  YYSYMBOL_61_ = 61,                       /* ','  */
  YYSYMBOL_62_ = 62,                       /* '['  */
  YYSYMBOL_63_ = 63,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 64,                  /* $accept  */
  YYSYMBOL_program = 65,                   /* program  */
  YYSYMBOL_TypeDefBlock = 66,              /* TypeDefBlock  */
  YYSYMBOL_TypeDefList = 67,               /* TypeDefList  */
  YYSYMBOL_TypeDef = 68,                   /* TypeDef  */
  YYSYMBOL_69_1 = 69,                      /* $@1  */
  YYSYMBOL_ClassDefBlock = 70,             /* ClassDefBlock  */
  YYSYMBOL_ClassDefList = 71,              /* ClassDefList  */
  YYSYMBOL_ClassDef = 72,                  /* ClassDef  */
  YYSYMBOL_73_2 = 73,                      /* $@2  */
  YYSYMBOL_ClassMemberList = 74,           /* ClassMemberList  */
  YYSYMBOL_ClassMember = 75,               /* ClassMember  */
  YYSYMBOL_76_3 = 76,                      /* $@3  */
  YYSYMBOL_ClassMemberType = 77,           /* ClassMemberType  */
  YYSYMBOL_ClassMemberTail = 78,           /* ClassMemberTail  */
  YYSYMBOL_MethodDefList = 79,             /* MethodDefList  */
  YYSYMBOL_MethodDef = 80,                 /* MethodDef  */
  YYSYMBOL_81_4 = 81,                      /* $@4  */
  YYSYMBOL_MainBlock = 82,                 /* MainBlock  */
  YYSYMBOL_83_5 = 83,                      /* $@5  */
  YYSYMBOL_GDeclBlock = 84,                /* GDeclBlock  */
  YYSYMBOL_GDeclList = 85,                 /* GDeclList  */
  YYSYMBOL_GDecl = 86,                     /* GDecl  */
  YYSYMBOL_GTupleDecl = 87,                /* GTupleDecl  */
  YYSYMBOL_88_6 = 88,                      /* $@6  */
  YYSYMBOL_GTupleIdList = 89,              /* GTupleIdList  */
  YYSYMBOL_GTupleId = 90,                  /* GTupleId  */
  YYSYMBOL_FieldDefList = 91,              /* FieldDefList  */
  YYSYMBOL_FieldDef = 92,                  /* FieldDef  */
  YYSYMBOL_FieldType = 93,                 /* FieldType  */
  YYSYMBOL_GidList = 94,                   /* GidList  */
  YYSYMBOL_Gid = 95,                       /* Gid  */
  YYSYMBOL_FDefBlock = 96,                 /* FDefBlock  */
  YYSYMBOL_Body = 97,                      /* Body  */
  YYSYMBOL_ParamList = 98,                 /* ParamList  */
  YYSYMBOL_Param = 99,                     /* Param  */
  YYSYMBOL_Ret = 100,                      /* Ret  */
  YYSYMBOL_LdeclBlock = 101,               /* LdeclBlock  */
  YYSYMBOL_LDeclList = 102,                /* LDeclList  */
  YYSYMBOL_LDecl = 103,                    /* LDecl  */
  YYSYMBOL_LTupleDecl = 104,               /* LTupleDecl  */
  YYSYMBOL_105_7 = 105,                    /* $@7  */
  YYSYMBOL_LTupleIdList = 106,             /* LTupleIdList  */
  YYSYMBOL_LTupleId = 107,                 /* LTupleId  */
  YYSYMBOL_IdList = 108,                   /* IdList  */
  YYSYMBOL_GType = 109,                    /* GType  */
  YYSYMBOL_ParType = 110,                  /* ParType  */
  YYSYMBOL_LType = 111,                    /* LType  */
  YYSYMBOL_EXPR = 112,                     /* EXPR  */
  YYSYMBOL_ArgList = 113,                  /* ArgList  */
  YYSYMBOL_stmtList = 114,                 /* stmtList  */
  YYSYMBOL_stmt = 115,                     /* stmt  */
  YYSYMBOL_Fdef = 116,                     /* Fdef  */
  YYSYMBOL_117_8 = 117,                    /* $@8  */
  YYSYMBOL_ifStmt = 118,                   /* ifStmt  */
  YYSYMBOL_whileStmt = 119,                /* whileStmt  */
  YYSYMBOL_doWhileStmt = 120,              /* doWhileStmt  */
  YYSYMBOL_repeatStmt = 121,               /* repeatStmt  */
  YYSYMBOL_ipStmt = 122,                   /* ipStmt  */
  YYSYMBOL_opStmt = 123,                   /* opStmt  */
  YYSYMBOL_assignStmt = 124,               /* assignStmt  */
  YYSYMBOL_IDENTIFIERS = 125               /* IDENTIFIERS  */
};
typedef enum yysymbol_kind_t yysymbol_kind_t;




#ifdef short
# undef short
#endif

/* On compilers that do not define __PTRDIFF_MAX__ etc., make sure
   <limits.h> and (if available) <stdint.h> are included
   so that the code can choose integer types of a good width.  */

#ifndef __PTRDIFF_MAX__
# include <limits.h> /* INFRINGES ON USER NAME SPACE */
# if defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stdint.h> /* INFRINGES ON USER NAME SPACE */
#  define YY_STDINT_H
# endif
#endif

/* Narrow types that promote to a signed type and that can represent a
   signed or unsigned integer of at least N bits.  In tables they can
   save space and decrease cache pressure.  Promoting to a signed type
   helps avoid bugs in integer arithmetic.  */

#ifdef __INT_LEAST8_MAX__
typedef __INT_LEAST8_TYPE__ yytype_int8;
#elif defined YY_STDINT_H
typedef int_least8_t yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef __INT_LEAST16_MAX__
typedef __INT_LEAST16_TYPE__ yytype_int16;
#elif defined YY_STDINT_H
typedef int_least16_t yytype_int16;
#else
typedef short yytype_int16;
#endif

/* Work around bug in HP-UX 11.23, which defines these macros
   incorrectly for preprocessor constants.  This workaround can likely
   be removed in 2023, as HPE has promised support for HP-UX 11.23
   (aka HP-UX 11i v2) only through the end of 2022; see Table 2 of
   <https://h20195.www2.hpe.com/V2/getpdf.aspx/4AA4-7673ENW.pdf>.  */
#ifdef __hpux
# undef UINT_LEAST8_MAX
# undef UINT_LEAST16_MAX
# define UINT_LEAST8_MAX 255
# define UINT_LEAST16_MAX 65535
#endif

#if defined __UINT_LEAST8_MAX__ && __UINT_LEAST8_MAX__ <= __INT_MAX__
typedef __UINT_LEAST8_TYPE__ yytype_uint8;
#elif (!defined __UINT_LEAST8_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST8_MAX <= INT_MAX)
typedef uint_least8_t yytype_uint8;
#elif !defined __UINT_LEAST8_MAX__ && UCHAR_MAX <= INT_MAX
typedef unsigned char yytype_uint8;
#else
typedef short yytype_uint8;
#endif

#if defined __UINT_LEAST16_MAX__ && __UINT_LEAST16_MAX__ <= __INT_MAX__
typedef __UINT_LEAST16_TYPE__ yytype_uint16;
#elif (!defined __UINT_LEAST16_MAX__ && defined YY_STDINT_H \
       && UINT_LEAST16_MAX <= INT_MAX)
typedef uint_least16_t yytype_uint16;
#elif !defined __UINT_LEAST16_MAX__ && USHRT_MAX <= INT_MAX
typedef unsigned short yytype_uint16;
#else
typedef int yytype_uint16;
#endif

#ifndef YYPTRDIFF_T
# if defined __PTRDIFF_TYPE__ && defined __PTRDIFF_MAX__
#  define YYPTRDIFF_T __PTRDIFF_TYPE__
#  define YYPTRDIFF_MAXIMUM __PTRDIFF_MAX__
# elif defined PTRDIFF_MAX
#  ifndef ptrdiff_t
#   include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  endif
#  define YYPTRDIFF_T ptrdiff_t
#  define YYPTRDIFF_MAXIMUM PTRDIFF_MAX
# else
#  define YYPTRDIFF_T long
#  define YYPTRDIFF_MAXIMUM LONG_MAX
# endif
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif defined __STDC_VERSION__ && 199901 <= __STDC_VERSION__
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM                                  \
  YY_CAST (YYPTRDIFF_T,                                 \
           (YYPTRDIFF_MAXIMUM < YY_CAST (YYSIZE_T, -1)  \
            ? YYPTRDIFF_MAXIMUM                         \
            : YY_CAST (YYSIZE_T, -1)))

#define YYSIZEOF(X) YY_CAST (YYPTRDIFF_T, sizeof (X))


/* Stored state numbers (used for stacks). */
typedef yytype_int16 yy_state_t;

/* State numbers in computations.  */
typedef int yy_state_fast_t;

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif


#ifndef YY_ATTRIBUTE_PURE
# if defined __GNUC__ && 2 < __GNUC__ + (96 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_PURE __attribute__ ((__pure__))
# else
#  define YY_ATTRIBUTE_PURE
# endif
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# if defined __GNUC__ && 2 < __GNUC__ + (7 <= __GNUC_MINOR__)
#  define YY_ATTRIBUTE_UNUSED __attribute__ ((__unused__))
# else
#  define YY_ATTRIBUTE_UNUSED
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YY_USE(E) ((void) (E))
#else
# define YY_USE(E) /* empty */
#endif

/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
#if defined __GNUC__ && ! defined __ICC && 406 <= __GNUC__ * 100 + __GNUC_MINOR__
# if __GNUC__ * 100 + __GNUC_MINOR__ < 407
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")
# else
#  define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                           \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# endif
# define YY_IGNORE_MAYBE_UNINITIALIZED_END      \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif

#if defined __cplusplus && defined __GNUC__ && ! defined __ICC && 6 <= __GNUC__
# define YY_IGNORE_USELESS_CAST_BEGIN                          \
    _Pragma ("GCC diagnostic push")                            \
    _Pragma ("GCC diagnostic ignored \"-Wuseless-cast\"")
# define YY_IGNORE_USELESS_CAST_END            \
    _Pragma ("GCC diagnostic pop")
#endif
#ifndef YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_BEGIN
# define YY_IGNORE_USELESS_CAST_END
#endif


#define YY_ASSERT(E) ((void) (0 && (E)))

#if !defined yyoverflow

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* !defined yyoverflow */

#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yy_state_t yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (YYSIZEOF (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (YYSIZEOF (yy_state_t) + YYSIZEOF (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYPTRDIFF_T yynewbytes;                                         \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * YYSIZEOF (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / YYSIZEOF (*yyptr);                        \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, YY_CAST (YYSIZE_T, (Count)) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYPTRDIFF_T yyi;                      \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  7
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   587

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  64
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  62
/* YYNRULES -- Number of rules.  */
#define YYNRULES  152
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  326

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   311


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK                     \
   ? YY_CAST (yysymbol_kind_t, yytranslate[YYX])        \
   : YYSYMBOL_YYUNDEF)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      59,    60,     2,     2,    61,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    62,     2,    63,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    57,     2,    58,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27,    28,    29,    30,    31,    32,    33,    34,
      35,    36,    37,    38,    39,    40,    41,    42,    43,    44,
      45,    46,    47,    48,    49,    50,    51,    52,    53,    54,
      55,    56
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   150,   150,   155,   156,   159,   160,   164,   163,   194,
     195,   198,   199,   202,   202,   224,   225,   228,   228,   233,
     234,   235,   247,   253,   261,   262,   263,   266,   266,   300,
     300,   323,   324,   325,   328,   329,   332,   333,   336,   336,
     354,   355,   358,   359,   363,   374,   385,   389,   396,   400,
     401,   402,   412,   413,   416,   420,   424,   429,   435,   436,
     439,   442,   445,   448,   453,   454,   455,   458,   459,   462,
     466,   467,   470,   471,   474,   475,   478,   478,   495,   496,
     499,   500,   504,   505,   508,   509,   510,   523,   524,   525,
     536,   537,   538,   550,   551,   552,   553,   554,   555,   556,
     557,   558,   559,   560,   561,   562,   563,   564,   565,   566,
     567,   568,   569,   575,   576,   577,   580,   581,   585,   589,
     603,   617,   623,   631,   634,   640,   641,   644,   645,   646,
     647,   648,   649,   650,   651,   652,   653,   656,   656,   677,
     681,   687,   692,   695,   698,   701,   704,   707,   716,   721,
     726,   731,   736
};
#endif

/** Accessing symbol of state STATE.  */
#define YY_ACCESSING_SYMBOL(State) YY_CAST (yysymbol_kind_t, yystos[State])

#if YYDEBUG || 0
/* The user-facing name of the symbol whose (internal) number is
   YYSYMBOL.  No bounds checking.  */
static const char *yysymbol_name (yysymbol_kind_t yysymbol) YY_ATTRIBUTE_UNUSED;

/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "\"end of file\"", "error", "\"invalid token\"", "PLUS", "MINUS",
  "STAR", "DIV", "AMPERSAND", "LE", "LT", "GT", "GE", "EQ", "NE", "AND",
  "OR", "NOT", "ASSIGN", "IF", "ELSE", "WHILE", "REPEAT", "UNTIL", "DO",
  "BREAK", "CONTINUE", "ENDIF", "ENDWHILE", "DECL", "ENDDECL", "BEG",
  "END", "INT", "STR", "SEMICOLON", "THEN", "READ", "WRITE", "EXIT_PR",
  "MAIN", "RETURN", "TUPLE", "DOT", "TYPE", "ENDTYPE", "NULL_VAL", "ALLOC",
  "INITIALIZE", "FREE", "CLASS", "ENDCLASS", "SELF", "NEW", "NUM",
  "STR_LITERAL", "ID", "FIELD_BASE", "'{'", "'}'", "'('", "')'", "','",
  "'['", "']'", "$accept", "program", "TypeDefBlock", "TypeDefList",
  "TypeDef", "$@1", "ClassDefBlock", "ClassDefList", "ClassDef", "$@2",
  "ClassMemberList", "ClassMember", "$@3", "ClassMemberType",
  "ClassMemberTail", "MethodDefList", "MethodDef", "$@4", "MainBlock",
  "$@5", "GDeclBlock", "GDeclList", "GDecl", "GTupleDecl", "$@6",
  "GTupleIdList", "GTupleId", "FieldDefList", "FieldDef", "FieldType",
  "GidList", "Gid", "FDefBlock", "Body", "ParamList", "Param", "Ret",
  "LdeclBlock", "LDeclList", "LDecl", "LTupleDecl", "$@7", "LTupleIdList",
  "LTupleId", "IdList", "GType", "ParType", "LType", "EXPR", "ArgList",
  "stmtList", "stmt", "Fdef", "$@8", "ifStmt", "whileStmt", "doWhileStmt",
  "repeatStmt", "ipStmt", "opStmt", "assignStmt", "IDENTIFIERS", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-255)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-1)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -24,    -2,    24,    47,  -255,    49,  -255,  -255,    62,   114,
      89,  -255,  -255,   103,   -41,  -255,   112,  -255,   136,  -255,
    -255,  -255,  -255,  -255,  -255,   109,  -255,   213,  -255,   183,
      15,   245,  -255,  -255,  -255,    -9,  -255,   149,   212,   188,
    -255,  -255,  -255,   194,    33,    57,  -255,   204,  -255,   198,
    -255,   136,  -255,   136,  -255,  -255,   136,  -255,   309,   202,
    -255,    15,   200,   217,  -255,  -255,   106,   -22,  -255,  -255,
    -255,   111,  -255,    16,   206,  -255,   231,   309,   333,  -255,
    -255,  -255,  -255,   240,  -255,  -255,   309,   253,  -255,  -255,
     255,   201,   104,  -255,   262,  -255,    17,  -255,  -255,  -255,
     269,  -255,  -255,   270,    54,   288,  -255,   298,  -255,   318,
    -255,   309,  -255,   309,  -255,  -255,    17,   219,   338,   318,
     203,   205,  -255,  -255,  -255,  -255,   307,  -255,   482,  -255,
     329,   315,   362,   317,   338,   324,   350,   335,  -255,  -255,
    -255,  -255,    63,   340,   341,   342,   347,   462,   462,   363,
     373,   351,   352,   375,   302,   370,   365,   383,   362,  -255,
    -255,  -255,  -255,  -255,  -255,  -255,  -255,    72,  -255,   357,
    -255,  -255,   136,  -255,   374,  -255,  -255,   302,   302,   398,
     483,  -255,  -255,    35,   302,  -255,   302,  -255,   366,   369,
     371,   389,   379,  -255,  -255,    93,   302,   117,   390,   378,
     302,   407,   417,  -255,   302,   395,  -255,   318,   -16,  -255,
      97,   170,   392,   393,   -31,   184,   563,   394,   399,    35,
     413,   416,   234,   197,   302,   302,   302,   302,   302,   302,
     302,   302,   302,   302,   302,   302,  -255,   418,  -255,    53,
    -255,   428,   536,  -255,   338,  -255,   437,   452,   302,   302,
     445,   447,  -255,  -255,   -25,   -29,   424,  -255,   549,   211,
    -255,   319,   319,  -255,  -255,   574,   574,   574,   574,   574,
     574,   563,   563,   430,  -255,  -255,  -255,   434,    18,   462,
     462,   210,   223,  -255,  -255,  -255,   438,   251,  -255,  -255,
     302,   268,  -255,   439,  -255,   435,  -255,   353,   419,   463,
     120,   443,  -255,   274,   549,  -255,   291,  -255,    18,   462,
     471,   475,  -255,  -255,   285,  -255,  -255,  -255,   440,  -255,
    -255,  -255,   332,   476,  -255,  -255
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,    10,     7,     0,     6,     1,     0,    33,
       0,     3,     5,     0,     0,    12,     0,    59,     0,    13,
       9,    11,    32,    84,    85,     0,    86,     0,    35,     0,
       0,     0,    49,    50,    51,     0,    47,     0,     0,     0,
      31,    34,    37,     0,    54,     0,    53,    84,     2,     0,
      58,    46,     8,     0,    48,    16,     0,    57,    66,     0,
      36,     0,     0,     0,    45,    44,     0,     0,    87,    88,
      89,     0,    65,     0,     0,    52,     0,    66,    26,    19,
      20,    21,    15,     0,    38,    56,     0,     0,    67,    55,
       0,     0,     0,    25,     0,    17,     0,    64,    68,    29,
       0,    14,    24,     0,     0,     0,    42,    39,    41,     0,
     137,    66,    22,    66,    18,    43,     0,     0,     0,     0,
       0,     0,    40,    71,    90,    91,     0,    92,     0,    73,
       0,     0,     0,     0,     0,     0,     0,     0,    70,    72,
      75,    83,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,   148,     0,     0,   126,
     130,   131,   132,   133,   127,   128,   129,     0,    30,     0,
      27,    23,     0,    74,     0,   150,   151,     0,     0,     0,
       0,   134,   135,     0,     0,   136,     0,   110,     0,     0,
       0,     0,     0,   107,   108,   148,     0,     0,   109,     0,
       0,    63,     0,   125,     0,     0,   138,     0,     0,    82,
       0,     0,     0,     0,     0,     0,    99,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,    69,     0,   147,     0,
      61,    62,     0,   152,     0,    76,     0,     0,     0,     0,
       0,     0,   111,   113,     0,   147,     0,   115,   124,     0,
     106,    94,    93,    95,    96,   100,   101,   102,   103,   104,
     105,    97,    98,   152,   149,    60,   146,     0,     0,     0,
       0,     0,     0,   144,   145,   114,     0,     0,   112,   116,
       0,     0,    28,     0,    80,    77,    79,     0,     0,     0,
       0,     0,   117,     0,   123,   119,     0,    81,     0,     0,
       0,     0,   143,   142,     0,   118,   120,    78,     0,   140,
     141,   121,     0,     0,   122,   139
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -255,  -255,  -255,  -255,   507,  -255,  -255,  -255,   502,  -255,
    -255,  -255,  -255,  -255,  -255,  -255,   426,  -255,  -255,  -255,
    -255,  -255,   495,  -255,  -255,  -255,   408,   -53,   105,  -255,
    -255,   464,  -255,  -129,   -70,   441,   368,  -113,  -255,   400,
    -255,  -255,  -255,   221,  -255,   -23,  -255,  -255,  -150,  -254,
    -146,  -148,  -255,  -255,  -255,  -255,  -255,  -255,  -255,  -255,
    -255,  -132
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     3,     5,     6,    10,     9,    14,    15,    38,
      66,    82,   104,    83,   114,    92,    93,   207,    48,   109,
      17,    27,    28,    29,    96,   107,   108,    35,    36,    37,
      45,    46,    31,   133,    71,    72,   157,   118,   128,   129,
     130,   278,   295,   296,   142,    30,    73,   131,   258,   259,
     158,   159,    50,   119,   160,   161,   162,   163,   164,   165,
     166,   198
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     167,   179,   180,    67,   197,   169,   134,    91,    49,    20,
     203,   205,    51,   286,    13,   167,   167,   205,    51,     1,
      43,    87,   105,   293,     7,    51,   167,   210,   211,   250,
     287,   203,   203,   303,   215,   285,   216,   306,    84,    53,
     143,   120,   144,   121,   245,    53,   223,   167,   167,    52,
     239,   214,    53,     4,   242,    94,   224,   225,   226,   227,
     322,   228,   229,   230,   231,   232,   233,   234,   235,    94,
      44,    88,   106,   294,   261,   262,   263,   264,   265,   266,
     267,   268,   269,   270,   271,   272,   155,   254,   112,   204,
     156,    60,    58,    11,   244,    59,     8,   173,   281,   282,
     224,   225,   226,   227,     4,   228,   229,   230,   231,   232,
     233,   234,   235,   113,   205,   277,   274,    13,    61,   208,
     224,   225,   226,   227,   174,   228,   229,   230,   231,   232,
     233,   234,   235,   297,   298,    78,    23,    24,    79,    80,
     304,    22,    16,   280,    23,    24,    18,   167,   167,   203,
     203,   236,   222,    25,   313,   200,    64,   246,    65,    26,
      19,    81,   101,   318,    39,   167,   167,    26,    32,    33,
     203,    85,    86,   224,   225,   226,   227,   167,   228,   229,
     230,   231,   232,   233,   234,   235,   167,   224,   225,   226,
     227,    34,   228,   229,   230,   231,   232,   233,   234,   235,
     224,   225,   226,   227,    54,   228,   229,   230,   231,   232,
     233,   234,   235,   224,   225,   226,   227,    42,   228,   229,
     230,   231,   232,   233,   234,   235,   224,   225,   226,   227,
     247,   228,   229,   230,   231,   232,   233,   234,   235,   143,
      55,   144,    40,    62,   251,    23,    24,    56,   123,    57,
     186,   124,   125,    63,    25,    74,   143,   260,   144,    76,
     126,   100,    86,   135,    86,   136,    86,   186,    26,    89,
     299,   289,   290,   143,   127,   144,    77,    47,    24,   187,
     188,   189,   190,   300,   186,   191,   192,   193,   194,   195,
     143,    90,   144,   196,   257,    95,   187,   188,   189,   190,
      26,   186,   191,   192,   193,   194,   195,   143,    98,   144,
     196,   302,    99,   187,   188,   189,   190,   103,   186,   191,
     192,   193,   194,   195,   226,   227,   110,   196,   305,   111,
     187,   188,   189,   190,   315,   290,   191,   192,   193,   194,
     195,    68,    69,   115,   196,   321,   117,   187,   188,   189,
     190,   316,   290,   191,   192,   193,   194,   195,   143,   116,
     144,   196,   137,   140,    70,    23,    24,   143,   132,   144,
     141,   145,   309,   146,   147,   168,   148,   149,   150,   310,
     145,   170,   146,   147,   171,   148,   149,   150,    26,   151,
     152,   153,   324,   290,   172,   175,   176,   181,   151,   152,
     153,   177,   154,   143,   155,   144,   178,   182,   156,   185,
     183,   184,   199,   155,   201,   206,   145,   156,   146,   147,
     212,   148,   149,   150,   143,   217,   144,   200,   218,   209,
     219,   220,   237,   238,   151,   152,   153,   145,   221,   146,
     147,   240,   148,   149,   150,   143,   311,   144,   241,   155,
     243,   248,   249,   156,   252,   151,   152,   153,   145,   253,
     146,   147,   275,   148,   149,   150,   323,   143,   255,   144,
     155,   256,   279,   273,   156,   280,   151,   152,   153,   283,
     145,   284,   146,   147,   288,   148,   149,   150,   143,   291,
     144,   155,   292,   301,   307,   156,   308,   312,   151,   152,
     153,   145,   314,   213,   147,   319,   148,   149,   150,   320,
     325,   138,    12,   155,   124,   125,    21,   156,   102,   151,
     152,   153,    41,   126,   122,    75,   202,    97,   139,   317,
       0,     0,     0,     0,   155,     0,     0,   127,   156,   224,
     225,   226,   227,     0,   228,   229,   230,   231,   232,   233,
     234,   235,   224,   225,   226,   227,     0,   228,   229,   230,
     231,   232,   233,   234,   235,     0,   224,   225,   226,   227,
     276,   228,   229,   230,   231,   232,   233,   224,   225,   226,
     227,     0,    -1,    -1,    -1,    -1,    -1,    -1
};

static const yytype_int16 yycheck[] =
{
     132,   147,   148,    56,   154,   134,   119,    77,    31,    50,
     158,    42,    34,    42,    55,   147,   148,    42,    34,    43,
       5,     5,     5,     5,     0,    34,   158,   177,   178,    60,
      59,   179,   180,   287,   184,    60,   186,   291,    60,    61,
       5,   111,     7,   113,    60,    61,   196,   179,   180,    58,
     200,   183,    61,    55,   204,    78,     3,     4,     5,     6,
     314,     8,     9,    10,    11,    12,    13,    14,    15,    92,
      55,    55,    55,    55,   224,   225,   226,   227,   228,   229,
     230,   231,   232,   233,   234,   235,    51,   219,    34,    17,
      55,    34,    59,    44,   207,    62,    49,    34,   248,   249,
       3,     4,     5,     6,    55,     8,     9,    10,    11,    12,
      13,    14,    15,    59,    42,   244,    63,    55,    61,   172,
       3,     4,     5,     6,    61,     8,     9,    10,    11,    12,
      13,    14,    15,   279,   280,    29,    32,    33,    32,    33,
     290,    29,    28,    23,    32,    33,    57,   279,   280,   297,
     298,    34,    59,    41,    34,    62,    51,    60,    53,    55,
      57,    55,    58,   309,    55,   297,   298,    55,    32,    33,
     318,    60,    61,     3,     4,     5,     6,   309,     8,     9,
      10,    11,    12,    13,    14,    15,   318,     3,     4,     5,
       6,    55,     8,     9,    10,    11,    12,    13,    14,    15,
       3,     4,     5,     6,    55,     8,     9,    10,    11,    12,
      13,    14,    15,     3,     4,     5,     6,    34,     8,     9,
      10,    11,    12,    13,    14,    15,     3,     4,     5,     6,
      60,     8,     9,    10,    11,    12,    13,    14,    15,     5,
      28,     7,    29,    39,    60,    32,    33,    59,    29,    55,
      16,    32,    33,    55,    41,    53,     5,    60,     7,    59,
      41,    60,    61,    60,    61,    60,    61,    16,    55,    63,
      60,    60,    61,     5,    55,     7,    59,    32,    33,    45,
      46,    47,    48,    60,    16,    51,    52,    53,    54,    55,
       5,    60,     7,    59,    60,    55,    45,    46,    47,    48,
      55,    16,    51,    52,    53,    54,    55,     5,    55,     7,
      59,    60,    57,    45,    46,    47,    48,    55,    16,    51,
      52,    53,    54,    55,     5,     6,    57,    59,    60,    59,
      45,    46,    47,    48,    60,    61,    51,    52,    53,    54,
      55,    32,    33,    55,    59,    60,    28,    45,    46,    47,
      48,    60,    61,    51,    52,    53,    54,    55,     5,    61,
       7,    59,    55,    34,    55,    32,    33,     5,    30,     7,
      55,    18,    19,    20,    21,    58,    23,    24,    25,    26,
      18,    57,    20,    21,    34,    23,    24,    25,    55,    36,
      37,    38,    60,    61,    59,    55,    55,    34,    36,    37,
      38,    59,    40,     5,    51,     7,    59,    34,    55,    34,
      59,    59,    42,    51,    31,    58,    18,    55,    20,    21,
      22,    23,    24,    25,     5,    59,     7,    62,    59,    55,
      59,    42,    42,    55,    36,    37,    38,    18,    59,    20,
      21,    34,    23,    24,    25,     5,    27,     7,    31,    51,
      55,    59,    59,    55,    60,    36,    37,    38,    18,    60,
      20,    21,    34,    23,    24,    25,    26,     5,    55,     7,
      51,    55,    35,    55,    55,    23,    36,    37,    38,    34,
      18,    34,    20,    21,    60,    23,    24,    25,     5,    59,
       7,    51,    58,    55,    55,    55,    61,    34,    36,    37,
      38,    18,    59,    20,    21,    34,    23,    24,    25,    34,
      34,    29,     5,    51,    32,    33,    14,    55,    92,    36,
      37,    38,    27,    41,   116,    61,   158,    86,   128,   308,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    55,    55,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    13,
      14,    15,     3,     4,     5,     6,    -1,     8,     9,    10,
      11,    12,    13,    14,    15,    -1,     3,     4,     5,     6,
      34,     8,     9,    10,    11,    12,    13,     3,     4,     5,
       6,    -1,     8,     9,    10,    11,    12,    13
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_int8 yystos[] =
{
       0,    43,    65,    66,    55,    67,    68,     0,    49,    70,
      69,    44,    68,    55,    71,    72,    28,    84,    57,    57,
      50,    72,    29,    32,    33,    41,    55,    85,    86,    87,
     109,    96,    32,    33,    55,    91,    92,    93,    73,    55,
      29,    86,    34,     5,    55,    94,    95,    32,    82,   109,
     116,    34,    58,    61,    55,    28,    59,    55,    59,    62,
      34,    61,    39,    55,    92,    92,    74,    91,    32,    33,
      55,    98,    99,   110,    53,    95,    59,    59,    29,    32,
      33,    55,    75,    77,    60,    60,    61,     5,    55,    63,
      60,    98,    79,    80,   109,    55,    88,    99,    55,    57,
      60,    58,    80,    55,    76,     5,    55,    89,    90,    83,
      57,    59,    34,    59,    78,    55,    61,    28,   101,   117,
      98,    98,    90,    29,    32,    33,    41,    55,   102,   103,
     104,   111,    30,    97,   101,    60,    60,    55,    29,   103,
      34,    55,   108,     5,     7,    18,    20,    21,    23,    24,
      25,    36,    37,    38,    40,    51,    55,   100,   114,   115,
     118,   119,   120,   121,   122,   123,   124,   125,    58,    97,
      57,    34,    59,    34,    61,    55,    55,    59,    59,   114,
     114,    34,    34,    59,    59,    34,    16,    45,    46,    47,
      48,    51,    52,    53,    54,    55,    59,   112,   125,    42,
      62,    31,   100,   115,    17,    42,    58,    81,    91,    55,
     112,   112,    22,    20,   125,   112,   112,    59,    59,    59,
      42,    59,    59,   112,     3,     4,     5,     6,     8,     9,
      10,    11,    12,    13,    14,    15,    34,    42,    55,   112,
      34,    31,   112,    55,   101,    60,    60,    60,    59,    59,
      60,    60,    60,    60,   125,    55,    55,    60,   112,   113,
      60,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,    55,    63,    34,    34,    97,   105,    35,
      23,   112,   112,    34,    34,    60,    42,    59,    60,    60,
      61,    59,    58,     5,    55,   106,   107,   114,   114,    60,
      60,    55,    60,   113,   112,    60,   113,    55,    61,    19,
      26,    27,    34,    34,    59,    60,    60,   107,   114,    34,
      34,    60,   113,    26,    60,    34
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr1[] =
{
       0,    64,    65,    66,    66,    67,    67,    69,    68,    70,
      70,    71,    71,    73,    72,    74,    74,    76,    75,    77,
      77,    77,    78,    78,    79,    79,    79,    81,    80,    83,
      82,    84,    84,    84,    85,    85,    86,    86,    88,    87,
      89,    89,    90,    90,    91,    91,    91,    91,    92,    93,
      93,    93,    94,    94,    95,    95,    95,    95,    96,    96,
      97,    97,    97,    97,    98,    98,    98,    99,    99,   100,
     101,   101,   102,   102,   103,   103,   105,   104,   106,   106,
     107,   107,   108,   108,   109,   109,   109,   110,   110,   110,
     111,   111,   111,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   112,   112,   112,   112,   112,   112,   112,
     112,   112,   112,   113,   113,   114,   114,   115,   115,   115,
     115,   115,   115,   115,   115,   115,   115,   117,   116,   118,
     118,   119,   120,   121,   122,   123,   124,   125,   125,   125,
     125,   125,   125
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     5,     3,     0,     2,     1,     0,     5,     3,
       0,     2,     1,     0,     8,     2,     0,     0,     4,     1,
       1,     1,     1,     4,     2,     1,     0,     0,    10,     0,
       9,     3,     2,     0,     2,     1,     3,     2,     0,     7,
       3,     1,     1,     2,     3,     3,     2,     1,     2,     1,
       1,     1,     3,     1,     1,     4,     4,     2,     2,     0,
       5,     4,     4,     3,     3,     1,     0,     2,     3,     3,
       3,     2,     2,     1,     3,     2,     0,     7,     3,     1,
       1,     2,     3,     1,     1,     1,     1,     1,     1,     1,
       1,     1,     1,     3,     3,     3,     3,     3,     3,     2,
       3,     3,     3,     3,     3,     3,     3,     1,     1,     1,
       1,     3,     4,     3,     4,     3,     4,     5,     6,     5,
       6,     7,     8,     3,     1,     2,     1,     1,     1,     1,
       1,     1,     1,     1,     2,     2,     2,     0,    10,    10,
       8,     8,     7,     7,     5,     5,     4,     3,     1,     4,
       2,     2,     3
};


enum { YYENOMEM = -2 };

#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYNOMEM         goto yyexhaustedlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                    \
  do                                                              \
    if (yychar == YYEMPTY)                                        \
      {                                                           \
        yychar = (Token);                                         \
        yylval = (Value);                                         \
        YYPOPSTACK (yylen);                                       \
        yystate = *yyssp;                                         \
        goto yybackup;                                            \
      }                                                           \
    else                                                          \
      {                                                           \
        yyerror (YY_("syntax error: cannot back up")); \
        YYERROR;                                                  \
      }                                                           \
  while (0)

/* Backward compatibility with an undocumented macro.
   Use YYerror or YYUNDEF. */
#define YYERRCODE YYUNDEF


/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)




# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Kind, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo,
                       yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YY_USE (yyoutput);
  if (!yyvaluep)
    return;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo,
                 yysymbol_kind_t yykind, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yykind < YYNTOKENS ? "token" : "nterm", yysymbol_name (yykind));

  yy_symbol_value_print (yyo, yykind, yyvaluep);
  YYFPRINTF (yyo, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yy_state_t *yybottom, yy_state_t *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp,
                 int yyrule)
{
  int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %d):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       YY_ACCESSING_SYMBOL (+yyssp[yyi + 1 - yynrhs]),
                       &yyvsp[(yyi + 1) - (yynrhs)]);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args) ((void) 0)
# define YY_SYMBOL_PRINT(Title, Kind, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif






/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg,
            yysymbol_kind_t yykind, YYSTYPE *yyvaluep)
{
  YY_USE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yykind, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YY_USE (yykind);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/* Lookahead token kind.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;




/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    yy_state_fast_t yystate = 0;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus = 0;

    /* Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* Their size.  */
    YYPTRDIFF_T yystacksize = YYINITDEPTH;

    /* The state stack: array, bottom, top.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss = yyssa;
    yy_state_t *yyssp = yyss;

    /* The semantic value stack: array, bottom, top.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs = yyvsa;
    YYSTYPE *yyvsp = yyvs;

  int yyn;
  /* The return value of yyparse.  */
  int yyresult;
  /* Lookahead symbol kind.  */
  yysymbol_kind_t yytoken = YYSYMBOL_YYEMPTY;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;



#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yychar = YYEMPTY; /* Cause a token to be read.  */

  goto yysetstate;


/*------------------------------------------------------------.
| yynewstate -- push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;


/*--------------------------------------------------------------------.
| yysetstate -- set current state (the top of the stack) to yystate.  |
`--------------------------------------------------------------------*/
yysetstate:
  YYDPRINTF ((stderr, "Entering state %d\n", yystate));
  YY_ASSERT (0 <= yystate && yystate < YYNSTATES);
  YY_IGNORE_USELESS_CAST_BEGIN
  *yyssp = YY_CAST (yy_state_t, yystate);
  YY_IGNORE_USELESS_CAST_END
  YY_STACK_PRINT (yyss, yyssp);

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    YYNOMEM;
#else
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYPTRDIFF_T yysize = yyssp - yyss + 1;

# if defined yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        yy_state_t *yyss1 = yyss;
        YYSTYPE *yyvs1 = yyvs;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * YYSIZEOF (*yyssp),
                    &yyvs1, yysize * YYSIZEOF (*yyvsp),
                    &yystacksize);
        yyss = yyss1;
        yyvs = yyvs1;
      }
# else /* defined YYSTACK_RELOCATE */
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        YYNOMEM;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          YYNOMEM;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YY_IGNORE_USELESS_CAST_BEGIN
      YYDPRINTF ((stderr, "Stack size increased to %ld\n",
                  YY_CAST (long, yystacksize)));
      YY_IGNORE_USELESS_CAST_END

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }
#endif /* !defined yyoverflow && !defined YYSTACK_RELOCATE */


  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;


/*-----------.
| yybackup.  |
`-----------*/
yybackup:
  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either empty, or end-of-input, or a valid lookahead.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token\n"));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = YYEOF;
      yytoken = YYSYMBOL_YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else if (yychar == YYerror)
    {
      /* The scanner already issued an error message, process directly
         to error recovery.  But do not keep the error token as
         lookahead, it is too special and may lead us to an endless
         loop in error recovery. */
      yychar = YYUNDEF;
      yytoken = YYSYMBOL_YYerror;
      goto yyerrlab1;
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);
  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  /* Discard the shifted token.  */
  yychar = YYEMPTY;
  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
  case 2: /* program: TypeDefBlock ClassDefBlock GDeclBlock FDefBlock MainBlock  */
#line 150 "parser.y"
                                                                          { }
#line 1737 "y.tab.c"
    break;

  case 3: /* TypeDefBlock: TYPE TypeDefList ENDTYPE  */
#line 155 "parser.y"
                                             {}
#line 1743 "y.tab.c"
    break;

  case 4: /* TypeDefBlock: %empty  */
#line 156 "parser.y"
                                                   {}
#line 1749 "y.tab.c"
    break;

  case 5: /* TypeDefList: TypeDefList TypeDef  */
#line 159 "parser.y"
                                             {}
#line 1755 "y.tab.c"
    break;

  case 6: /* TypeDefList: TypeDef  */
#line 160 "parser.y"
                                             {}
#line 1761 "y.tab.c"
    break;

  case 7: /* $@1: %empty  */
#line 164 "parser.y"
                {
                    /* Install a placeholder first so a type may contain a field
                       of its own type (for example, List next;). */
                    if (TLookup(TTableHead, (yyvsp[0].node)->varname) != NULL) {
                        fprintf(stderr, "Error: Type '%s' redeclared at line %d\\n",
                                (yyvsp[0].node)->varname, yylineNum);
                        exit(1);
                    }
                    TTableHead = TInstall(TTableHead, (yyvsp[0].node)->varname, 0, NULL);
                    currTupleType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                }
#line 1777 "y.tab.c"
    break;

  case 8: /* TypeDef: ID $@1 '{' FieldDefList '}'  */
#line 175 "parser.y"
                                     {
                    int fieldCount = 0;
                    struct Fieldlist *temp = (yyvsp[-1].flist);
                    for (; temp != NULL; temp = temp->next) {
                        fieldCount++;
                    }
                    if (fieldCount > 8) {
                        fprintf(stderr, "Error: Too many member fields in type '%s' (max 8)\\n",
                                (yyvsp[-4].node)->varname);
                        exit(1);
                    }
                    currTupleType->fields = (yyvsp[-1].flist);
                    currTupleType->size = fieldCount;
                    currTupleType = NULL;
                }
#line 1797 "y.tab.c"
    break;

  case 9: /* ClassDefBlock: CLASS ClassDefList ENDCLASS  */
#line 194 "parser.y"
                                            { }
#line 1803 "y.tab.c"
    break;

  case 10: /* ClassDefBlock: %empty  */
#line 195 "parser.y"
                                          { }
#line 1809 "y.tab.c"
    break;

  case 11: /* ClassDefList: ClassDefList ClassDef  */
#line 198 "parser.y"
                                      { }
#line 1815 "y.tab.c"
    break;

  case 12: /* ClassDefList: ClassDef  */
#line 199 "parser.y"
                                      { }
#line 1821 "y.tab.c"
    break;

  case 13: /* $@2: %empty  */
#line 202 "parser.y"
                       {
                              if (TLookup(TTableHead, (yyvsp[-1].node)->varname)) {
                                  fprintf(stderr, "Error: Class '%s' conflicts with an existing user-defined type at line %d\n", (yyvsp[-1].node)->varname, yylineNum);
                                  exit(1);
                              }
                              CTableHead = CInstall(CTableHead, (yyvsp[-1].node)->varname, NULL);
                              currentClass = CLookup(CTableHead, (yyvsp[-1].node)->varname);
                              hasClassDefinitions = 1;
                              seenMethodDecl = 0;
                         }
#line 1836 "y.tab.c"
    break;

  case 14: /* ClassDef: ID '{' $@2 DECL ClassMemberList ENDDECL MethodDefList '}'  */
#line 213 "parser.y"
                         {
                              for (struct MemberFunc *m = currentClass->methods; m; m = m->next) {
                                  if (!m->isDefined) {
                                      fprintf(stderr, "Error: Method '%s' declared but not defined in class '%s'\n", m->name, currentClass->name);
                                      exit(1);
                                  }
                              }
                              currentClass = NULL; currentMethod = NULL;
                         }
#line 1850 "y.tab.c"
    break;

  case 15: /* ClassMemberList: ClassMemberList ClassMember  */
#line 224 "parser.y"
                                              { }
#line 1856 "y.tab.c"
    break;

  case 16: /* ClassMemberList: %empty  */
#line 225 "parser.y"
                                             { }
#line 1862 "y.tab.c"
    break;

  case 17: /* $@3: %empty  */
#line 228 "parser.y"
                                     {
                              pendingMemberName = strdup((yyvsp[0].node)->varname);
                         }
#line 1870 "y.tab.c"
    break;

  case 19: /* ClassMemberType: INT  */
#line 233 "parser.y"
                      { currentType = T_INT; currentClassType = NULL; }
#line 1876 "y.tab.c"
    break;

  case 20: /* ClassMemberType: STR  */
#line 234 "parser.y"
                      { currentType = T_STR; currentClassType = NULL; }
#line 1882 "y.tab.c"
    break;

  case 21: /* ClassMemberType: ID  */
#line 235 "parser.y"
                      {
                          currentType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                          currentClassType = NULL;
                          if (!currentType) currentClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                          if (currentClassType == currentClass) stage7Error("A class cannot contain a field of its own class type in Stage 7");
                          if (!currentType && !currentClassType) {
                              fprintf(stderr, "Error: Undefined member type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                              exit(1);
                          }
                      }
#line 1897 "y.tab.c"
    break;

  case 22: /* ClassMemberTail: SEMICOLON  */
#line 247 "parser.y"
                            {
                              if (seenMethodDecl) stage7Error("Class fields must be declared before methods");
                              Class_Finstall(currentClass,
                                  currentClassType ? currentClassType->name : (currentType ? currentType->name : ""),
                                  pendingMemberName, currentType, currentClassType);
                         }
#line 1908 "y.tab.c"
    break;

  case 23: /* ClassMemberTail: '(' ParamList ')' SEMICOLON  */
#line 253 "parser.y"
                                              {
                              if (currentClassType != NULL || currentType == NULL)
                                  stage7Error("Class methods must have a non-class return type");
                              seenMethodDecl = 1;
                              Class_Minstall(currentClass, pendingMemberName, currentType, (yyvsp[-2].paramList), methodFlabel++);
                         }
#line 1919 "y.tab.c"
    break;

  case 24: /* MethodDefList: MethodDefList MethodDef  */
#line 261 "parser.y"
                                        { }
#line 1925 "y.tab.c"
    break;

  case 25: /* MethodDefList: MethodDef  */
#line 262 "parser.y"
                                         { }
#line 1931 "y.tab.c"
    break;

  case 26: /* MethodDefList: %empty  */
#line 263 "parser.y"
                                         { }
#line 1937 "y.tab.c"
    break;

  case 27: /* $@4: %empty  */
#line 266 "parser.y"
                                               {
                         if (currentClassType) stage7Error("Class methods cannot return class objects in Stage 7 Task 1");
                         struct MemberFunc *m = Class_Mlookup(currentClass, (yyvsp[-4].node)->varname);
                         if (!m) {
                             fprintf(stderr, "Error: Definition of undeclared method '%s' in class '%s' at line %d\n", (yyvsp[-4].node)->varname, currentClass->name, yylineNum);
                             exit(1);
                         }
                         if (!sameMethodSignature(m, currentType, (yyvsp[-2].paramList))) {
                             fprintf(stderr, "Error: Signature mismatch for method '%s' in class '%s' at line %d\n", (yyvsp[-4].node)->varname, currentClass->name, yylineNum);
                             exit(1);
                         }
                         if (m->isDefined) stage7Error("Method defined more than once");
                         m->isDefined = 1;
                         currentMethod = m;
                         currentFunc = createSymbolNode((yyvsp[-4].node)->varname, currentType, false, 0, m->flabel);
                         currentFunc->paramList = (yyvsp[-2].paramList);
                         Lhead = createLSTFromParams((yyvsp[-2].paramList));
                         /* The caller pushes self before the explicit arguments. */
                         Lhead = insertToLocalTable(Lhead, "self", T_VOID, false, 1);
                         findLocalSymbol(Lhead, "self")->binding = -countParameterList((yyvsp[-2].paramList)) - 3;
                         localBinding = 1;
                     }
#line 1964 "y.tab.c"
    break;

  case 28: /* MethodDef: GType ID '(' ParamList ')' '{' $@4 LdeclBlock Body '}'  */
#line 288 "parser.y"
                                    {
                         currentFunc->bodyAST = (yyvsp[-1].node);
                         currentFunc->Lentry = Lhead;
                         funcCodeGen(currentFunc, output);
                         currentFunc = NULL;
                         currentMethod = NULL;
                         Lhead = NULL;
                     }
#line 1977 "y.tab.c"
    break;

  case 29: /* $@5: %empty  */
#line 300 "parser.y"
                                                {   
                                                    Ghead = insertTable(Ghead,"F0",T_INT,false,0,0); 
                                                    currentFunc = find(Ghead, "F0");
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", "main");
                                                        exit(1);
                                                    }
                                                }
#line 1990 "y.tab.c"
    break;

  case 30: /* MainBlock: INT MAIN '(' ')' '{' $@5 LdeclBlock Body '}'  */
#line 308 "parser.y"
                                    {
                                        currentFunc->bodyAST = (yyvsp[-1].node);
                                        currentFunc->Lentry = Lhead;

                                        printf("\nAST:\n");
                                        printAST(currentFunc->bodyAST, 0);

                                        funcCodeGen(currentFunc, output);
                                        currentFunc = NULL;
                                        Lhead = NULL;
                                    }
#line 2006 "y.tab.c"
    break;

  case 31: /* GDeclBlock: DECL GDeclList ENDDECL  */
#line 323 "parser.y"
                                            {}
#line 2012 "y.tab.c"
    break;

  case 32: /* GDeclBlock: DECL ENDDECL  */
#line 324 "parser.y"
                                            {}
#line 2018 "y.tab.c"
    break;

  case 33: /* GDeclBlock: %empty  */
#line 325 "parser.y"
                                             {}
#line 2024 "y.tab.c"
    break;

  case 34: /* GDeclList: GDeclList GDecl  */
#line 328 "parser.y"
                                            {}
#line 2030 "y.tab.c"
    break;

  case 35: /* GDeclList: GDecl  */
#line 329 "parser.y"
                                            {}
#line 2036 "y.tab.c"
    break;

  case 36: /* GDecl: GType GidList SEMICOLON  */
#line 332 "parser.y"
                                            {}
#line 2042 "y.tab.c"
    break;

  case 37: /* GDecl: GTupleDecl SEMICOLON  */
#line 333 "parser.y"
                                            {}
#line 2048 "y.tab.c"
    break;

  case 38: /* $@6: %empty  */
#line 336 "parser.y"
                                                {
                                                    int size = 0;
                                                    struct Fieldlist * temp = (yyvsp[-1].flist) ;
                                                    while(temp) {
                                                        size += GetSize(temp->type);
                                                        temp = temp->next ;
                                                    }
                                                    TTableHead = TInstall(TTableHead,(yyvsp[-3].node)->varname,size,(yyvsp[-1].flist));
                                                    currTupleType = TLookup(TTableHead, (yyvsp[-3].node)->varname);

                                                    if(currTupleType == NULL) {
                                                        printf("Tuple Type installation failed\n");
                                                        exit(1);
                                                    }
                                                }
#line 2068 "y.tab.c"
    break;

  case 39: /* GTupleDecl: TUPLE ID '(' FieldDefList ')' $@6 GTupleIdList  */
#line 351 "parser.y"
                                                {   currTupleType = NULL ;}
#line 2074 "y.tab.c"
    break;

  case 40: /* GTupleIdList: GTupleIdList ',' GTupleId  */
#line 354 "parser.y"
                                               {}
#line 2080 "y.tab.c"
    break;

  case 41: /* GTupleIdList: GTupleId  */
#line 355 "parser.y"
                                               {}
#line 2086 "y.tab.c"
    break;

  case 42: /* GTupleId: ID  */
#line 358 "parser.y"
                                            {    Ghead = insertTable(Ghead, ((yyvsp[0].node))->varname, currTupleType, false, currTupleType->size, -1);}
#line 2092 "y.tab.c"
    break;

  case 43: /* GTupleId: STAR ID  */
#line 359 "parser.y"
                                            {    Ghead = insertTable(Ghead, ((yyvsp[0].node))->varname, currTupleType, true, currTupleType->size, -1);}
#line 2098 "y.tab.c"
    break;

  case 44: /* FieldDefList: FieldDefList ',' FieldDef  */
#line 364 "parser.y"
      {
          struct Fieldlist *temp = (yyvsp[-2].flist);

          while (temp->next)
              temp = temp->next;

          (yyvsp[0].flist)->fieldIndex = temp->fieldIndex + 1;
          temp->next = (yyvsp[0].flist);
          (yyval.flist) = (yyvsp[-2].flist);
      }
#line 2113 "y.tab.c"
    break;

  case 45: /* FieldDefList: FieldDefList SEMICOLON FieldDef  */
#line 375 "parser.y"
      {
          struct Fieldlist *temp = (yyvsp[-2].flist);

          while (temp->next)
              temp = temp->next;

          (yyvsp[0].flist)->fieldIndex = temp->fieldIndex + 1;
          temp->next = (yyvsp[0].flist);
          (yyval.flist) = (yyvsp[-2].flist);
      }
#line 2128 "y.tab.c"
    break;

  case 46: /* FieldDefList: FieldDefList SEMICOLON  */
#line 386 "parser.y"
      {
          (yyval.flist) = (yyvsp[-1].flist);
      }
#line 2136 "y.tab.c"
    break;

  case 47: /* FieldDefList: FieldDef  */
#line 390 "parser.y"
      {
          (yyvsp[0].flist)->fieldIndex = 0;
          (yyval.flist) = (yyvsp[0].flist);
      }
#line 2145 "y.tab.c"
    break;

  case 48: /* FieldDef: FieldType ID  */
#line 396 "parser.y"
                                            {   (yyval.flist) = createFieldlistNode(((yyvsp[0].node))->varname,currentType,0); }
#line 2151 "y.tab.c"
    break;

  case 49: /* FieldType: INT  */
#line 400 "parser.y"
                                            {   currentType = T_INT ;}
#line 2157 "y.tab.c"
    break;

  case 50: /* FieldType: STR  */
#line 401 "parser.y"
                                            {   currentType = T_STR ;}
#line 2163 "y.tab.c"
    break;

  case 51: /* FieldType: ID  */
#line 402 "parser.y"
                                            {   
                                                struct TypeTable * udt = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                if(udt == NULL) {
                                                    printf("Error: Undefined type '%s' in field definition at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                                currentType = udt ;
                                            }
#line 2176 "y.tab.c"
    break;

  case 52: /* GidList: GidList ',' Gid  */
#line 412 "parser.y"
                                            {}
#line 2182 "y.tab.c"
    break;

  case 53: /* GidList: Gid  */
#line 413 "parser.y"
                                            {}
#line 2188 "y.tab.c"
    break;

  case 54: /* Gid: ID  */
#line 416 "parser.y"
                                            {
                                                Ghead = insertTable(Ghead,((yyvsp[0].node))->varname,currentType,false,1,-1);
                                                if (currentClassType) find(Ghead,((yyvsp[0].node))->varname)->Ctype = currentClassType;
                                            }
#line 2197 "y.tab.c"
    break;

  case 55: /* Gid: ID '[' NUM ']'  */
#line 420 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Arrays of class objects are not supported");
                                                Ghead = insertTable(Ghead,((yyvsp[-3].node))->varname,currentType,false,(yyvsp[-1].number),-1);
                                            }
#line 2206 "y.tab.c"
    break;

  case 56: /* Gid: ID '(' ParamList ')'  */
#line 424 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Functions cannot return class objects in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,((yyvsp[-3].node))->varname,currentType,false,0,flabel++);
                                                insertParamListToGST(Ghead,((yyvsp[-3].node))->varname,(yyvsp[-1].paramList));
                                            }
#line 2216 "y.tab.c"
    break;

  case 57: /* Gid: STAR ID  */
#line 429 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Pointers to class objects are not supported in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,((yyvsp[0].node))->varname,currentType == T_INT ? T_INT_PTR : T_STR_PTR,true,1,-1);
                                            }
#line 2225 "y.tab.c"
    break;

  case 58: /* FDefBlock: FDefBlock Fdef  */
#line 435 "parser.y"
                           {}
#line 2231 "y.tab.c"
    break;

  case 59: /* FDefBlock: %empty  */
#line 436 "parser.y"
               {}
#line 2237 "y.tab.c"
    break;

  case 60: /* Body: BEG stmtList Ret END SEMICOLON  */
#line 439 "parser.y"
                                                {
                                                    (yyval.node) = connectNode((yyvsp[-3].node),(yyvsp[-2].node));
                                                }
#line 2245 "y.tab.c"
    break;

  case 61: /* Body: BEG Ret END SEMICOLON  */
#line 442 "parser.y"
                                                {
                                                    (yyval.node) = (yyvsp[-2].node);
                                                }
#line 2253 "y.tab.c"
    break;

  case 62: /* Body: BEG stmtList Ret END  */
#line 445 "parser.y"
                                                {
                                                    (yyval.node) = connectNode((yyvsp[-2].node),(yyvsp[-1].node));
                                                }
#line 2261 "y.tab.c"
    break;

  case 63: /* Body: BEG Ret END  */
#line 448 "parser.y"
                                               {
                                                    (yyval.node) = (yyvsp[-1].node);
                                                }
#line 2269 "y.tab.c"
    break;

  case 64: /* ParamList: ParamList ',' Param  */
#line 453 "parser.y"
                                            {   (yyval.paramList) = insertToParamList((yyvsp[-2].paramList),((yyvsp[0].paramList))->name,((yyvsp[0].paramList))->type,((yyvsp[0].paramList))->isPtr,((yyvsp[0].paramList))->Ctype) ; }
#line 2275 "y.tab.c"
    break;

  case 65: /* ParamList: Param  */
#line 454 "parser.y"
                                            {   (yyval.paramList) = insertToParamList(NULL,((yyvsp[0].paramList))->name,((yyvsp[0].paramList))->type,((yyvsp[0].paramList))->isPtr,((yyvsp[0].paramList))->Ctype) ;}
#line 2281 "y.tab.c"
    break;

  case 66: /* ParamList: %empty  */
#line 455 "parser.y"
                                            {   (yyval.paramList) = NULL ;}
#line 2287 "y.tab.c"
    break;

  case 67: /* Param: ParType ID  */
#line 458 "parser.y"
                                            {   (yyval.paramList) = createParamStructNode(strdup((yyvsp[0].node)->varname),paramType,false,paramClassType); }
#line 2293 "y.tab.c"
    break;

  case 68: /* Param: ParType STAR ID  */
#line 459 "parser.y"
                                            {   (yyval.paramList) = createParamStructNode(strdup((yyvsp[0].node)->varname),paramType == T_INT ? T_INT_PTR : T_STR_PTR,true,NULL); }
#line 2299 "y.tab.c"
    break;

  case 69: /* Ret: RETURN EXPR SEMICOLON  */
#line 462 "parser.y"
                                            {   
                                                (yyval.node) = returnNode(currentFunc,RETURN_NODE,(yyvsp[-1].node)); 
                                            }
#line 2307 "y.tab.c"
    break;

  case 70: /* LdeclBlock: DECL LDeclList ENDDECL  */
#line 466 "parser.y"
                                            {   localBinding = 1; }
#line 2313 "y.tab.c"
    break;

  case 71: /* LdeclBlock: DECL ENDDECL  */
#line 467 "parser.y"
                                            {}
#line 2319 "y.tab.c"
    break;

  case 72: /* LDeclList: LDeclList LDecl  */
#line 470 "parser.y"
                                            {}
#line 2325 "y.tab.c"
    break;

  case 73: /* LDeclList: LDecl  */
#line 471 "parser.y"
                                            {}
#line 2331 "y.tab.c"
    break;

  case 74: /* LDecl: LType IdList SEMICOLON  */
#line 474 "parser.y"
                                            {}
#line 2337 "y.tab.c"
    break;

  case 75: /* LDecl: LTupleDecl SEMICOLON  */
#line 475 "parser.y"
                                            {}
#line 2343 "y.tab.c"
    break;

  case 76: /* $@7: %empty  */
#line 478 "parser.y"
                                                {
                                                    int size = 0;
                                                    struct Fieldlist *temp = (yyvsp[-1].flist);
                                                    while(temp) {
                                                        size += GetSize(temp->type);
                                                        temp = temp->next;
                                                    }
                                                    TTableHead = TInstall(TTableHead, (yyvsp[-3].node)->varname, size, (yyvsp[-1].flist));
                                                    currTupleType = TLookup(TTableHead, (yyvsp[-3].node)->varname);
                                                    if (currTupleType == NULL) {
                                                        yyerror("Tuple type installation failed");
                                                        exit(1);
                                                    }
                                                }
#line 2362 "y.tab.c"
    break;

  case 77: /* LTupleDecl: TUPLE ID '(' FieldDefList ')' $@7 LTupleIdList  */
#line 492 "parser.y"
                                                {   currTupleType = NULL ;}
#line 2368 "y.tab.c"
    break;

  case 78: /* LTupleIdList: LTupleIdList ',' LTupleId  */
#line 495 "parser.y"
                                                {}
#line 2374 "y.tab.c"
    break;

  case 79: /* LTupleIdList: LTupleId  */
#line 496 "parser.y"
                                                {}
#line 2380 "y.tab.c"
    break;

  case 80: /* LTupleId: ID  */
#line 499 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead, ((yyvsp[0].node))->varname, currTupleType,false,currTupleType->size);}
#line 2386 "y.tab.c"
    break;

  case 81: /* LTupleId: STAR ID  */
#line 500 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead, ((yyvsp[0].node))->varname, currTupleType,true,1);}
#line 2392 "y.tab.c"
    break;

  case 82: /* IdList: IdList ',' ID  */
#line 504 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead,((yyvsp[0].node))->varname,currLocalType,false,currLocalClassType ? 1 : 1); if(currLocalClassType) findLocalSymbol(Lhead,((yyvsp[0].node))->varname)->Ctype=currLocalClassType;}
#line 2398 "y.tab.c"
    break;

  case 83: /* IdList: ID  */
#line 505 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead,((yyvsp[0].node))->varname,currLocalType,false,1); if(currLocalClassType) findLocalSymbol(Lhead,((yyvsp[0].node))->varname)->Ctype=currLocalClassType;}
#line 2404 "y.tab.c"
    break;

  case 84: /* GType: INT  */
#line 508 "parser.y"
                                            { currentType = T_INT; currentClassType = NULL; }
#line 2410 "y.tab.c"
    break;

  case 85: /* GType: STR  */
#line 509 "parser.y"
                                            { currentType = T_STR; currentClassType = NULL; }
#line 2416 "y.tab.c"
    break;

  case 86: /* GType: ID  */
#line 510 "parser.y"
                                            {
                                                struct TypeTable *udt = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                struct Classtable *ct = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                currentType = udt;
                                                currentClassType = NULL;
                                                if (!udt && ct) currentClassType = ct;
                                                else if (!udt && !ct) {
                                                    fprintf(stderr, "Error: Undefined type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2432 "y.tab.c"
    break;

  case 87: /* ParType: INT  */
#line 523 "parser.y"
                                            { paramType = T_INT; paramClassType = NULL; }
#line 2438 "y.tab.c"
    break;

  case 88: /* ParType: STR  */
#line 524 "parser.y"
                                            { paramType = T_STR; paramClassType = NULL; }
#line 2444 "y.tab.c"
    break;

  case 89: /* ParType: ID  */
#line 525 "parser.y"
                                            {
                                                paramType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                paramClassType = NULL;
                                                if (!paramType) paramClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                if (!paramType && !paramClassType) {
                                                    fprintf(stderr, "Error: Undefined parameter type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2458 "y.tab.c"
    break;

  case 90: /* LType: INT  */
#line 536 "parser.y"
                                            { currLocalType = T_INT; currLocalClassType = NULL; }
#line 2464 "y.tab.c"
    break;

  case 91: /* LType: STR  */
#line 537 "parser.y"
                                            { currLocalType = T_STR; currLocalClassType = NULL; }
#line 2470 "y.tab.c"
    break;

  case 92: /* LType: ID  */
#line 538 "parser.y"
                                            {
                                                currLocalType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                currLocalClassType = NULL;
                                                if (!currLocalType) currLocalClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                if (!currLocalType && !currLocalClassType) {
                                                    fprintf(stderr, "Error: Undefined local type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2484 "y.tab.c"
    break;

  case 93: /* EXPR: EXPR MINUS EXPR  */
#line 550 "parser.y"
                                            {   (yyval.node) = exprNode(MINUS_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2490 "y.tab.c"
    break;

  case 94: /* EXPR: EXPR PLUS EXPR  */
#line 551 "parser.y"
                                            {   (yyval.node) = exprNode(ADD_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2496 "y.tab.c"
    break;

  case 95: /* EXPR: EXPR STAR EXPR  */
#line 552 "parser.y"
                                            {   (yyval.node) = exprNode(MUL_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2502 "y.tab.c"
    break;

  case 96: /* EXPR: EXPR DIV EXPR  */
#line 553 "parser.y"
                                            {   (yyval.node) = exprNode(DIV_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2508 "y.tab.c"
    break;

  case 97: /* EXPR: EXPR AND EXPR  */
#line 554 "parser.y"
                                            {   (yyval.node) = exprNode(AND_NODE,(yyvsp[-2].node), (yyvsp[0].node)); }
#line 2514 "y.tab.c"
    break;

  case 98: /* EXPR: EXPR OR EXPR  */
#line 555 "parser.y"
                                            {   (yyval.node) = exprNode(OR_NODE,(yyvsp[-2].node), (yyvsp[0].node)); }
#line 2520 "y.tab.c"
    break;

  case 99: /* EXPR: NOT EXPR  */
#line 556 "parser.y"
                                            {   (yyval.node) = exprNode(NOT_NODE,(yyvsp[0].node), NULL);}
#line 2526 "y.tab.c"
    break;

  case 100: /* EXPR: EXPR LE EXPR  */
#line 557 "parser.y"
                                            {   (yyval.node) = exprNode(LE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2532 "y.tab.c"
    break;

  case 101: /* EXPR: EXPR LT EXPR  */
#line 558 "parser.y"
                                            {   (yyval.node) = exprNode(LT_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2538 "y.tab.c"
    break;

  case 102: /* EXPR: EXPR GT EXPR  */
#line 559 "parser.y"
                                            {   (yyval.node) = exprNode(GT_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2544 "y.tab.c"
    break;

  case 103: /* EXPR: EXPR GE EXPR  */
#line 560 "parser.y"
                                            {   (yyval.node) = exprNode(GE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2550 "y.tab.c"
    break;

  case 104: /* EXPR: EXPR EQ EXPR  */
#line 561 "parser.y"
                                            {   (yyval.node) = exprNode(EQ_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2556 "y.tab.c"
    break;

  case 105: /* EXPR: EXPR NE EXPR  */
#line 562 "parser.y"
                                            {   (yyval.node) = exprNode(NE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2562 "y.tab.c"
    break;

  case 106: /* EXPR: '(' EXPR ')'  */
#line 563 "parser.y"
                                            {   (yyval.node) = (yyvsp[-1].node) ;}
#line 2568 "y.tab.c"
    break;

  case 107: /* EXPR: NUM  */
#line 564 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,(yyvsp[0].number),T_INT,NULL,NUM_NODE,NULL,NULL,NULL,NULL); }
#line 2574 "y.tab.c"
    break;

  case 108: /* EXPR: STR_LITERAL  */
#line 565 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,NO_VAL,T_STR,(yyvsp[0].string),STR_NODE,NULL,NULL,NULL,NULL); }
#line 2580 "y.tab.c"
    break;

  case 109: /* EXPR: IDENTIFIERS  */
#line 566 "parser.y"
                                            {   (yyval.node) = (yyvsp[0].node) ; }
#line 2586 "y.tab.c"
    break;

  case 110: /* EXPR: NULL_VAL  */
#line 567 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,0,T_VOID,"null",NULL_NODE,NULL,NULL,NULL,NULL); }
#line 2592 "y.tab.c"
    break;

  case 111: /* EXPR: ALLOC '(' ')'  */
#line 568 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_VOID,NULL,ALLOC_NODE,NULL,NULL,NULL,NULL); }
#line 2598 "y.tab.c"
    break;

  case 112: /* EXPR: NEW '(' ID ')'  */
#line 569 "parser.y"
                                            {
                                                struct Classtable *ct = CLookup(CTableHead, (yyvsp[-1].node)->varname);
                                                if (!ct) { fprintf(stderr, "Error: Unknown class '%s' in new at line %d\n", (yyvsp[-1].node)->varname, yylineNum); exit(1); }
                                                (yyval.node) = createTreeNode(0,NO_VAL,T_VOID,strdup((yyvsp[-1].node)->varname),ALLOC_NODE,NULL,NULL,NULL,NULL);
                                                (yyval.node)->Ctype = ct;
                                            }
#line 2609 "y.tab.c"
    break;

  case 113: /* EXPR: INITIALIZE '(' ')'  */
#line 575 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_INT,NULL,INIT_NODE,NULL,NULL,NULL,NULL); }
#line 2615 "y.tab.c"
    break;

  case 114: /* EXPR: FREE '(' IDENTIFIERS ')'  */
#line 576 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_INT,NULL,FREE_NODE,NULL,(yyvsp[-1].node),NULL,NULL); }
#line 2621 "y.tab.c"
    break;

  case 115: /* EXPR: ID '(' ')'  */
#line 577 "parser.y"
                                           {   
                                                (yyval.node) = funcNode(Ghead,(yyvsp[-2].node),NULL);
                                            }
#line 2629 "y.tab.c"
    break;

  case 116: /* EXPR: ID '(' ArgList ')'  */
#line 580 "parser.y"
                                            {   (yyval.node) = funcNode(Ghead,(yyvsp[-3].node),(yyvsp[-1].node));}
#line 2635 "y.tab.c"
    break;

  case 117: /* EXPR: SELF DOT ID '(' ')'  */
#line 581 "parser.y"
                                           {
                                                struct MemberFunc *m = Class_Mlookup(currentClass, (yyvsp[-2].node)->varname);
                                                (yyval.node) = makeMethodCall(m, NULL, makeSelfReceiver());
                                            }
#line 2644 "y.tab.c"
    break;

  case 118: /* EXPR: SELF DOT ID '(' ArgList ')'  */
#line 585 "parser.y"
                                            {
                                                struct MemberFunc *m = Class_Mlookup(currentClass, (yyvsp[-3].node)->varname);
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), makeSelfReceiver());
                                            }
#line 2653 "y.tab.c"
    break;

  case 119: /* EXPR: IDENTIFIERS DOT ID '(' ')'  */
#line 589 "parser.y"
                                            {
                                                /* Receiver may be global, local, or a class-valued parameter. */
                                                struct Classtable *receiverClass = (yyvsp[-4].node)->Ctype;
                                                if (!receiverClass && (yyvsp[-4].node)->Lentry) receiverClass = (yyvsp[-4].node)->Lentry->Ctype;
                                                if (!receiverClass && (yyvsp[-4].node)->Gentry) receiverClass = (yyvsp[-4].node)->Gentry->Ctype;
                                                if (!receiverClass) {
                                                    struct Gsymbol *obj = find(Ghead, (yyvsp[-4].node)->varname);
                                                    if (obj) { receiverClass = obj->Ctype; (yyvsp[-4].node)->Gentry = obj; }
                                                }
                                                if (!receiverClass) stage7Error("Method call receiver is not a declared class object");
                                                (yyvsp[-4].node)->Ctype = receiverClass;
                                                struct MemberFunc *m = Class_Mlookup(receiverClass, (yyvsp[-2].node)->varname);
                                                (yyval.node) = makeMethodCall(m, NULL, (yyvsp[-4].node));
                                            }
#line 2672 "y.tab.c"
    break;

  case 120: /* EXPR: IDENTIFIERS DOT ID '(' ArgList ')'  */
#line 603 "parser.y"
                                                   {
                                                /* Receiver may be global, local, or a class-valued parameter. */
                                                struct Classtable *receiverClass = (yyvsp[-5].node)->Ctype;
                                                if (!receiverClass && (yyvsp[-5].node)->Lentry) receiverClass = (yyvsp[-5].node)->Lentry->Ctype;
                                                if (!receiverClass && (yyvsp[-5].node)->Gentry) receiverClass = (yyvsp[-5].node)->Gentry->Ctype;
                                                if (!receiverClass) {
                                                    struct Gsymbol *obj = find(Ghead, (yyvsp[-5].node)->varname);
                                                    if (obj) { receiverClass = obj->Ctype; (yyvsp[-5].node)->Gentry = obj; }
                                                }
                                                if (!receiverClass) stage7Error("Method call receiver is not a declared class object");
                                                (yyvsp[-5].node)->Ctype = receiverClass;
                                                struct MemberFunc *m = Class_Mlookup(receiverClass, (yyvsp[-3].node)->varname);
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), (yyvsp[-5].node));
                                            }
#line 2691 "y.tab.c"
    break;

  case 121: /* EXPR: SELF DOT ID DOT ID '(' ')'  */
#line 617 "parser.y"
                                           {
                                                struct ClassField *f = Class_Flookup(currentClass, (yyvsp[-4].node)->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = Class_Mlookup(f->ctype, (yyvsp[-2].node)->varname);
                                                (yyval.node) = makeMethodCall(m, NULL, makeSelfFieldReceiver(f));
                                            }
#line 2702 "y.tab.c"
    break;

  case 122: /* EXPR: SELF DOT ID DOT ID '(' ArgList ')'  */
#line 623 "parser.y"
                                                   {
                                                struct ClassField *f = Class_Flookup(currentClass, (yyvsp[-5].node)->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = Class_Mlookup(f->ctype, (yyvsp[-3].node)->varname);
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), makeSelfFieldReceiver(f));
                                            }
#line 2713 "y.tab.c"
    break;

  case 123: /* ArgList: ArgList ',' EXPR  */
#line 631 "parser.y"
                                            {
                                                (yyval.node) = appendArg((yyvsp[-2].node),(yyvsp[0].node));
                                            }
#line 2721 "y.tab.c"
    break;

  case 124: /* ArgList: EXPR  */
#line 634 "parser.y"
                                            {   
                                                (yyval.node) = (yyvsp[0].node);
                                                (yyval.node)->argList = NULL ;
                                            }
#line 2730 "y.tab.c"
    break;

  case 125: /* stmtList: stmtList stmt  */
#line 640 "parser.y"
                                                {   (yyval.node) = connectNode((yyvsp[-1].node),(yyvsp[0].node)); }
#line 2736 "y.tab.c"
    break;

  case 126: /* stmtList: stmt  */
#line 641 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2742 "y.tab.c"
    break;

  case 127: /* stmt: ipStmt  */
#line 644 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2748 "y.tab.c"
    break;

  case 128: /* stmt: opStmt  */
#line 645 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2754 "y.tab.c"
    break;

  case 129: /* stmt: assignStmt  */
#line 646 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2760 "y.tab.c"
    break;

  case 130: /* stmt: ifStmt  */
#line 647 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2766 "y.tab.c"
    break;

  case 131: /* stmt: whileStmt  */
#line 648 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2772 "y.tab.c"
    break;

  case 132: /* stmt: doWhileStmt  */
#line 649 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2778 "y.tab.c"
    break;

  case 133: /* stmt: repeatStmt  */
#line 650 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2784 "y.tab.c"
    break;

  case 134: /* stmt: BREAK SEMICOLON  */
#line 651 "parser.y"
                                                {   (yyval.node) = flowControlNode(BREAK_NODE); }
#line 2790 "y.tab.c"
    break;

  case 135: /* stmt: CONTINUE SEMICOLON  */
#line 652 "parser.y"
                                                {   (yyval.node) = flowControlNode(CONTINUE_NODE); }
#line 2796 "y.tab.c"
    break;

  case 136: /* stmt: EXIT_PR SEMICOLON  */
#line 653 "parser.y"
                                                {   (yyval.node) = exitNode(EXIT_NODE); }
#line 2802 "y.tab.c"
    break;

  case 137: /* $@8: %empty  */
#line 656 "parser.y"
                                                {
                                                    if (currentClassType) stage7Error("Functions cannot return class objects in Stage 7 Task 1");
                                                    currentFunc = find(Ghead, (yyvsp[-4].node)->varname);
                                                    checkDeclAndDefn(currentFunc,currentType);
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", (yyvsp[-4].node)->varname);
                                                        exit(1);
                                                    }
                                                    compareParamList(currentFunc,(yyvsp[-2].paramList)) ;
                                                    Lhead = createLSTFromParams((yyvsp[-2].paramList));
                                                }
#line 2818 "y.tab.c"
    break;

  case 138: /* Fdef: GType ID '(' ParamList ')' '{' $@8 LdeclBlock Body '}'  */
#line 667 "parser.y"
                                                {
                                                    currentFunc->bodyAST = (yyvsp[-1].node) ;
                                                    currentFunc->Lentry = Lhead ;
                                                    
                                                    funcCodeGen(currentFunc,output);                                       
                                                    currentFunc = NULL ;
                                                    Lhead = NULL ;
                                                }
#line 2831 "y.tab.c"
    break;

  case 139: /* ifStmt: IF '(' EXPR ')' THEN stmtList ELSE stmtList ENDIF SEMICOLON  */
#line 677 "parser.y"
                                                                                {   
                                                                                    
                                                                                    (yyval.node) = ifelseNode(IF_NODE,(yyvsp[-7].node),(yyvsp[-4].node),(yyvsp[-2].node));
                                                                                }
#line 2840 "y.tab.c"
    break;

  case 140: /* ifStmt: IF '(' EXPR ')' THEN stmtList ENDIF SEMICOLON  */
#line 681 "parser.y"
                                                                                {
                                                                                    
                                                                                    (yyval.node) = ifelseNode(IF_NODE,(yyvsp[-5].node),(yyvsp[-2].node),NULL);
                                                                                }
#line 2849 "y.tab.c"
    break;

  case 141: /* whileStmt: WHILE '(' EXPR ')' DO stmtList ENDWHILE SEMICOLON  */
#line 687 "parser.y"
                                                                                {
                                                                                    (yyval.node) = loopNode(WHILE_NODE,(yyvsp[-5].node),(yyvsp[-2].node));
                                                                                }
#line 2857 "y.tab.c"
    break;

  case 142: /* doWhileStmt: DO stmtList WHILE '(' EXPR ')' SEMICOLON  */
#line 692 "parser.y"
                                                                                {   (yyval.node) = loopNode(DOWHILE_NODE,(yyvsp[-2].node),(yyvsp[-5].node)); }
#line 2863 "y.tab.c"
    break;

  case 143: /* repeatStmt: REPEAT stmtList UNTIL '(' EXPR ')' SEMICOLON  */
#line 695 "parser.y"
                                                                                {   (yyval.node) = loopNode(REPEAT_NODE,(yyvsp[-2].node),(yyvsp[-5].node)); }
#line 2869 "y.tab.c"
    break;

  case 144: /* ipStmt: READ '(' IDENTIFIERS ')' SEMICOLON  */
#line 698 "parser.y"
                                                                                {   (yyval.node) = ipOpNode(READ_NODE,(yyvsp[-2].node)); }
#line 2875 "y.tab.c"
    break;

  case 145: /* opStmt: WRITE '(' EXPR ')' SEMICOLON  */
#line 701 "parser.y"
                                                                                {   (yyval.node) = ipOpNode(WRITE_NODE,(yyvsp[-2].node)); }
#line 2881 "y.tab.c"
    break;

  case 146: /* assignStmt: IDENTIFIERS ASSIGN EXPR SEMICOLON  */
#line 704 "parser.y"
                                                                                {   (yyval.node) = assignNode((yyvsp[-3].node),(yyvsp[-1].node)); }
#line 2887 "y.tab.c"
    break;

  case 147: /* IDENTIFIERS: SELF DOT ID  */
#line 707 "parser.y"
                                                                              {
                                                                                    if (!currentClass) stage7Error("self can only be used inside a class method");
                                                                                    struct ClassField *f = Class_Flookup(currentClass, (yyvsp[0].node)->varname);
                                                                                    if (!f) { fprintf(stderr, "Error: Field '%s' not found in class '%s' at line %d\n", (yyvsp[0].node)->varname, currentClass->name, yylineNum); exit(1); }
                                                                                    (yyval.node) = createTreeNode(0, f->fieldIndex, f->type, strdup(f->name), SELF_FIELD_NODE, NULL, NULL, NULL, NULL);
                                                                                    (yyval.node)->Ctype = f->ctype;
                                                                                    (yyval.node)->Lentry = findLocalSymbol(Lhead, "self");
                                                                                    if (!(yyval.node)->Lentry) stage7Error("Internal error: self is missing from method local table");
                                                                                }
#line 2901 "y.tab.c"
    break;

  case 148: /* IDENTIFIERS: ID  */
#line 716 "parser.y"
                                                                                  {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    (yyval.node) = node;
                                                                                }
#line 2911 "y.tab.c"
    break;

  case 149: /* IDENTIFIERS: ID '[' EXPR ']'  */
#line 721 "parser.y"
                                                                                {   
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[-3].node),(yyvsp[-1].node),NULL);
                                                                                    node->nodetype = ARR_NODE ;
                                                                                    (yyval.node) = node;
                                                                                }
#line 2921 "y.tab.c"
    break;

  case 150: /* IDENTIFIERS: STAR ID  */
#line 726 "parser.y"
                                                                                {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE;                              
                                                                                    (yyval.node) = pointerNode(Ghead,DEREF_NODE,node,NULL,NULL);
                                                                                }
#line 2931 "y.tab.c"
    break;

  case 151: /* IDENTIFIERS: AMPERSAND ID  */
#line 731 "parser.y"
                                                                                {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    (yyval.node) = pointerNode(Ghead,ADDR_NODE,node,NULL,NULL);
                                                                                }
#line 2941 "y.tab.c"
    break;

  case 152: /* IDENTIFIERS: IDENTIFIERS DOT ID  */
#line 736 "parser.y"
                                                                                {
                                                                                    /* Allow nested UDT field access such as self.tail.next. */
                                                                                    if ((yyvsp[-2].node)->Gentry && (yyvsp[-2].node)->Gentry->Ctype) stage7Error("Class fields are private; access fields only through self inside their class");
                                                                                    if (!(yyvsp[-2].node)->type || (yyvsp[-2].node)->type->fields == NULL) {
                                                                                        const char *baseType = (yyvsp[-2].node)->Ctype ? (yyvsp[-2].node)->Ctype->name : ((yyvsp[-2].node)->type ? (yyvsp[-2].node)->type->name : "<class>");
                                                                                        fprintf(stderr, "Error: Field access ('.') on a non-structured type '%s' at line %d\n", baseType, yylineNum);
                                                                                        exit(1);
                                                                                    }
                                                                                    struct Fieldlist *field = FLookup((yyvsp[-2].node)->type, (yyvsp[0].node)->varname);

                                                                                    if (field == NULL) {
                                                                                        printf("Error: Field '%s' not found in type '%s' at line %d\n", (yyvsp[0].node)->varname, (yyvsp[-2].node)->type->name, yylineNum);                                                                                   
                                                                                        exit(1);
                                                                                    }
                                                                                    (yyval.node) = createTreeNode(0,field->fieldIndex, field->type, field->name, FIELD_NODE, NULL, (yyvsp[-2].node), NULL, NULL);
                                                                                }
#line 2962 "y.tab.c"
    break;


#line 2966 "y.tab.c"

      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", YY_CAST (yysymbol_kind_t, yyr1[yyn]), &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */
  {
    const int yylhs = yyr1[yyn] - YYNTOKENS;
    const int yyi = yypgoto[yylhs] + *yyssp;
    yystate = (0 <= yyi && yyi <= YYLAST && yycheck[yyi] == *yyssp
               ? yytable[yyi]
               : yydefgoto[yylhs]);
  }

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYSYMBOL_YYEMPTY : YYTRANSLATE (yychar);
  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
      yyerror (YY_("syntax error"));
    }

  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:
  /* Pacify compilers when the user code never invokes YYERROR and the
     label yyerrorlab therefore never appears in user code.  */
  if (0)
    YYERROR;
  ++yynerrs;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  /* Pop stack until we find a state that shifts the error token.  */
  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYSYMBOL_YYerror;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYSYMBOL_YYerror)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  YY_ACCESSING_SYMBOL (yystate), yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", YY_ACCESSING_SYMBOL (yyn), yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturnlab;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturnlab;


/*-----------------------------------------------------------.
| yyexhaustedlab -- YYNOMEM (memory exhaustion) comes here.  |
`-----------------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  goto yyreturnlab;


/*----------------------------------------------------------.
| yyreturnlab -- parsing is finished, clean up and return.  |
`----------------------------------------------------------*/
yyreturnlab:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  YY_ACCESSING_SYMBOL (+*yyssp), yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif

  return yyresult;
}

#line 754 "parser.y"


int yyerror(const char *s) {
    fprintf(stderr, "Error: %s at line %d\n", s, yylineNum);
    return 1;
}

extern FILE* yyin;

int main(int argc, char** argv) {

    if(argc > 1){
        yyin = fopen(argv[1], "r");
        if(!yyin){
            printf("Error opening file\n");
            return 1;
        }
    }

    output = fopen("output.xsm", "w");

    TTableHead = createTypeTable(); // TInstall default types -> int, str, void, bool

    auxFunctions(output,ENTRY_CODE,-1,-1);
    fprintf(output,"CALL F0\n");
    yyparse();
    auxFunctions(output,EXIT_CODE,-1,-1);
    printSymbolTable(Ghead);
    if (CTableHead) Class_PrintTable(CTableHead);

    fclose(output);
    return 0;
}
