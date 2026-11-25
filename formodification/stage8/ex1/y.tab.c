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

    static int classCount(struct Classtable *head) {
        int n = 0;
        for (; head; head = head->next) n++;
        return n;
    }

    static int paramWordCount(struct ParamStruct *p) {
        int n = 0;
        for (; p; p = p->next)
            n += (p->Ctype != NULL) ? 2 :
                 ((p->isPtr || p->type == NULL) ? 1 : p->type->size);
        return n;
    }

    static void stage7Error(const char *msg) {
        fprintf(stderr, "Error: %s at line %d\n", msg, yylineNum);
        exit(1);
    }

    static int checkMethodArgs(struct MemberFunc *m, struct tnode *args) {
        struct ParamStruct *p = m ? m->paramList : NULL;
        struct tnode *a = args;
        while (p && a) {
            if (!((a->nodetype == NULL_NODE && (isUserDefinedType(p->type) || p->Ctype)) ||
                  (p->Ctype ? (a->Ctype && Class_IsSubtype(a->Ctype, p->Ctype)) : (p->type == a->type && p->isPtr == a->isPtr)))) {
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
        n->val = m->funcPosition;
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

    static struct MemberFunc *lookupMethodForDefinition(struct Classtable *c, const char *name,
                                                         struct TypeTable *ret, struct ParamStruct *params) {
        return Class_MlookupSignature(c, name, ret, params);
    }

    static int argsMatchSignature(struct ParamStruct *p, struct tnode *a) {
        while (p && a) {
            int match;
            if (p->Ctype)
                match = (a->nodetype == NULL_NODE) ||
                        (a->Ctype && Class_IsSubtype(a->Ctype, p->Ctype));
            else
                match = (p->type == a->type && p->isPtr == a->isPtr);
            if (!match) return 0;
            p = p->next;
            a = a->argList;
        }
        return p == NULL && a == NULL;
    }

    static struct MemberFunc *lookupMethodForCall(struct Classtable *c, const char *name,
                                                   struct tnode *args) {
        for (struct MemberFunc *m = c ? c->methods : NULL; m; m = m->next)
            if (strcmp(m->name, name) == 0 && argsMatchSignature(m->paramList, args))
                return m;
        return NULL;
    }


    // For user-defined type definitions (Stage 6)
    struct Fieldlist * currFieldHead = NULL ;
    int currFieldIndex = 0 ;
/* Insert VFT contents after parsing, when the complete class table and
   overridden method labels are known. Keep this before the entry CALL F0. */
static void insertVFTInitialization(const char *filename) {
    FILE *in = fopen(filename, "r");
    if (!in) { perror(filename); exit(1); }
    FILE *tmp = fopen("output.xsm.tmp", "w");
    if (!tmp) { perror("output.xsm.tmp"); fclose(in); exit(1); }

    char line[4096];
    int inserted = 0;
    while (fgets(line, sizeof(line), in)) {
        if (!inserted && strcmp(line, "CALL F0\n") == 0) {
            for (struct Classtable *cl = CTableHead; cl; cl = cl->next) {
                for (int slot = 0; slot < 8; slot++) {
                    int target = -1;
                    for (struct MemberFunc *mf = cl->methods; mf; mf = mf->next) {
                        if (mf->funcPosition == slot) { target = mf->flabel; break; }
                    }
                    if (target >= 0)
                        fprintf(tmp, "MOV [%d], F%d\n", STACK_BASE + 8 * cl->classIndex + slot, target);
                    else
                        fprintf(tmp, "MOV [%d], 0\n", STACK_BASE + 8 * cl->classIndex + slot);
                }
            }
            inserted = 1;
        }
        fputs(line, tmp);
    }
    fclose(in);
    fclose(tmp);
    if (!inserted) {
        fprintf(stderr, "Error: could not find entry CALL F0 to insert VFT initialization\n");
        remove("output.xsm.tmp");
        exit(1);
    }
    if (rename("output.xsm.tmp", filename) != 0) { perror("rename output.xsm.tmp"); exit(1); }
}



#line 262 "y.tab.c"

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
    EXTENDS = 308,                 /* EXTENDS  */
    NUM = 309,                     /* NUM  */
    STR_LITERAL = 310,             /* STR_LITERAL  */
    ID = 311,                      /* ID  */
    FIELD_BASE = 312               /* FIELD_BASE  */
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
#define EXTENDS 308
#define NUM 309
#define STR_LITERAL 310
#define ID 311
#define FIELD_BASE 312

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

#line 438 "y.tab.c"

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
  YYSYMBOL_EXTENDS = 53,                   /* EXTENDS  */
  YYSYMBOL_NUM = 54,                       /* NUM  */
  YYSYMBOL_STR_LITERAL = 55,               /* STR_LITERAL  */
  YYSYMBOL_ID = 56,                        /* ID  */
  YYSYMBOL_FIELD_BASE = 57,                /* FIELD_BASE  */
  YYSYMBOL_58_ = 58,                       /* '{'  */
  YYSYMBOL_59_ = 59,                       /* '}'  */
  YYSYMBOL_60_ = 60,                       /* '('  */
  YYSYMBOL_61_ = 61,                       /* ')'  */
  YYSYMBOL_62_ = 62,                       /* ','  */
  YYSYMBOL_63_ = 63,                       /* '['  */
  YYSYMBOL_64_ = 64,                       /* ']'  */
  YYSYMBOL_YYACCEPT = 65,                  /* $accept  */
  YYSYMBOL_program = 66,                   /* program  */
  YYSYMBOL_TypeDefBlock = 67,              /* TypeDefBlock  */
  YYSYMBOL_TypeDefList = 68,               /* TypeDefList  */
  YYSYMBOL_TypeDef = 69,                   /* TypeDef  */
  YYSYMBOL_70_1 = 70,                      /* $@1  */
  YYSYMBOL_ClassDefBlock = 71,             /* ClassDefBlock  */
  YYSYMBOL_ClassDefList = 72,              /* ClassDefList  */
  YYSYMBOL_ClassHeader = 73,               /* ClassHeader  */
  YYSYMBOL_ClassDef = 74,                  /* ClassDef  */
  YYSYMBOL_75_2 = 75,                      /* $@2  */
  YYSYMBOL_ClassMemberList = 76,           /* ClassMemberList  */
  YYSYMBOL_ClassMember = 77,               /* ClassMember  */
  YYSYMBOL_78_3 = 78,                      /* $@3  */
  YYSYMBOL_ClassMemberType = 79,           /* ClassMemberType  */
  YYSYMBOL_ClassMemberTail = 80,           /* ClassMemberTail  */
  YYSYMBOL_MethodDefList = 81,             /* MethodDefList  */
  YYSYMBOL_MethodDef = 82,                 /* MethodDef  */
  YYSYMBOL_83_4 = 83,                      /* $@4  */
  YYSYMBOL_MainBlock = 84,                 /* MainBlock  */
  YYSYMBOL_85_5 = 85,                      /* $@5  */
  YYSYMBOL_GDeclBlock = 86,                /* GDeclBlock  */
  YYSYMBOL_87_6 = 87,                      /* $@6  */
  YYSYMBOL_88_7 = 88,                      /* $@7  */
  YYSYMBOL_GDeclList = 89,                 /* GDeclList  */
  YYSYMBOL_GDecl = 90,                     /* GDecl  */
  YYSYMBOL_GTupleDecl = 91,                /* GTupleDecl  */
  YYSYMBOL_92_8 = 92,                      /* $@8  */
  YYSYMBOL_GTupleIdList = 93,              /* GTupleIdList  */
  YYSYMBOL_GTupleId = 94,                  /* GTupleId  */
  YYSYMBOL_FieldDefList = 95,              /* FieldDefList  */
  YYSYMBOL_FieldDef = 96,                  /* FieldDef  */
  YYSYMBOL_FieldType = 97,                 /* FieldType  */
  YYSYMBOL_GidList = 98,                   /* GidList  */
  YYSYMBOL_Gid = 99,                       /* Gid  */
  YYSYMBOL_FDefBlock = 100,                /* FDefBlock  */
  YYSYMBOL_Body = 101,                     /* Body  */
  YYSYMBOL_ParamList = 102,                /* ParamList  */
  YYSYMBOL_Param = 103,                    /* Param  */
  YYSYMBOL_Ret = 104,                      /* Ret  */
  YYSYMBOL_LdeclBlock = 105,               /* LdeclBlock  */
  YYSYMBOL_LDeclList = 106,                /* LDeclList  */
  YYSYMBOL_LDecl = 107,                    /* LDecl  */
  YYSYMBOL_LTupleDecl = 108,               /* LTupleDecl  */
  YYSYMBOL_109_9 = 109,                    /* $@9  */
  YYSYMBOL_LTupleIdList = 110,             /* LTupleIdList  */
  YYSYMBOL_LTupleId = 111,                 /* LTupleId  */
  YYSYMBOL_IdList = 112,                   /* IdList  */
  YYSYMBOL_GType = 113,                    /* GType  */
  YYSYMBOL_ParType = 114,                  /* ParType  */
  YYSYMBOL_LType = 115,                    /* LType  */
  YYSYMBOL_EXPR = 116,                     /* EXPR  */
  YYSYMBOL_ArgList = 117,                  /* ArgList  */
  YYSYMBOL_stmtList = 118,                 /* stmtList  */
  YYSYMBOL_stmt = 119,                     /* stmt  */
  YYSYMBOL_Fdef = 120,                     /* Fdef  */
  YYSYMBOL_121_10 = 121,                   /* $@10  */
  YYSYMBOL_ifStmt = 122,                   /* ifStmt  */
  YYSYMBOL_whileStmt = 123,                /* whileStmt  */
  YYSYMBOL_doWhileStmt = 124,              /* doWhileStmt  */
  YYSYMBOL_repeatStmt = 125,               /* repeatStmt  */
  YYSYMBOL_ipStmt = 126,                   /* ipStmt  */
  YYSYMBOL_opStmt = 127,                   /* opStmt  */
  YYSYMBOL_assignStmt = 128,               /* assignStmt  */
  YYSYMBOL_IDENTIFIERS = 129               /* IDENTIFIERS  */
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
#define YYLAST   593

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  65
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  156
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  331

/* YYMAXUTOK -- Last valid token kind.  */
#define YYMAXUTOK   312


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
      60,    61,     2,     2,    62,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    63,     2,    64,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    58,     2,    59,     2,     2,     2,     2,
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
      55,    56,    57
};

#if YYDEBUG
/* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   233,   233,   238,   239,   242,   243,   247,   246,   277,
     278,   281,   282,   285,   293,   303,   303,   321,   322,   325,
     325,   330,   331,   332,   344,   350,   358,   359,   360,   363,
     363,   399,   399,   422,   422,   423,   423,   424,   427,   428,
     431,   432,   435,   435,   453,   454,   457,   458,   462,   473,
     484,   488,   495,   499,   500,   501,   511,   512,   515,   519,
     523,   528,   534,   535,   538,   541,   544,   547,   552,   553,
     554,   557,   558,   561,   565,   566,   569,   570,   573,   574,
     577,   577,   594,   595,   598,   599,   603,   604,   607,   608,
     609,   622,   623,   624,   635,   636,   637,   649,   650,   651,
     652,   653,   654,   655,   656,   657,   658,   659,   660,   661,
     662,   663,   664,   665,   666,   667,   668,   674,   675,   676,
     679,   680,   684,   688,   702,   716,   722,   730,   733,   739,
     740,   743,   744,   745,   746,   747,   748,   749,   750,   751,
     752,   755,   755,   776,   780,   786,   791,   794,   797,   800,
     803,   806,   815,   820,   825,   830,   835
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
  "INITIALIZE", "FREE", "CLASS", "ENDCLASS", "SELF", "NEW", "EXTENDS",
  "NUM", "STR_LITERAL", "ID", "FIELD_BASE", "'{'", "'}'", "'('", "')'",
  "','", "'['", "']'", "$accept", "program", "TypeDefBlock", "TypeDefList",
  "TypeDef", "$@1", "ClassDefBlock", "ClassDefList", "ClassHeader",
  "ClassDef", "$@2", "ClassMemberList", "ClassMember", "$@3",
  "ClassMemberType", "ClassMemberTail", "MethodDefList", "MethodDef",
  "$@4", "MainBlock", "$@5", "GDeclBlock", "$@6", "$@7", "GDeclList",
  "GDecl", "GTupleDecl", "$@8", "GTupleIdList", "GTupleId", "FieldDefList",
  "FieldDef", "FieldType", "GidList", "Gid", "FDefBlock", "Body",
  "ParamList", "Param", "Ret", "LdeclBlock", "LDeclList", "LDecl",
  "LTupleDecl", "$@9", "LTupleIdList", "LTupleId", "IdList", "GType",
  "ParType", "LType", "EXPR", "ArgList", "stmtList", "stmt", "Fdef",
  "$@10", "ifStmt", "whileStmt", "doWhileStmt", "repeatStmt", "ipStmt",
  "opStmt", "assignStmt", "IDENTIFIERS", YY_NULLPTR
};

static const char *
yysymbol_name (yysymbol_kind_t yysymbol)
{
  return yytname[yysymbol];
}
#endif

#define YYPACT_NINF (-284)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-36)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

/* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
   STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     -19,   -24,    53,    66,  -284,   -33,  -284,  -284,    49,   108,
      82,  -284,  -284,   105,   -12,   124,  -284,   139,  -284,    -2,
     118,  -284,  -284,  -284,   217,   143,   212,  -284,  -284,  -284,
     -20,  -284,   144,  -284,   185,  -284,  -284,   170,  -284,   106,
    -284,   204,     5,  -284,   208,  -284,   223,  -284,    -2,  -284,
      -2,  -284,  -284,   205,  -284,  -284,  -284,   233,    35,   -22,
    -284,   232,   241,  -284,  -284,   123,    -2,  -284,   255,   252,
    -284,     5,   249,   255,   297,  -284,  -284,  -284,  -284,   261,
     -16,  -284,  -284,  -284,    99,  -284,    14,   256,  -284,   286,
     114,   210,  -284,   269,  -284,  -284,  -284,   255,   293,  -284,
    -284,  -284,   292,  -284,  -284,   298,    56,    15,  -284,  -284,
     329,  -284,   255,  -284,   255,  -284,   303,  -284,   301,  -284,
     113,   338,   329,   119,   190,  -284,    15,  -284,  -284,  -284,
     313,  -284,   207,  -284,   343,   316,   380,   321,   338,   323,
     348,  -284,   324,  -284,  -284,  -284,  -284,   103,   327,   330,
     333,   339,   483,   483,   354,   368,   346,   350,   377,   319,
     370,   351,   382,   380,  -284,  -284,  -284,  -284,  -284,  -284,
    -284,  -284,   115,  -284,   356,  -284,  -284,    -2,  -284,   363,
    -284,  -284,   319,   319,   419,   504,  -284,  -284,    36,   319,
    -284,   319,  -284,   361,   365,   369,   381,   373,  -284,  -284,
     110,   319,   328,   386,   374,   319,   401,   407,  -284,   319,
     390,  -284,   329,    89,  -284,    98,   116,   389,   393,   -27,
     180,   540,   397,   398,    36,   404,   405,   230,   193,   319,
     319,   319,   319,   319,   319,   319,   319,   319,   319,   319,
     319,  -284,   406,  -284,    54,  -284,   433,   553,  -284,   338,
    -284,   434,   448,   319,   319,   438,   439,  -284,  -284,    32,
     111,   413,  -284,   566,   195,  -284,   254,   254,  -284,  -284,
     580,   580,   580,   580,   580,   580,   540,   540,   416,  -284,
    -284,  -284,   418,    16,   483,   483,   206,   219,  -284,  -284,
    -284,   424,   248,  -284,  -284,   319,   267,  -284,   435,  -284,
     430,  -284,   371,   427,   455,    -1,   436,  -284,   200,   566,
    -284,   209,  -284,    16,   483,   459,   460,  -284,  -284,   300,
    -284,  -284,  -284,   461,  -284,  -284,  -284,   236,   466,  -284,
    -284
};

/* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
   Performed when YYTABLE does not specify something else to do.  Zero
   means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       4,     0,     0,    10,     7,     0,     6,     1,     0,    37,
       0,     3,     5,    13,     0,     0,    12,    33,    63,     0,
       0,     9,    11,    15,     0,     0,     0,    53,    54,    55,
       0,    51,     0,    14,     0,    88,    89,     0,    90,     0,
      39,     0,     0,    36,    88,     2,     0,    62,    50,     8,
       0,    52,    18,     0,    34,    38,    41,     0,    58,     0,
      57,     0,     0,    49,    48,     0,     0,    61,    70,     0,
      40,     0,     0,    70,    28,    21,    22,    23,    17,     0,
       0,    91,    92,    93,     0,    69,     0,     0,    56,     0,
       0,     0,    27,     0,    19,    42,    60,     0,     0,    71,
      59,    31,     0,    16,    26,     0,     0,     0,    68,    72,
       0,   141,    70,    24,    70,    20,     0,    46,    43,    45,
       0,     0,     0,     0,     0,    47,     0,    75,    94,    95,
       0,    96,     0,    77,     0,     0,     0,     0,     0,     0,
       0,    44,     0,    74,    76,    79,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   152,     0,     0,   130,   134,   135,   136,   137,   131,
     132,   133,     0,    32,     0,    29,    25,     0,    78,     0,
     154,   155,     0,     0,     0,     0,   138,   139,     0,     0,
     140,     0,   114,     0,     0,     0,     0,     0,   111,   112,
     152,     0,     0,   113,     0,     0,    67,     0,   129,     0,
       0,   142,     0,     0,    86,     0,     0,     0,     0,     0,
       0,   103,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,    73,     0,   151,     0,    65,    66,     0,   156,     0,
      80,     0,     0,     0,     0,     0,     0,   115,   117,     0,
     151,     0,   119,   128,     0,   110,    98,    97,    99,   100,
     104,   105,   106,   107,   108,   109,   101,   102,   156,   153,
      64,   150,     0,     0,     0,     0,     0,     0,   148,   149,
     118,     0,     0,   116,   120,     0,     0,    30,     0,    84,
      81,    83,     0,     0,     0,     0,     0,   121,     0,   127,
     123,     0,    85,     0,     0,     0,     0,   147,   146,     0,
     122,   124,    82,     0,   144,   145,   125,     0,     0,   126,
     143
};

/* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -284,  -284,  -284,  -284,   490,  -284,  -284,  -284,  -284,   488,
    -284,  -284,  -284,  -284,  -284,  -284,  -284,   414,  -284,  -284,
    -284,  -284,  -284,  -284,  -284,   471,  -284,  -284,  -284,   387,
     -63,    46,  -284,  -284,   443,  -284,  -132,   -23,   421,   352,
    -115,  -284,   384,  -284,  -284,  -284,   213,  -284,   -18,  -284,
    -284,  -154,  -283,  -151,  -159,  -284,  -284,  -284,  -284,  -284,
    -284,  -284,  -284,  -284,  -136
};

/* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
       0,     2,     3,     5,     6,    10,     9,    14,    15,    16,
      34,    65,    78,   106,    79,   115,    91,    92,   212,    45,
     110,    18,    24,    25,    39,    40,    41,   107,   118,   119,
      30,    31,    32,    59,    60,    26,   137,    84,    85,   162,
     121,   132,   133,   134,   283,   300,   301,   147,    42,    86,
     135,   263,   264,   163,   164,    47,   122,   165,   166,   167,
     168,   169,   170,   171,   203
};

/* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
   positive, shift that token.  If negative, reduce the rule whose
   number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     172,   184,   185,    80,   208,   202,   174,   138,    46,   308,
      57,    11,    70,   311,    48,   210,   172,   172,    48,    98,
     116,   298,   285,     4,     1,   208,   208,   172,   215,   216,
      27,    28,     4,   318,   255,   220,   327,   221,    21,    49,
      71,   148,    50,   149,    13,    95,    50,   228,   172,   172,
      90,   244,   219,     7,    29,   247,    93,   229,   230,   231,
     232,    58,   233,   234,   235,   236,   237,   238,   239,   240,
      99,   117,   299,    93,   210,   266,   267,   268,   269,   270,
     271,   272,   273,   274,   275,   276,   277,   160,   259,   123,
     113,   124,   161,   290,    63,    68,    64,   249,    69,   286,
     287,   229,   230,   231,   232,    13,   233,   234,   235,   236,
     237,   238,   239,   240,   213,     8,   114,   282,   279,   229,
     230,   231,   232,    48,   233,   234,   235,   236,   237,   238,
     239,   240,   209,   302,   303,    54,    17,   178,    35,    36,
      19,   309,   127,   208,   208,   128,   129,    37,   172,   172,
     250,    50,    74,   291,   130,    75,    76,   210,    20,   251,
      96,    97,    38,   323,   208,   179,   172,   172,   -35,   131,
     227,   292,    43,   205,    33,   102,    97,   252,   172,    77,
     139,    97,    23,   229,   230,   231,   232,   172,   233,   234,
     235,   236,   237,   238,   239,   240,   229,   230,   231,   232,
      51,   233,   234,   235,   236,   237,   238,   239,   240,   229,
     230,   231,   232,    52,   233,   234,   235,   236,   237,   238,
     239,   240,   229,   230,   231,   232,    53,   233,   234,   235,
     236,   237,   238,   239,   240,   148,   143,   149,    56,   128,
     129,   256,    35,    36,    44,    36,   191,    61,   130,    35,
      36,   140,    97,   148,   265,   149,   294,   295,    37,   231,
     232,   320,   295,   131,   191,    66,    38,   304,    38,   103,
     321,   295,   148,    38,   149,   192,   193,   194,   195,    62,
     305,   196,   197,   191,   198,   199,   200,    81,    82,    67,
     201,   262,    72,   192,   193,   194,   195,   329,   295,   196,
     197,    73,   198,   199,   200,   148,    87,   149,   201,   307,
      89,    83,   192,   193,   194,   195,   191,    94,   196,   197,
     100,   198,   199,   200,   148,   105,   149,   201,   310,    35,
      36,   229,   230,   231,   232,   191,   233,   234,   235,   236,
     237,   238,   239,   240,   101,   192,   193,   194,   195,   109,
     111,   196,   197,    38,   198,   199,   200,   120,   112,   125,
     201,   326,   241,   126,   192,   193,   194,   195,   136,   142,
     196,   197,   146,   198,   199,   200,   148,   145,   149,   201,
     173,   175,   176,   180,   177,   148,   181,   149,   186,   150,
     314,   151,   152,   182,   153,   154,   155,   315,   150,   183,
     151,   152,   187,   153,   154,   155,   188,   156,   157,   158,
     189,   190,   204,   206,   205,   211,   156,   157,   158,   214,
     159,   222,   160,   225,   148,   223,   149,   161,   242,   224,
     243,   160,   148,   226,   149,   245,   161,   150,   246,   151,
     152,   217,   153,   154,   155,   150,   248,   151,   152,   253,
     153,   154,   155,   254,   316,   156,   157,   158,   257,   258,
     260,   261,   278,   156,   157,   158,   148,   280,   149,   284,
     160,   285,   288,   289,   293,   161,   296,   297,   160,   150,
     306,   151,   152,   161,   153,   154,   155,   328,   148,   317,
     149,   312,   313,   324,   325,    12,   319,   156,   157,   158,
     330,   150,    22,   151,   152,   104,   153,   154,   155,   148,
      55,   149,   160,   141,    88,   207,   144,   161,   108,   156,
     157,   158,   150,     0,   218,   152,   322,   153,   154,   155,
       0,     0,     0,     0,   160,     0,     0,     0,     0,   161,
     156,   157,   158,   229,   230,   231,   232,     0,   233,   234,
     235,   236,   237,   238,     0,   160,   229,   230,   231,   232,
     161,   233,   234,   235,   236,   237,   238,   239,   240,   229,
     230,   231,   232,     0,   233,   234,   235,   236,   237,   238,
     239,   240,     0,   229,   230,   231,   232,   281,   -36,   -36,
     -36,   -36,   -36,   -36
};

static const yytype_int16 yycheck[] =
{
     136,   152,   153,    66,   163,   159,   138,   122,    26,   292,
       5,    44,    34,   296,    34,    42,   152,   153,    34,     5,
       5,     5,    23,    56,    43,   184,   185,   163,   182,   183,
      32,    33,    56,    34,    61,   189,   319,   191,    50,    59,
      62,     5,    62,     7,    56,    61,    62,   201,   184,   185,
      73,   205,   188,     0,    56,   209,    74,     3,     4,     5,
       6,    56,     8,     9,    10,    11,    12,    13,    14,    15,
      56,    56,    56,    91,    42,   229,   230,   231,   232,   233,
     234,   235,   236,   237,   238,   239,   240,    51,   224,   112,
      34,   114,    56,    61,    48,    60,    50,   212,    63,   253,
     254,     3,     4,     5,     6,    56,     8,     9,    10,    11,
      12,    13,    14,    15,   177,    49,    60,   249,    64,     3,
       4,     5,     6,    34,     8,     9,    10,    11,    12,    13,
      14,    15,    17,   284,   285,    29,    28,    34,    32,    33,
      58,   295,    29,   302,   303,    32,    33,    41,   284,   285,
      61,    62,    29,    42,    41,    32,    33,    42,    53,    61,
      61,    62,    56,   314,   323,    62,   302,   303,    29,    56,
      60,    60,    29,    63,    56,    61,    62,    61,   314,    56,
      61,    62,    58,     3,     4,     5,     6,   323,     8,     9,
      10,    11,    12,    13,    14,    15,     3,     4,     5,     6,
      56,     8,     9,    10,    11,    12,    13,    14,    15,     3,
       4,     5,     6,    28,     8,     9,    10,    11,    12,    13,
      14,    15,     3,     4,     5,     6,    56,     8,     9,    10,
      11,    12,    13,    14,    15,     5,    29,     7,    34,    32,
      33,    61,    32,    33,    32,    33,    16,    39,    41,    32,
      33,    61,    62,     5,    61,     7,    61,    62,    41,     5,
       6,    61,    62,    56,    16,    60,    56,    61,    56,    59,
      61,    62,     5,    56,     7,    45,    46,    47,    48,    56,
      61,    51,    52,    16,    54,    55,    56,    32,    33,    56,
      60,    61,    60,    45,    46,    47,    48,    61,    62,    51,
      52,    60,    54,    55,    56,     5,    54,     7,    60,    61,
      61,    56,    45,    46,    47,    48,    16,    56,    51,    52,
      64,    54,    55,    56,     5,    56,     7,    60,    61,    32,
      33,     3,     4,     5,     6,    16,     8,     9,    10,    11,
      12,    13,    14,    15,    58,    45,    46,    47,    48,    56,
      58,    51,    52,    56,    54,    55,    56,    28,    60,    56,
      60,    61,    34,    62,    45,    46,    47,    48,    30,    56,
      51,    52,    56,    54,    55,    56,     5,    34,     7,    60,
      59,    58,    34,    56,    60,     5,    56,     7,    34,    18,
      19,    20,    21,    60,    23,    24,    25,    26,    18,    60,
      20,    21,    34,    23,    24,    25,    60,    36,    37,    38,
      60,    34,    42,    31,    63,    59,    36,    37,    38,    56,
      40,    60,    51,    42,     5,    60,     7,    56,    42,    60,
      56,    51,     5,    60,     7,    34,    56,    18,    31,    20,
      21,    22,    23,    24,    25,    18,    56,    20,    21,    60,
      23,    24,    25,    60,    27,    36,    37,    38,    61,    61,
      56,    56,    56,    36,    37,    38,     5,    34,     7,    35,
      51,    23,    34,    34,    61,    56,    60,    59,    51,    18,
      56,    20,    21,    56,    23,    24,    25,    26,     5,    34,
       7,    56,    62,    34,    34,     5,    60,    36,    37,    38,
      34,    18,    14,    20,    21,    91,    23,    24,    25,     5,
      39,     7,    51,   126,    71,   163,   132,    56,    97,    36,
      37,    38,    18,    -1,    20,    21,   313,    23,    24,    25,
      -1,    -1,    -1,    -1,    51,    -1,    -1,    -1,    -1,    56,
      36,    37,    38,     3,     4,     5,     6,    -1,     8,     9,
      10,    11,    12,    13,    -1,    51,     3,     4,     5,     6,
      56,     8,     9,    10,    11,    12,    13,    14,    15,     3,
       4,     5,     6,    -1,     8,     9,    10,    11,    12,    13,
      14,    15,    -1,     3,     4,     5,     6,    34,     8,     9,
      10,    11,    12,    13
};

/* YYSTOS[STATE-NUM] -- The symbol kind of the accessing symbol of
   state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    43,    66,    67,    56,    68,    69,     0,    49,    71,
      70,    44,    69,    56,    72,    73,    74,    28,    86,    58,
      53,    50,    74,    58,    87,    88,   100,    32,    33,    56,
      95,    96,    97,    56,    75,    32,    33,    41,    56,    89,
      90,    91,   113,    29,    32,    84,   113,   120,    34,    59,
      62,    56,    28,    56,    29,    90,    34,     5,    56,    98,
      99,    39,    56,    96,    96,    76,    60,    56,    60,    63,
      34,    62,    60,    60,    29,    32,    33,    56,    77,    79,
      95,    32,    33,    56,   102,   103,   114,    54,    99,    61,
     102,    81,    82,   113,    56,    61,    61,    62,     5,    56,
      64,    58,    61,    59,    82,    56,    78,    92,   103,    56,
      85,    58,    60,    34,    60,    80,     5,    56,    93,    94,
      28,   105,   121,   102,   102,    56,    62,    29,    32,    33,
      41,    56,   106,   107,   108,   115,    30,   101,   105,    61,
      61,    94,    56,    29,   107,    34,    56,   112,     5,     7,
      18,    20,    21,    23,    24,    25,    36,    37,    38,    40,
      51,    56,   104,   118,   119,   122,   123,   124,   125,   126,
     127,   128,   129,    59,   101,    58,    34,    60,    34,    62,
      56,    56,    60,    60,   118,   118,    34,    34,    60,    60,
      34,    16,    45,    46,    47,    48,    51,    52,    54,    55,
      56,    60,   116,   129,    42,    63,    31,   104,   119,    17,
      42,    59,    83,    95,    56,   116,   116,    22,    20,   129,
     116,   116,    60,    60,    60,    42,    60,    60,   116,     3,
       4,     5,     6,     8,     9,    10,    11,    12,    13,    14,
      15,    34,    42,    56,   116,    34,    31,   116,    56,   105,
      61,    61,    61,    60,    60,    61,    61,    61,    61,   129,
      56,    56,    61,   116,   117,    61,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,    56,    64,
      34,    34,   101,   109,    35,    23,   116,   116,    34,    34,
      61,    42,    60,    61,    61,    62,    60,    59,     5,    56,
     110,   111,   118,   118,    61,    61,    56,    61,   117,   116,
      61,   117,    56,    62,    19,    26,    27,    34,    34,    60,
      61,    61,   111,   118,    34,    34,    61,   117,    26,    61,
      34
};

/* YYR1[RULE-NUM] -- Symbol kind of the left-hand side of rule RULE-NUM.  */
static const yytype_uint8 yyr1[] =
{
       0,    65,    66,    67,    67,    68,    68,    70,    69,    71,
      71,    72,    72,    73,    73,    75,    74,    76,    76,    78,
      77,    79,    79,    79,    80,    80,    81,    81,    81,    83,
      82,    85,    84,    87,    86,    88,    86,    86,    89,    89,
      90,    90,    92,    91,    93,    93,    94,    94,    95,    95,
      95,    95,    96,    97,    97,    97,    98,    98,    99,    99,
      99,    99,   100,   100,   101,   101,   101,   101,   102,   102,
     102,   103,   103,   104,   105,   105,   106,   106,   107,   107,
     109,   108,   110,   110,   111,   111,   112,   112,   113,   113,
     113,   114,   114,   114,   115,   115,   115,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   116,   116,   116,
     116,   116,   116,   116,   116,   116,   116,   117,   117,   118,
     118,   119,   119,   119,   119,   119,   119,   119,   119,   119,
     119,   121,   120,   122,   122,   123,   124,   125,   126,   127,
     128,   129,   129,   129,   129,   129,   129
};

/* YYR2[RULE-NUM] -- Number of symbols on the right-hand side of rule RULE-NUM.  */
static const yytype_int8 yyr2[] =
{
       0,     2,     5,     3,     0,     2,     1,     0,     5,     3,
       0,     2,     1,     1,     3,     0,     8,     2,     0,     0,
       4,     1,     1,     1,     1,     4,     2,     1,     0,     0,
      10,     0,     9,     0,     4,     0,     3,     0,     2,     1,
       3,     2,     0,     7,     3,     1,     1,     2,     3,     3,
       2,     1,     2,     1,     1,     1,     3,     1,     1,     4,
       4,     2,     2,     0,     5,     4,     4,     3,     3,     1,
       0,     2,     3,     3,     3,     2,     2,     1,     3,     2,
       0,     7,     3,     1,     1,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     1,     1,     1,     3,     3,     3,
       3,     3,     3,     2,     3,     3,     3,     3,     3,     3,
       3,     1,     1,     1,     1,     3,     4,     3,     4,     3,
       4,     5,     6,     5,     6,     7,     8,     3,     1,     2,
       1,     1,     1,     1,     1,     1,     1,     1,     2,     2,
       2,     0,    10,    10,     8,     8,     7,     7,     5,     5,
       4,     3,     1,     4,     2,     2,     3
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
#line 233 "parser.y"
                                                                          { }
#line 1830 "y.tab.c"
    break;

  case 3: /* TypeDefBlock: TYPE TypeDefList ENDTYPE  */
#line 238 "parser.y"
                                             {}
#line 1836 "y.tab.c"
    break;

  case 4: /* TypeDefBlock: %empty  */
#line 239 "parser.y"
                                                   {}
#line 1842 "y.tab.c"
    break;

  case 5: /* TypeDefList: TypeDefList TypeDef  */
#line 242 "parser.y"
                                             {}
#line 1848 "y.tab.c"
    break;

  case 6: /* TypeDefList: TypeDef  */
#line 243 "parser.y"
                                             {}
#line 1854 "y.tab.c"
    break;

  case 7: /* $@1: %empty  */
#line 247 "parser.y"
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
#line 1870 "y.tab.c"
    break;

  case 8: /* TypeDef: ID $@1 '{' FieldDefList '}'  */
#line 258 "parser.y"
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
#line 1890 "y.tab.c"
    break;

  case 9: /* ClassDefBlock: CLASS ClassDefList ENDCLASS  */
#line 277 "parser.y"
                                            { }
#line 1896 "y.tab.c"
    break;

  case 10: /* ClassDefBlock: %empty  */
#line 278 "parser.y"
                                          { }
#line 1902 "y.tab.c"
    break;

  case 11: /* ClassDefList: ClassDefList ClassDef  */
#line 281 "parser.y"
                                      { }
#line 1908 "y.tab.c"
    break;

  case 12: /* ClassDefList: ClassDef  */
#line 282 "parser.y"
                                      { }
#line 1914 "y.tab.c"
    break;

  case 13: /* ClassHeader: ID  */
#line 285 "parser.y"
                   {
                              if (TLookup(TTableHead, (yyvsp[0].node)->varname)) {
                                  fprintf(stderr, "Error: Class '%s' conflicts with an existing user-defined type at line %d\\n", (yyvsp[0].node)->varname, yylineNum);
                                  exit(1);
                              }
                              CTableHead = CInstall(CTableHead, (yyvsp[0].node)->varname, NULL);
                              (yyval.string) = strdup((yyvsp[0].node)->varname);
                         }
#line 1927 "y.tab.c"
    break;

  case 14: /* ClassHeader: ID EXTENDS ID  */
#line 293 "parser.y"
                              {
                              if (TLookup(TTableHead, (yyvsp[-2].node)->varname)) {
                                  fprintf(stderr, "Error: Class '%s' conflicts with an existing user-defined type at line %d\\n", (yyvsp[-2].node)->varname, yylineNum);
                                  exit(1);
                              }
                              CTableHead = CInstall(CTableHead, (yyvsp[-2].node)->varname, (yyvsp[0].node)->varname);
                              (yyval.string) = strdup((yyvsp[-2].node)->varname);
                         }
#line 1940 "y.tab.c"
    break;

  case 15: /* $@2: %empty  */
#line 303 "parser.y"
                                {
                              currentClass = CLookup(CTableHead, (yyvsp[-1].string));
                              if (!currentClass) stage7Error("Internal error: class header not installed");
                              hasClassDefinitions = 1;
                              seenMethodDecl = 0;
                         }
#line 1951 "y.tab.c"
    break;

  case 16: /* ClassDef: ClassHeader '{' $@2 DECL ClassMemberList ENDDECL MethodDefList '}'  */
#line 310 "parser.y"
                         {
                              for (struct MemberFunc *m = currentClass->methods; m; m = m->next) {
                                  if (!m->isDefined) {
                                      fprintf(stderr, "Error: Method '%s' declared but not defined in class '%s'\\n", m->name, currentClass->name);
                                      exit(1);
                                  }
                              }
                              currentClass = NULL; currentMethod = NULL;
                         }
#line 1965 "y.tab.c"
    break;

  case 17: /* ClassMemberList: ClassMemberList ClassMember  */
#line 321 "parser.y"
                                              { }
#line 1971 "y.tab.c"
    break;

  case 18: /* ClassMemberList: %empty  */
#line 322 "parser.y"
                                             { }
#line 1977 "y.tab.c"
    break;

  case 19: /* $@3: %empty  */
#line 325 "parser.y"
                                     {
                              pendingMemberName = strdup((yyvsp[0].node)->varname);
                         }
#line 1985 "y.tab.c"
    break;

  case 21: /* ClassMemberType: INT  */
#line 330 "parser.y"
                      { currentType = T_INT; currentClassType = NULL; }
#line 1991 "y.tab.c"
    break;

  case 22: /* ClassMemberType: STR  */
#line 331 "parser.y"
                      { currentType = T_STR; currentClassType = NULL; }
#line 1997 "y.tab.c"
    break;

  case 23: /* ClassMemberType: ID  */
#line 332 "parser.y"
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
#line 2012 "y.tab.c"
    break;

  case 24: /* ClassMemberTail: SEMICOLON  */
#line 344 "parser.y"
                            {
                              if (seenMethodDecl) stage7Error("Class fields must be declared before methods");
                              Class_Finstall(currentClass,
                                  currentClassType ? currentClassType->name : (currentType ? currentType->name : ""),
                                  pendingMemberName, currentType, currentClassType);
                         }
#line 2023 "y.tab.c"
    break;

  case 25: /* ClassMemberTail: '(' ParamList ')' SEMICOLON  */
#line 350 "parser.y"
                                              {
                              if (currentClassType != NULL || currentType == NULL)
                                  stage7Error("Class methods must have a non-class return type");
                              seenMethodDecl = 1;
                              Class_Minstall(currentClass, pendingMemberName, currentType, (yyvsp[-2].paramList), methodFlabel++);
                         }
#line 2034 "y.tab.c"
    break;

  case 26: /* MethodDefList: MethodDefList MethodDef  */
#line 358 "parser.y"
                                        { }
#line 2040 "y.tab.c"
    break;

  case 27: /* MethodDefList: MethodDef  */
#line 359 "parser.y"
                                         { }
#line 2046 "y.tab.c"
    break;

  case 28: /* MethodDefList: %empty  */
#line 360 "parser.y"
                                         { }
#line 2052 "y.tab.c"
    break;

  case 29: /* $@4: %empty  */
#line 363 "parser.y"
                                               {
                         if (currentClassType) stage7Error("Class methods cannot return class objects in Stage 7 Task 1");
                         struct MemberFunc *m = lookupMethodForDefinition(currentClass, (yyvsp[-4].node)->varname, currentType, (yyvsp[-2].paramList));
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
                         /* Methods receive self and its VFT pointer before explicit arguments. */
                         for (struct Lsymbol *lp = Lhead; lp; lp = lp->next)
                             lp->binding -= 1;
                         Lhead = insertToLocalTable(Lhead, "self", T_VOID, false, 1);
                         findLocalSymbol(Lhead, "self")->binding = -paramWordCount((yyvsp[-2].paramList)) - 4;
                         localBinding = 1;
                     }
#line 2081 "y.tab.c"
    break;

  case 30: /* MethodDef: GType ID '(' ParamList ')' '{' $@4 LdeclBlock Body '}'  */
#line 387 "parser.y"
                                    {
                         currentFunc->bodyAST = (yyvsp[-1].node);
                         currentFunc->Lentry = Lhead;
                         funcCodeGen(currentFunc, output);
                         currentFunc = NULL;
                         currentMethod = NULL;
                         Lhead = NULL;
                     }
#line 2094 "y.tab.c"
    break;

  case 31: /* $@5: %empty  */
#line 399 "parser.y"
                                                {   
                                                    Ghead = insertTable(Ghead,"F0",T_INT,false,0,0); 
                                                    currentFunc = find(Ghead, "F0");
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", "main");
                                                        exit(1);
                                                    }
                                                }
#line 2107 "y.tab.c"
    break;

  case 32: /* MainBlock: INT MAIN '(' ')' '{' $@5 LdeclBlock Body '}'  */
#line 407 "parser.y"
                                    {
                                        currentFunc->bodyAST = (yyvsp[-1].node);
                                        currentFunc->Lentry = Lhead;

                                        printf("\nAST:\n");
                                        printAST(currentFunc->bodyAST, 0);

                                        funcCodeGen(currentFunc, output);
                                        currentFunc = NULL;
                                        Lhead = NULL;
                                    }
#line 2123 "y.tab.c"
    break;

  case 33: /* $@6: %empty  */
#line 422 "parser.y"
                     { bindAddress = STACK_BASE + 8 * classCount(CTableHead); }
#line 2129 "y.tab.c"
    break;

  case 35: /* $@7: %empty  */
#line 423 "parser.y"
                     { bindAddress = STACK_BASE + 8 * classCount(CTableHead); }
#line 2135 "y.tab.c"
    break;

  case 37: /* GDeclBlock: %empty  */
#line 424 "parser.y"
                { bindAddress = STACK_BASE + 8 * classCount(CTableHead); }
#line 2141 "y.tab.c"
    break;

  case 38: /* GDeclList: GDeclList GDecl  */
#line 427 "parser.y"
                                            {}
#line 2147 "y.tab.c"
    break;

  case 39: /* GDeclList: GDecl  */
#line 428 "parser.y"
                                            {}
#line 2153 "y.tab.c"
    break;

  case 40: /* GDecl: GType GidList SEMICOLON  */
#line 431 "parser.y"
                                            {}
#line 2159 "y.tab.c"
    break;

  case 41: /* GDecl: GTupleDecl SEMICOLON  */
#line 432 "parser.y"
                                            {}
#line 2165 "y.tab.c"
    break;

  case 42: /* $@8: %empty  */
#line 435 "parser.y"
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
#line 2185 "y.tab.c"
    break;

  case 43: /* GTupleDecl: TUPLE ID '(' FieldDefList ')' $@8 GTupleIdList  */
#line 450 "parser.y"
                                                {   currTupleType = NULL ;}
#line 2191 "y.tab.c"
    break;

  case 44: /* GTupleIdList: GTupleIdList ',' GTupleId  */
#line 453 "parser.y"
                                               {}
#line 2197 "y.tab.c"
    break;

  case 45: /* GTupleIdList: GTupleId  */
#line 454 "parser.y"
                                               {}
#line 2203 "y.tab.c"
    break;

  case 46: /* GTupleId: ID  */
#line 457 "parser.y"
                                            {    Ghead = insertTable(Ghead, ((yyvsp[0].node))->varname, currTupleType, false, currTupleType->size, -1);}
#line 2209 "y.tab.c"
    break;

  case 47: /* GTupleId: STAR ID  */
#line 458 "parser.y"
                                            {    Ghead = insertTable(Ghead, ((yyvsp[0].node))->varname, currTupleType, true, currTupleType->size, -1);}
#line 2215 "y.tab.c"
    break;

  case 48: /* FieldDefList: FieldDefList ',' FieldDef  */
#line 463 "parser.y"
      {
          struct Fieldlist *temp = (yyvsp[-2].flist);

          while (temp->next)
              temp = temp->next;

          (yyvsp[0].flist)->fieldIndex = temp->fieldIndex + 1;
          temp->next = (yyvsp[0].flist);
          (yyval.flist) = (yyvsp[-2].flist);
      }
#line 2230 "y.tab.c"
    break;

  case 49: /* FieldDefList: FieldDefList SEMICOLON FieldDef  */
#line 474 "parser.y"
      {
          struct Fieldlist *temp = (yyvsp[-2].flist);

          while (temp->next)
              temp = temp->next;

          (yyvsp[0].flist)->fieldIndex = temp->fieldIndex + 1;
          temp->next = (yyvsp[0].flist);
          (yyval.flist) = (yyvsp[-2].flist);
      }
#line 2245 "y.tab.c"
    break;

  case 50: /* FieldDefList: FieldDefList SEMICOLON  */
#line 485 "parser.y"
      {
          (yyval.flist) = (yyvsp[-1].flist);
      }
#line 2253 "y.tab.c"
    break;

  case 51: /* FieldDefList: FieldDef  */
#line 489 "parser.y"
      {
          (yyvsp[0].flist)->fieldIndex = 0;
          (yyval.flist) = (yyvsp[0].flist);
      }
#line 2262 "y.tab.c"
    break;

  case 52: /* FieldDef: FieldType ID  */
#line 495 "parser.y"
                                            {   (yyval.flist) = createFieldlistNode(((yyvsp[0].node))->varname,currentType,0); }
#line 2268 "y.tab.c"
    break;

  case 53: /* FieldType: INT  */
#line 499 "parser.y"
                                            {   currentType = T_INT ;}
#line 2274 "y.tab.c"
    break;

  case 54: /* FieldType: STR  */
#line 500 "parser.y"
                                            {   currentType = T_STR ;}
#line 2280 "y.tab.c"
    break;

  case 55: /* FieldType: ID  */
#line 501 "parser.y"
                                            {   
                                                struct TypeTable * udt = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                if(udt == NULL) {
                                                    printf("Error: Undefined type '%s' in field definition at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                                currentType = udt ;
                                            }
#line 2293 "y.tab.c"
    break;

  case 56: /* GidList: GidList ',' Gid  */
#line 511 "parser.y"
                                            {}
#line 2299 "y.tab.c"
    break;

  case 57: /* GidList: Gid  */
#line 512 "parser.y"
                                            {}
#line 2305 "y.tab.c"
    break;

  case 58: /* Gid: ID  */
#line 515 "parser.y"
                                            {
                                                Ghead = insertTable(Ghead,((yyvsp[0].node))->varname,currentType,false,currentClassType ? 2 : 1,-1);
                                                if (currentClassType) find(Ghead,((yyvsp[0].node))->varname)->Ctype = currentClassType;
                                            }
#line 2314 "y.tab.c"
    break;

  case 59: /* Gid: ID '[' NUM ']'  */
#line 519 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Arrays of class objects are not supported");
                                                Ghead = insertTable(Ghead,((yyvsp[-3].node))->varname,currentType,false,(yyvsp[-1].number),-1);
                                            }
#line 2323 "y.tab.c"
    break;

  case 60: /* Gid: ID '(' ParamList ')'  */
#line 523 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Functions cannot return class objects in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,((yyvsp[-3].node))->varname,currentType,false,0,flabel++);
                                                insertParamListToGST(Ghead,((yyvsp[-3].node))->varname,(yyvsp[-1].paramList));
                                            }
#line 2333 "y.tab.c"
    break;

  case 61: /* Gid: STAR ID  */
#line 528 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Pointers to class objects are not supported in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,((yyvsp[0].node))->varname,currentType == T_INT ? T_INT_PTR : T_STR_PTR,true,1,-1);
                                            }
#line 2342 "y.tab.c"
    break;

  case 62: /* FDefBlock: FDefBlock Fdef  */
#line 534 "parser.y"
                           {}
#line 2348 "y.tab.c"
    break;

  case 63: /* FDefBlock: %empty  */
#line 535 "parser.y"
               {}
#line 2354 "y.tab.c"
    break;

  case 64: /* Body: BEG stmtList Ret END SEMICOLON  */
#line 538 "parser.y"
                                                {
                                                    (yyval.node) = connectNode((yyvsp[-3].node),(yyvsp[-2].node));
                                                }
#line 2362 "y.tab.c"
    break;

  case 65: /* Body: BEG Ret END SEMICOLON  */
#line 541 "parser.y"
                                                {
                                                    (yyval.node) = (yyvsp[-2].node);
                                                }
#line 2370 "y.tab.c"
    break;

  case 66: /* Body: BEG stmtList Ret END  */
#line 544 "parser.y"
                                                {
                                                    (yyval.node) = connectNode((yyvsp[-2].node),(yyvsp[-1].node));
                                                }
#line 2378 "y.tab.c"
    break;

  case 67: /* Body: BEG Ret END  */
#line 547 "parser.y"
                                               {
                                                    (yyval.node) = (yyvsp[-1].node);
                                                }
#line 2386 "y.tab.c"
    break;

  case 68: /* ParamList: ParamList ',' Param  */
#line 552 "parser.y"
                                            {   (yyval.paramList) = insertToParamList((yyvsp[-2].paramList),((yyvsp[0].paramList))->name,((yyvsp[0].paramList))->type,((yyvsp[0].paramList))->isPtr,((yyvsp[0].paramList))->Ctype) ; }
#line 2392 "y.tab.c"
    break;

  case 69: /* ParamList: Param  */
#line 553 "parser.y"
                                            {   (yyval.paramList) = insertToParamList(NULL,((yyvsp[0].paramList))->name,((yyvsp[0].paramList))->type,((yyvsp[0].paramList))->isPtr,((yyvsp[0].paramList))->Ctype) ;}
#line 2398 "y.tab.c"
    break;

  case 70: /* ParamList: %empty  */
#line 554 "parser.y"
                                            {   (yyval.paramList) = NULL ;}
#line 2404 "y.tab.c"
    break;

  case 71: /* Param: ParType ID  */
#line 557 "parser.y"
                                            {   (yyval.paramList) = createParamStructNode(strdup((yyvsp[0].node)->varname),paramType,false,paramClassType); }
#line 2410 "y.tab.c"
    break;

  case 72: /* Param: ParType STAR ID  */
#line 558 "parser.y"
                                            {   (yyval.paramList) = createParamStructNode(strdup((yyvsp[0].node)->varname),paramType == T_INT ? T_INT_PTR : T_STR_PTR,true,NULL); }
#line 2416 "y.tab.c"
    break;

  case 73: /* Ret: RETURN EXPR SEMICOLON  */
#line 561 "parser.y"
                                            {   
                                                (yyval.node) = returnNode(currentFunc,RETURN_NODE,(yyvsp[-1].node)); 
                                            }
#line 2424 "y.tab.c"
    break;

  case 74: /* LdeclBlock: DECL LDeclList ENDDECL  */
#line 565 "parser.y"
                                            {   localBinding = 1; }
#line 2430 "y.tab.c"
    break;

  case 75: /* LdeclBlock: DECL ENDDECL  */
#line 566 "parser.y"
                                            {}
#line 2436 "y.tab.c"
    break;

  case 76: /* LDeclList: LDeclList LDecl  */
#line 569 "parser.y"
                                            {}
#line 2442 "y.tab.c"
    break;

  case 77: /* LDeclList: LDecl  */
#line 570 "parser.y"
                                            {}
#line 2448 "y.tab.c"
    break;

  case 78: /* LDecl: LType IdList SEMICOLON  */
#line 573 "parser.y"
                                            {}
#line 2454 "y.tab.c"
    break;

  case 79: /* LDecl: LTupleDecl SEMICOLON  */
#line 574 "parser.y"
                                            {}
#line 2460 "y.tab.c"
    break;

  case 80: /* $@9: %empty  */
#line 577 "parser.y"
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
#line 2479 "y.tab.c"
    break;

  case 81: /* LTupleDecl: TUPLE ID '(' FieldDefList ')' $@9 LTupleIdList  */
#line 591 "parser.y"
                                                {   currTupleType = NULL ;}
#line 2485 "y.tab.c"
    break;

  case 82: /* LTupleIdList: LTupleIdList ',' LTupleId  */
#line 594 "parser.y"
                                                {}
#line 2491 "y.tab.c"
    break;

  case 83: /* LTupleIdList: LTupleId  */
#line 595 "parser.y"
                                                {}
#line 2497 "y.tab.c"
    break;

  case 84: /* LTupleId: ID  */
#line 598 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead, ((yyvsp[0].node))->varname, currTupleType,false,currTupleType->size);}
#line 2503 "y.tab.c"
    break;

  case 85: /* LTupleId: STAR ID  */
#line 599 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead, ((yyvsp[0].node))->varname, currTupleType,true,1);}
#line 2509 "y.tab.c"
    break;

  case 86: /* IdList: IdList ',' ID  */
#line 603 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead,((yyvsp[0].node))->varname,currLocalType,false,currLocalClassType ? 2 : 1); if(currLocalClassType) findLocalSymbol(Lhead,((yyvsp[0].node))->varname)->Ctype=currLocalClassType;}
#line 2515 "y.tab.c"
    break;

  case 87: /* IdList: ID  */
#line 604 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead,((yyvsp[0].node))->varname,currLocalType,false,currLocalClassType ? 2 : 1); if(currLocalClassType) findLocalSymbol(Lhead,((yyvsp[0].node))->varname)->Ctype=currLocalClassType;}
#line 2521 "y.tab.c"
    break;

  case 88: /* GType: INT  */
#line 607 "parser.y"
                                            { currentType = T_INT; currentClassType = NULL; }
#line 2527 "y.tab.c"
    break;

  case 89: /* GType: STR  */
#line 608 "parser.y"
                                            { currentType = T_STR; currentClassType = NULL; }
#line 2533 "y.tab.c"
    break;

  case 90: /* GType: ID  */
#line 609 "parser.y"
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
#line 2549 "y.tab.c"
    break;

  case 91: /* ParType: INT  */
#line 622 "parser.y"
                                            { paramType = T_INT; paramClassType = NULL; }
#line 2555 "y.tab.c"
    break;

  case 92: /* ParType: STR  */
#line 623 "parser.y"
                                            { paramType = T_STR; paramClassType = NULL; }
#line 2561 "y.tab.c"
    break;

  case 93: /* ParType: ID  */
#line 624 "parser.y"
                                            {
                                                paramType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                paramClassType = NULL;
                                                if (!paramType) paramClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                if (!paramType && !paramClassType) {
                                                    fprintf(stderr, "Error: Undefined parameter type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2575 "y.tab.c"
    break;

  case 94: /* LType: INT  */
#line 635 "parser.y"
                                            { currLocalType = T_INT; currLocalClassType = NULL; }
#line 2581 "y.tab.c"
    break;

  case 95: /* LType: STR  */
#line 636 "parser.y"
                                            { currLocalType = T_STR; currLocalClassType = NULL; }
#line 2587 "y.tab.c"
    break;

  case 96: /* LType: ID  */
#line 637 "parser.y"
                                            {
                                                currLocalType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                currLocalClassType = NULL;
                                                if (!currLocalType) currLocalClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                if (!currLocalType && !currLocalClassType) {
                                                    fprintf(stderr, "Error: Undefined local type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2601 "y.tab.c"
    break;

  case 97: /* EXPR: EXPR MINUS EXPR  */
#line 649 "parser.y"
                                            {   (yyval.node) = exprNode(MINUS_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2607 "y.tab.c"
    break;

  case 98: /* EXPR: EXPR PLUS EXPR  */
#line 650 "parser.y"
                                            {   (yyval.node) = exprNode(ADD_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2613 "y.tab.c"
    break;

  case 99: /* EXPR: EXPR STAR EXPR  */
#line 651 "parser.y"
                                            {   (yyval.node) = exprNode(MUL_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2619 "y.tab.c"
    break;

  case 100: /* EXPR: EXPR DIV EXPR  */
#line 652 "parser.y"
                                            {   (yyval.node) = exprNode(DIV_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2625 "y.tab.c"
    break;

  case 101: /* EXPR: EXPR AND EXPR  */
#line 653 "parser.y"
                                            {   (yyval.node) = exprNode(AND_NODE,(yyvsp[-2].node), (yyvsp[0].node)); }
#line 2631 "y.tab.c"
    break;

  case 102: /* EXPR: EXPR OR EXPR  */
#line 654 "parser.y"
                                            {   (yyval.node) = exprNode(OR_NODE,(yyvsp[-2].node), (yyvsp[0].node)); }
#line 2637 "y.tab.c"
    break;

  case 103: /* EXPR: NOT EXPR  */
#line 655 "parser.y"
                                            {   (yyval.node) = exprNode(NOT_NODE,(yyvsp[0].node), NULL);}
#line 2643 "y.tab.c"
    break;

  case 104: /* EXPR: EXPR LE EXPR  */
#line 656 "parser.y"
                                            {   (yyval.node) = exprNode(LE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2649 "y.tab.c"
    break;

  case 105: /* EXPR: EXPR LT EXPR  */
#line 657 "parser.y"
                                            {   (yyval.node) = exprNode(LT_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2655 "y.tab.c"
    break;

  case 106: /* EXPR: EXPR GT EXPR  */
#line 658 "parser.y"
                                            {   (yyval.node) = exprNode(GT_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2661 "y.tab.c"
    break;

  case 107: /* EXPR: EXPR GE EXPR  */
#line 659 "parser.y"
                                            {   (yyval.node) = exprNode(GE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2667 "y.tab.c"
    break;

  case 108: /* EXPR: EXPR EQ EXPR  */
#line 660 "parser.y"
                                            {   (yyval.node) = exprNode(EQ_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2673 "y.tab.c"
    break;

  case 109: /* EXPR: EXPR NE EXPR  */
#line 661 "parser.y"
                                            {   (yyval.node) = exprNode(NE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2679 "y.tab.c"
    break;

  case 110: /* EXPR: '(' EXPR ')'  */
#line 662 "parser.y"
                                            {   (yyval.node) = (yyvsp[-1].node) ;}
#line 2685 "y.tab.c"
    break;

  case 111: /* EXPR: NUM  */
#line 663 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,(yyvsp[0].number),T_INT,NULL,NUM_NODE,NULL,NULL,NULL,NULL); }
#line 2691 "y.tab.c"
    break;

  case 112: /* EXPR: STR_LITERAL  */
#line 664 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,NO_VAL,T_STR,(yyvsp[0].string),STR_NODE,NULL,NULL,NULL,NULL); }
#line 2697 "y.tab.c"
    break;

  case 113: /* EXPR: IDENTIFIERS  */
#line 665 "parser.y"
                                            {   (yyval.node) = (yyvsp[0].node) ; }
#line 2703 "y.tab.c"
    break;

  case 114: /* EXPR: NULL_VAL  */
#line 666 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,0,T_VOID,"null",NULL_NODE,NULL,NULL,NULL,NULL); }
#line 2709 "y.tab.c"
    break;

  case 115: /* EXPR: ALLOC '(' ')'  */
#line 667 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_VOID,NULL,ALLOC_NODE,NULL,NULL,NULL,NULL); }
#line 2715 "y.tab.c"
    break;

  case 116: /* EXPR: NEW '(' ID ')'  */
#line 668 "parser.y"
                                            {
                                                struct Classtable *ct = CLookup(CTableHead, (yyvsp[-1].node)->varname);
                                                if (!ct) { fprintf(stderr, "Error: Unknown class '%s' in new at line %d\n", (yyvsp[-1].node)->varname, yylineNum); exit(1); }
                                                (yyval.node) = createTreeNode(0,NO_VAL,T_VOID,strdup((yyvsp[-1].node)->varname),ALLOC_NODE,NULL,NULL,NULL,NULL);
                                                (yyval.node)->Ctype = ct;
                                            }
#line 2726 "y.tab.c"
    break;

  case 117: /* EXPR: INITIALIZE '(' ')'  */
#line 674 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_INT,NULL,INIT_NODE,NULL,NULL,NULL,NULL); }
#line 2732 "y.tab.c"
    break;

  case 118: /* EXPR: FREE '(' IDENTIFIERS ')'  */
#line 675 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_INT,NULL,FREE_NODE,NULL,(yyvsp[-1].node),NULL,NULL); }
#line 2738 "y.tab.c"
    break;

  case 119: /* EXPR: ID '(' ')'  */
#line 676 "parser.y"
                                           {   
                                                (yyval.node) = funcNode(Ghead,(yyvsp[-2].node),NULL);
                                            }
#line 2746 "y.tab.c"
    break;

  case 120: /* EXPR: ID '(' ArgList ')'  */
#line 679 "parser.y"
                                            {   (yyval.node) = funcNode(Ghead,(yyvsp[-3].node),(yyvsp[-1].node));}
#line 2752 "y.tab.c"
    break;

  case 121: /* EXPR: SELF DOT ID '(' ')'  */
#line 680 "parser.y"
                                           {
                                                struct MemberFunc *m = lookupMethodForCall(currentClass, (yyvsp[-2].node)->varname, NULL);
                                                (yyval.node) = makeMethodCall(m, NULL, makeSelfReceiver());
                                            }
#line 2761 "y.tab.c"
    break;

  case 122: /* EXPR: SELF DOT ID '(' ArgList ')'  */
#line 684 "parser.y"
                                            {
                                                struct MemberFunc *m = lookupMethodForCall(currentClass, (yyvsp[-3].node)->varname, (yyvsp[-1].node));
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), makeSelfReceiver());
                                            }
#line 2770 "y.tab.c"
    break;

  case 123: /* EXPR: IDENTIFIERS DOT ID '(' ')'  */
#line 688 "parser.y"
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
                                                struct MemberFunc *m = lookupMethodForCall(receiverClass, (yyvsp[-2].node)->varname, NULL);
                                                (yyval.node) = makeMethodCall(m, NULL, (yyvsp[-4].node));
                                            }
#line 2789 "y.tab.c"
    break;

  case 124: /* EXPR: IDENTIFIERS DOT ID '(' ArgList ')'  */
#line 702 "parser.y"
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
                                                struct MemberFunc *m = lookupMethodForCall(receiverClass, (yyvsp[-3].node)->varname, (yyvsp[-1].node));
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), (yyvsp[-5].node));
                                            }
#line 2808 "y.tab.c"
    break;

  case 125: /* EXPR: SELF DOT ID DOT ID '(' ')'  */
#line 716 "parser.y"
                                           {
                                                struct ClassField *f = Class_Flookup(currentClass, (yyvsp[-4].node)->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = lookupMethodForCall(f->ctype, (yyvsp[-2].node)->varname, NULL);
                                                (yyval.node) = makeMethodCall(m, NULL, makeSelfFieldReceiver(f));
                                            }
#line 2819 "y.tab.c"
    break;

  case 126: /* EXPR: SELF DOT ID DOT ID '(' ArgList ')'  */
#line 722 "parser.y"
                                                   {
                                                struct ClassField *f = Class_Flookup(currentClass, (yyvsp[-5].node)->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = lookupMethodForCall(f->ctype, (yyvsp[-3].node)->varname, (yyvsp[-1].node));
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), makeSelfFieldReceiver(f));
                                            }
#line 2830 "y.tab.c"
    break;

  case 127: /* ArgList: ArgList ',' EXPR  */
#line 730 "parser.y"
                                            {
                                                (yyval.node) = appendArg((yyvsp[-2].node),(yyvsp[0].node));
                                            }
#line 2838 "y.tab.c"
    break;

  case 128: /* ArgList: EXPR  */
#line 733 "parser.y"
                                            {   
                                                (yyval.node) = (yyvsp[0].node);
                                                (yyval.node)->argList = NULL ;
                                            }
#line 2847 "y.tab.c"
    break;

  case 129: /* stmtList: stmtList stmt  */
#line 739 "parser.y"
                                                {   (yyval.node) = connectNode((yyvsp[-1].node),(yyvsp[0].node)); }
#line 2853 "y.tab.c"
    break;

  case 130: /* stmtList: stmt  */
#line 740 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2859 "y.tab.c"
    break;

  case 131: /* stmt: ipStmt  */
#line 743 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2865 "y.tab.c"
    break;

  case 132: /* stmt: opStmt  */
#line 744 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2871 "y.tab.c"
    break;

  case 133: /* stmt: assignStmt  */
#line 745 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2877 "y.tab.c"
    break;

  case 134: /* stmt: ifStmt  */
#line 746 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2883 "y.tab.c"
    break;

  case 135: /* stmt: whileStmt  */
#line 747 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2889 "y.tab.c"
    break;

  case 136: /* stmt: doWhileStmt  */
#line 748 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2895 "y.tab.c"
    break;

  case 137: /* stmt: repeatStmt  */
#line 749 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2901 "y.tab.c"
    break;

  case 138: /* stmt: BREAK SEMICOLON  */
#line 750 "parser.y"
                                                {   (yyval.node) = flowControlNode(BREAK_NODE); }
#line 2907 "y.tab.c"
    break;

  case 139: /* stmt: CONTINUE SEMICOLON  */
#line 751 "parser.y"
                                                {   (yyval.node) = flowControlNode(CONTINUE_NODE); }
#line 2913 "y.tab.c"
    break;

  case 140: /* stmt: EXIT_PR SEMICOLON  */
#line 752 "parser.y"
                                                {   (yyval.node) = exitNode(EXIT_NODE); }
#line 2919 "y.tab.c"
    break;

  case 141: /* $@10: %empty  */
#line 755 "parser.y"
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
#line 2935 "y.tab.c"
    break;

  case 142: /* Fdef: GType ID '(' ParamList ')' '{' $@10 LdeclBlock Body '}'  */
#line 766 "parser.y"
                                                {
                                                    currentFunc->bodyAST = (yyvsp[-1].node) ;
                                                    currentFunc->Lentry = Lhead ;
                                                    
                                                    funcCodeGen(currentFunc,output);                                       
                                                    currentFunc = NULL ;
                                                    Lhead = NULL ;
                                                }
#line 2948 "y.tab.c"
    break;

  case 143: /* ifStmt: IF '(' EXPR ')' THEN stmtList ELSE stmtList ENDIF SEMICOLON  */
#line 776 "parser.y"
                                                                                {   
                                                                                    
                                                                                    (yyval.node) = ifelseNode(IF_NODE,(yyvsp[-7].node),(yyvsp[-4].node),(yyvsp[-2].node));
                                                                                }
#line 2957 "y.tab.c"
    break;

  case 144: /* ifStmt: IF '(' EXPR ')' THEN stmtList ENDIF SEMICOLON  */
#line 780 "parser.y"
                                                                                {
                                                                                    
                                                                                    (yyval.node) = ifelseNode(IF_NODE,(yyvsp[-5].node),(yyvsp[-2].node),NULL);
                                                                                }
#line 2966 "y.tab.c"
    break;

  case 145: /* whileStmt: WHILE '(' EXPR ')' DO stmtList ENDWHILE SEMICOLON  */
#line 786 "parser.y"
                                                                                {
                                                                                    (yyval.node) = loopNode(WHILE_NODE,(yyvsp[-5].node),(yyvsp[-2].node));
                                                                                }
#line 2974 "y.tab.c"
    break;

  case 146: /* doWhileStmt: DO stmtList WHILE '(' EXPR ')' SEMICOLON  */
#line 791 "parser.y"
                                                                                {   (yyval.node) = loopNode(DOWHILE_NODE,(yyvsp[-2].node),(yyvsp[-5].node)); }
#line 2980 "y.tab.c"
    break;

  case 147: /* repeatStmt: REPEAT stmtList UNTIL '(' EXPR ')' SEMICOLON  */
#line 794 "parser.y"
                                                                                {   (yyval.node) = loopNode(REPEAT_NODE,(yyvsp[-2].node),(yyvsp[-5].node)); }
#line 2986 "y.tab.c"
    break;

  case 148: /* ipStmt: READ '(' IDENTIFIERS ')' SEMICOLON  */
#line 797 "parser.y"
                                                                                {   (yyval.node) = ipOpNode(READ_NODE,(yyvsp[-2].node)); }
#line 2992 "y.tab.c"
    break;

  case 149: /* opStmt: WRITE '(' EXPR ')' SEMICOLON  */
#line 800 "parser.y"
                                                                                {   (yyval.node) = ipOpNode(WRITE_NODE,(yyvsp[-2].node)); }
#line 2998 "y.tab.c"
    break;

  case 150: /* assignStmt: IDENTIFIERS ASSIGN EXPR SEMICOLON  */
#line 803 "parser.y"
                                                                                {   (yyval.node) = assignNode((yyvsp[-3].node),(yyvsp[-1].node)); }
#line 3004 "y.tab.c"
    break;

  case 151: /* IDENTIFIERS: SELF DOT ID  */
#line 806 "parser.y"
                                                                              {
                                                                                    if (!currentClass) stage7Error("self can only be used inside a class method");
                                                                                    struct ClassField *f = Class_Flookup(currentClass, (yyvsp[0].node)->varname);
                                                                                    if (!f) { fprintf(stderr, "Error: Field '%s' not found in class '%s' at line %d\n", (yyvsp[0].node)->varname, currentClass->name, yylineNum); exit(1); }
                                                                                    (yyval.node) = createTreeNode(0, f->fieldIndex, f->type, strdup(f->name), SELF_FIELD_NODE, NULL, NULL, NULL, NULL);
                                                                                    (yyval.node)->Ctype = f->ctype;
                                                                                    (yyval.node)->Lentry = findLocalSymbol(Lhead, "self");
                                                                                    if (!(yyval.node)->Lentry) stage7Error("Internal error: self is missing from method local table");
                                                                                }
#line 3018 "y.tab.c"
    break;

  case 152: /* IDENTIFIERS: ID  */
#line 815 "parser.y"
                                                                                  {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    (yyval.node) = node;
                                                                                }
#line 3028 "y.tab.c"
    break;

  case 153: /* IDENTIFIERS: ID '[' EXPR ']'  */
#line 820 "parser.y"
                                                                                {   
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[-3].node),(yyvsp[-1].node),NULL);
                                                                                    node->nodetype = ARR_NODE ;
                                                                                    (yyval.node) = node;
                                                                                }
#line 3038 "y.tab.c"
    break;

  case 154: /* IDENTIFIERS: STAR ID  */
#line 825 "parser.y"
                                                                                {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE;                              
                                                                                    (yyval.node) = pointerNode(Ghead,DEREF_NODE,node,NULL,NULL);
                                                                                }
#line 3048 "y.tab.c"
    break;

  case 155: /* IDENTIFIERS: AMPERSAND ID  */
#line 830 "parser.y"
                                                                                {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    (yyval.node) = pointerNode(Ghead,ADDR_NODE,node,NULL,NULL);
                                                                                }
#line 3058 "y.tab.c"
    break;

  case 156: /* IDENTIFIERS: IDENTIFIERS DOT ID  */
#line 835 "parser.y"
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
#line 3079 "y.tab.c"
    break;


#line 3083 "y.tab.c"

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

#line 853 "parser.y"


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
    fflush(output);
    fclose(output);
    insertVFTInitialization("output.xsm");
    printSymbolTable(Ghead);
    if (CTableHead) Class_PrintTable(CTableHead);
    return 0;
}
