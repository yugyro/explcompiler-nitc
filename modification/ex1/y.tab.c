/* A Bison parser, made by GNU Bison 3.5.1.  */

/* Bison implementation for Yacc-like parsers in C

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

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Undocumented macros, especially those whose name start with YY_,
   are private implementation details.  Do not rely on them.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.5.1"

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



#line 261 "y.tab.c"

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

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif

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

#line 440 "y.tab.c"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);

#endif /* !YY_YY_Y_TAB_H_INCLUDED  */



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
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && ! defined __ICC && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN                            \
    _Pragma ("GCC diagnostic push")                                     \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")              \
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
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

#if ! defined yyoverflow || YYERROR_VERBOSE

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
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


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
#define YYLAST   657

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  67
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  65
/* YYNRULES -- Number of rules.  */
#define YYNRULES  158
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  335

#define YYUNDEFTOK  2
#define YYMAXUTOK   314


/* YYTRANSLATE(TOKEN-NUM) -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, with out-of-bounds checking.  */
#define YYTRANSLATE(YYX)                                                \
  (0 <= (YYX) && (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex.  */
static const yytype_int8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
      62,    63,     2,     2,    64,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    65,     2,    66,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    60,     2,    61,     2,     2,     2,     2,
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
      55,    56,    57,    58,    59
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_int16 yyrline[] =
{
       0,   235,   235,   240,   241,   244,   245,   249,   248,   279,
     280,   283,   284,   287,   295,   305,   305,   323,   324,   327,
     327,   332,   333,   334,   346,   352,   360,   361,   362,   365,
     365,   401,   401,   424,   424,   425,   425,   426,   429,   430,
     433,   434,   437,   437,   455,   456,   459,   460,   464,   475,
     486,   490,   497,   501,   502,   503,   513,   514,   517,   521,
     525,   530,   536,   537,   540,   543,   546,   549,   554,   555,
     556,   559,   560,   563,   567,   568,   571,   572,   575,   576,
     579,   579,   596,   597,   600,   601,   605,   606,   609,   610,
     611,   624,   625,   626,   637,   638,   639,   651,   652,   653,
     654,   655,   656,   657,   658,   659,   660,   661,   662,   663,
     664,   665,   666,   667,   668,   669,   670,   671,   672,   678,
     679,   680,   683,   684,   688,   692,   706,   720,   726,   734,
     737,   743,   744,   747,   748,   749,   750,   751,   752,   753,
     754,   755,   756,   759,   759,   780,   784,   790,   795,   798,
     801,   804,   807,   810,   819,   824,   829,   834,   839
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "PLUS", "MINUS", "STAR", "DIV", "MOD",
  "AMPERSAND", "LE", "LT", "GT", "GE", "EQ", "NE", "AND", "OR", "NOT",
  "ASSIGN", "IF", "ELSE", "WHILE", "REPEAT", "UNTIL", "DO", "BREAK",
  "CONTINUE", "ENDIF", "ENDWHILE", "DECL", "ENDDECL", "BEG", "END", "INT",
  "STR", "SEMICOLON", "THEN", "READ", "WRITE", "EXIT_PR", "MAIN", "RETURN",
  "TUPLE", "DOT", "TYPE", "ENDTYPE", "NULL_VAL", "ALLOC", "INITIALIZE",
  "FREE", "CLASS", "ENDCLASS", "SELF", "NEW", "EXTENDS", "NUM",
  "STR_LITERAL", "ID", "UMINUS", "FIELD_BASE", "'{'", "'}'", "'('", "')'",
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
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_int16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,   280,   281,   282,   283,   284,
     285,   286,   287,   288,   289,   290,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   300,   301,   302,   303,   304,
     305,   306,   307,   308,   309,   310,   311,   312,   313,   314,
     123,   125,    40,    41,    44,    91,    93
};
# endif

#define YYPACT_NINF (-209)

#define yypact_value_is_default(Yyn) \
  ((Yyn) == YYPACT_NINF)

#define YYTABLE_NINF (-36)

#define yytable_value_is_error(Yyn) \
  ((Yyn) == YYTABLE_NINF)

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
      11,    10,   114,    62,  -209,   -33,  -209,  -209,    83,   120,
     119,  -209,  -209,   118,    84,   145,  -209,   147,  -209,    32,
     127,  -209,  -209,  -209,   124,   189,   224,  -209,  -209,  -209,
      78,  -209,   188,  -209,   218,  -209,  -209,   194,  -209,   121,
    -209,   234,     1,  -209,   233,  -209,   202,  -209,    32,  -209,
      32,  -209,  -209,   203,  -209,  -209,  -209,   221,   -43,    15,
    -209,   222,   232,  -209,  -209,   126,    32,  -209,   230,   261,
    -209,     1,   257,   230,   298,  -209,  -209,  -209,  -209,   264,
     -21,  -209,  -209,  -209,     7,  -209,     2,   273,  -209,   280,
     179,   112,  -209,   289,  -209,  -209,  -209,   230,   290,  -209,
    -209,  -209,   288,  -209,  -209,   287,    -2,     4,  -209,  -209,
     321,  -209,   230,  -209,   230,  -209,   294,  -209,   292,  -209,
     220,   322,   321,   192,   212,  -209,     4,  -209,  -209,  -209,
     295,  -209,   238,  -209,   323,   300,   392,   299,   322,   301,
     324,  -209,   306,  -209,  -209,  -209,  -209,    72,   305,   312,
     310,   314,   483,   483,   328,   342,   316,   317,   347,   337,
     344,   330,   364,   392,  -209,  -209,  -209,  -209,  -209,  -209,
    -209,  -209,   107,  -209,   340,  -209,  -209,    32,  -209,   341,
    -209,  -209,   337,   337,   429,   520,  -209,  -209,     5,   337,
    -209,   337,   337,  -209,   350,   353,   357,   363,   361,  -209,
    -209,   -42,   337,   575,   381,   368,   337,   391,   395,  -209,
     337,   371,  -209,   321,   -17,  -209,    71,   117,   370,   374,
     101,   183,  -209,  -209,   375,   376,     5,   384,   386,   244,
     197,   337,   337,   337,   337,   337,   337,   337,   337,   337,
     337,   337,   337,   337,  -209,   389,  -209,    25,  -209,   412,
     589,  -209,   322,  -209,   421,   436,   337,   337,   434,   435,
    -209,  -209,   122,   100,   401,  -209,   616,   219,  -209,   169,
     169,  -209,  -209,  -209,   557,   557,   557,   557,   557,   557,
     643,   630,   409,  -209,  -209,  -209,   411,     6,   483,   483,
     211,   225,  -209,  -209,  -209,   416,   262,  -209,  -209,   337,
     281,  -209,   420,  -209,   414,  -209,   383,   437,   445,   209,
     422,  -209,   239,   616,  -209,   241,  -209,     6,   483,   448,
     450,  -209,  -209,   318,  -209,  -209,  -209,   474,  -209,  -209,
    -209,   249,   452,  -209,  -209
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
       0,   143,    70,    24,    70,    20,     0,    46,    43,    45,
       0,     0,     0,     0,     0,    47,     0,    75,    94,    95,
       0,    96,     0,    77,     0,     0,     0,     0,     0,     0,
       0,    44,     0,    74,    76,    79,    87,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,   154,     0,     0,   132,   136,   137,   138,   139,   133,
     134,   135,     0,    32,     0,    29,    25,     0,    78,     0,
     156,   157,     0,     0,     0,     0,   140,   141,     0,     0,
     142,     0,     0,   116,     0,     0,     0,     0,     0,   113,
     114,   154,     0,     0,   115,     0,     0,    67,     0,   131,
       0,     0,   144,     0,     0,    86,     0,     0,     0,     0,
       0,     0,    98,   105,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,    73,     0,   153,     0,    65,    66,
       0,   158,     0,    80,     0,     0,     0,     0,     0,     0,
     117,   119,     0,   153,     0,   121,   130,     0,   112,    99,
      97,   100,   101,   102,   106,   107,   108,   109,   110,   111,
     103,   104,   158,   155,    64,   152,     0,     0,     0,     0,
       0,     0,   150,   151,   120,     0,     0,   118,   122,     0,
       0,    30,     0,    84,    81,    83,     0,     0,     0,     0,
       0,   123,     0,   129,   125,     0,    85,     0,     0,     0,
       0,   149,   148,     0,   124,   126,    82,     0,   146,   147,
     127,     0,     0,   128,   145
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int16 yypgoto[] =
{
    -209,  -209,  -209,  -209,   485,  -209,  -209,  -209,  -209,   478,
    -209,  -209,  -209,  -209,  -209,  -209,  -209,   406,  -209,  -209,
    -209,  -209,  -209,  -209,  -209,   464,  -209,  -209,  -209,   380,
     -61,    61,  -209,  -209,   439,  -209,  -135,    -4,   417,   352,
    -107,  -209,   385,  -209,  -209,  -209,   199,  -209,   -18,  -209,
    -209,  -138,  -208,  -151,  -159,  -209,  -209,  -209,  -209,  -209,
    -209,  -209,  -209,  -209,  -136
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,     2,     3,     5,     6,    10,     9,    14,    15,    16,
      34,    65,    78,   106,    79,   115,    91,    92,   213,    45,
     110,    18,    24,    25,    39,    40,    41,   107,   118,   119,
      30,    31,    32,    59,    60,    26,   137,    84,    85,   162,
     121,   132,   133,   134,   287,   304,   305,   147,    42,    86,
     135,   266,   267,   163,   164,    47,   122,   165,   166,   167,
     168,   169,   170,   171,   204
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
     172,   184,   185,   174,   209,    80,    57,    98,    46,   116,
     148,   302,    11,   149,    48,   138,   172,   172,    48,    68,
     229,   203,    69,   206,     4,   209,   209,   172,   231,   232,
     233,   234,   235,   113,   236,   237,   238,   239,   240,   241,
     242,   243,    95,    50,   216,   217,   253,    50,   172,   172,
      70,   221,   220,   222,   223,     1,    93,   160,    58,    99,
     114,   117,   161,   303,   230,    27,    28,     4,   247,    90,
      96,    97,   250,    93,   231,   232,   233,   234,   235,    71,
     236,   237,   238,   239,   240,   241,   242,   243,   312,    29,
     262,   283,   315,   269,   270,   271,   272,   273,   274,   275,
     276,   277,   278,   279,   280,   281,   252,   178,   123,    63,
     124,    64,     8,    48,     7,   331,   214,   286,   290,   291,
     231,   232,   233,   234,   235,   210,   236,   237,   238,   239,
     240,   241,   242,   243,   254,    21,   179,   306,   307,    49,
      13,    13,    50,   295,   211,    35,    36,   209,   209,    17,
     211,    54,   172,   172,    35,    36,    74,    35,    36,    75,
      76,   313,   296,    37,   258,   211,    37,   327,   209,    38,
     172,   172,    20,   103,   233,   234,   235,   -35,    38,    19,
     255,    38,   172,    77,    33,   294,   231,   232,   233,   234,
     235,   172,   236,   237,   238,   239,   240,   241,   242,   243,
     231,   232,   233,   234,   235,    23,   236,   237,   238,   239,
     240,   241,   242,   243,   231,   232,   233,   234,   235,    43,
     236,   237,   238,   239,   240,   241,   242,   243,   231,   232,
     233,   234,   235,   289,   236,   237,   238,   239,   240,   241,
     242,   243,   102,    97,   322,    51,   259,    52,   191,   148,
     127,    53,   149,   128,   129,   139,    97,    44,    36,    62,
     268,   192,   130,    81,    82,    66,   191,   148,   143,    56,
     149,   128,   129,    61,   308,   140,    97,   131,    67,   192,
     130,    38,   298,   299,    72,   191,   148,    83,   309,   149,
     193,   194,   195,   196,    73,   131,   197,   198,   192,   199,
     200,   201,   324,   299,   325,   299,   202,   265,   193,   194,
     195,   196,   333,   299,   197,   198,    87,   199,   200,   201,
      89,    94,   191,   148,   202,   311,   149,   193,   194,   195,
     196,    35,    36,   197,   198,   192,   199,   200,   201,   100,
     101,   191,   148,   202,   314,   149,   105,   109,   111,   112,
     120,   125,   142,   136,   192,    38,   126,   146,   145,   176,
     173,   175,   180,   186,   193,   194,   195,   196,   177,   181,
     197,   198,   182,   199,   200,   201,   183,   187,   188,   189,
     202,   330,   190,   193,   194,   195,   196,   205,   148,   197,
     198,   149,   199,   200,   201,   206,   207,   148,   215,   202,
     149,   212,   150,   318,   151,   152,   227,   153,   154,   155,
     319,   150,   224,   151,   152,   225,   153,   154,   155,   226,
     156,   157,   158,   228,   245,   246,   248,   249,   251,   156,
     157,   158,   256,   159,   148,   160,   257,   149,   260,   261,
     161,   263,   148,   264,   160,   149,   282,   284,   150,   161,
     151,   152,   218,   153,   154,   155,   150,   288,   151,   152,
     289,   153,   154,   155,   297,   320,   156,   157,   158,   292,
     293,   300,   301,   310,   156,   157,   158,   316,   317,   148,
     321,   160,   149,   328,   323,   329,   161,   334,   148,   160,
      12,   149,    22,   150,   161,   151,   152,   104,   153,   154,
     155,   332,   150,    55,   151,   152,   141,   153,   154,   155,
      88,   156,   157,   158,   108,   208,   326,   144,     0,     0,
     156,   157,   158,     0,     0,   148,   160,     0,   149,     0,
       0,   161,     0,     0,     0,   160,     0,     0,     0,   150,
     161,   219,   152,     0,   153,   154,   155,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,   156,   157,   158,
     231,   232,   233,   234,   235,     0,   -36,   -36,   -36,   -36,
     -36,   -36,   160,     0,     0,     0,     0,   161,   231,   232,
     233,   234,   235,     0,   236,   237,   238,   239,   240,   241,
     242,   243,   231,   232,   233,   234,   235,     0,   236,   237,
     238,   239,   240,   241,   242,   243,     0,     0,     0,     0,
     244,     0,     0,     0,     0,     0,     0,     0,     0,   231,
     232,   233,   234,   235,   285,   236,   237,   238,   239,   240,
     241,   242,   243,   231,   232,   233,   234,   235,     0,   236,
     237,   238,   239,   240,   241,   242,   231,   232,   233,   234,
     235,     0,   236,   237,   238,   239,   240,   241
};

static const yytype_int16 yycheck[] =
{
     136,   152,   153,   138,   163,    66,     5,     5,    26,     5,
       5,     5,    45,     8,    35,   122,   152,   153,    35,    62,
      62,   159,    65,    65,    57,   184,   185,   163,     3,     4,
       5,     6,     7,    35,     9,    10,    11,    12,    13,    14,
      15,    16,    63,    64,   182,   183,    63,    64,   184,   185,
      35,   189,   188,   191,   192,    44,    74,    52,    57,    57,
      62,    57,    57,    57,   202,    33,    34,    57,   206,    73,
      63,    64,   210,    91,     3,     4,     5,     6,     7,    64,
       9,    10,    11,    12,    13,    14,    15,    16,   296,    57,
     226,    66,   300,   231,   232,   233,   234,   235,   236,   237,
     238,   239,   240,   241,   242,   243,   213,    35,   112,    48,
     114,    50,    50,    35,     0,   323,   177,   252,   256,   257,
       3,     4,     5,     6,     7,    18,     9,    10,    11,    12,
      13,    14,    15,    16,    63,    51,    64,   288,   289,    61,
      57,    57,    64,    43,    43,    33,    34,   306,   307,    29,
      43,    30,   288,   289,    33,    34,    30,    33,    34,    33,
      34,   299,    62,    42,    63,    43,    42,   318,   327,    57,
     306,   307,    54,    61,     5,     6,     7,    30,    57,    60,
      63,    57,   318,    57,    57,    63,     3,     4,     5,     6,
       7,   327,     9,    10,    11,    12,    13,    14,    15,    16,
       3,     4,     5,     6,     7,    60,     9,    10,    11,    12,
      13,    14,    15,    16,     3,     4,     5,     6,     7,    30,
       9,    10,    11,    12,    13,    14,    15,    16,     3,     4,
       5,     6,     7,    24,     9,    10,    11,    12,    13,    14,
      15,    16,    63,    64,    35,    57,    63,    29,     4,     5,
      30,    57,     8,    33,    34,    63,    64,    33,    34,    57,
      63,    17,    42,    33,    34,    62,     4,     5,    30,    35,
       8,    33,    34,    40,    63,    63,    64,    57,    57,    17,
      42,    57,    63,    64,    62,     4,     5,    57,    63,     8,
      46,    47,    48,    49,    62,    57,    52,    53,    17,    55,
      56,    57,    63,    64,    63,    64,    62,    63,    46,    47,
      48,    49,    63,    64,    52,    53,    55,    55,    56,    57,
      63,    57,     4,     5,    62,    63,     8,    46,    47,    48,
      49,    33,    34,    52,    53,    17,    55,    56,    57,    66,
      60,     4,     5,    62,    63,     8,    57,    57,    60,    62,
      29,    57,    57,    31,    17,    57,    64,    57,    35,    35,
      61,    60,    57,    35,    46,    47,    48,    49,    62,    57,
      52,    53,    62,    55,    56,    57,    62,    35,    62,    62,
      62,    63,    35,    46,    47,    48,    49,    43,     5,    52,
      53,     8,    55,    56,    57,    65,    32,     5,    57,    62,
       8,    61,    19,    20,    21,    22,    43,    24,    25,    26,
      27,    19,    62,    21,    22,    62,    24,    25,    26,    62,
      37,    38,    39,    62,    43,    57,    35,    32,    57,    37,
      38,    39,    62,    41,     5,    52,    62,     8,    63,    63,
      57,    57,     5,    57,    52,     8,    57,    35,    19,    57,
      21,    22,    23,    24,    25,    26,    19,    36,    21,    22,
      24,    24,    25,    26,    63,    28,    37,    38,    39,    35,
      35,    62,    61,    57,    37,    38,    39,    57,    64,     5,
      35,    52,     8,    35,    62,    35,    57,    35,     5,    52,
       5,     8,    14,    19,    57,    21,    22,    91,    24,    25,
      26,    27,    19,    39,    21,    22,   126,    24,    25,    26,
      71,    37,    38,    39,    97,   163,   317,   132,    -1,    -1,
      37,    38,    39,    -1,    -1,     5,    52,    -1,     8,    -1,
      -1,    57,    -1,    -1,    -1,    52,    -1,    -1,    -1,    19,
      57,    21,    22,    -1,    24,    25,    26,    -1,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,    -1,    -1,    37,    38,    39,
       3,     4,     5,     6,     7,    -1,     9,    10,    11,    12,
      13,    14,    52,    -1,    -1,    -1,    -1,    57,     3,     4,
       5,     6,     7,    -1,     9,    10,    11,    12,    13,    14,
      15,    16,     3,     4,     5,     6,     7,    -1,     9,    10,
      11,    12,    13,    14,    15,    16,    -1,    -1,    -1,    -1,
      35,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,     3,
       4,     5,     6,     7,    35,     9,    10,    11,    12,    13,
      14,    15,    16,     3,     4,     5,     6,     7,    -1,     9,
      10,    11,    12,    13,    14,    15,     3,     4,     5,     6,
       7,    -1,     9,    10,    11,    12,    13,    14
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,    44,    68,    69,    57,    70,    71,     0,    50,    73,
      72,    45,    71,    57,    74,    75,    76,    29,    88,    60,
      54,    51,    76,    60,    89,    90,   102,    33,    34,    57,
      97,    98,    99,    57,    77,    33,    34,    42,    57,    91,
      92,    93,   115,    30,    33,    86,   115,   122,    35,    61,
      64,    57,    29,    57,    30,    92,    35,     5,    57,   100,
     101,    40,    57,    98,    98,    78,    62,    57,    62,    65,
      35,    64,    62,    62,    30,    33,    34,    57,    79,    81,
      97,    33,    34,    57,   104,   105,   116,    55,   101,    63,
     104,    83,    84,   115,    57,    63,    63,    64,     5,    57,
      66,    60,    63,    61,    84,    57,    80,    94,   105,    57,
      87,    60,    62,    35,    62,    82,     5,    57,    95,    96,
      29,   107,   123,   104,   104,    57,    64,    30,    33,    34,
      42,    57,   108,   109,   110,   117,    31,   103,   107,    63,
      63,    96,    57,    30,   109,    35,    57,   114,     5,     8,
      19,    21,    22,    24,    25,    26,    37,    38,    39,    41,
      52,    57,   106,   120,   121,   124,   125,   126,   127,   128,
     129,   130,   131,    61,   103,    60,    35,    62,    35,    64,
      57,    57,    62,    62,   120,   120,    35,    35,    62,    62,
      35,     4,    17,    46,    47,    48,    49,    52,    53,    55,
      56,    57,    62,   118,   131,    43,    65,    32,   106,   121,
      18,    43,    61,    85,    97,    57,   118,   118,    23,    21,
     131,   118,   118,   118,    62,    62,    62,    43,    62,    62,
     118,     3,     4,     5,     6,     7,     9,    10,    11,    12,
      13,    14,    15,    16,    35,    43,    57,   118,    35,    32,
     118,    57,   107,    63,    63,    63,    62,    62,    63,    63,
      63,    63,   131,    57,    57,    63,   118,   119,    63,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,    57,    66,    35,    35,   103,   111,    36,    24,
     118,   118,    35,    35,    63,    43,    62,    63,    63,    64,
      62,    61,     5,    57,   112,   113,   120,   120,    63,    63,
      57,    63,   119,   118,    63,   119,    57,    64,    20,    27,
      28,    35,    35,    62,    63,    63,   113,   120,    35,    35,
      63,   119,    27,    63,    35
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    67,    68,    69,    69,    70,    70,    72,    71,    73,
      73,    74,    74,    75,    75,    77,    76,    78,    78,    80,
      79,    81,    81,    81,    82,    82,    83,    83,    83,    85,
      84,    87,    86,    89,    88,    90,    88,    88,    91,    91,
      92,    92,    94,    93,    95,    95,    96,    96,    97,    97,
      97,    97,    98,    99,    99,    99,   100,   100,   101,   101,
     101,   101,   102,   102,   103,   103,   103,   103,   104,   104,
     104,   105,   105,   106,   107,   107,   108,   108,   109,   109,
     111,   110,   112,   112,   113,   113,   114,   114,   115,   115,
     115,   116,   116,   116,   117,   117,   117,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   118,
     118,   118,   118,   118,   118,   118,   118,   118,   118,   119,
     119,   120,   120,   121,   121,   121,   121,   121,   121,   121,
     121,   121,   121,   123,   122,   124,   124,   125,   126,   127,
     128,   129,   130,   131,   131,   131,   131,   131,   131
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
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
       1,     1,     1,     1,     1,     1,     1,     3,     2,     3,
       3,     3,     3,     3,     3,     2,     3,     3,     3,     3,
       3,     3,     3,     1,     1,     1,     1,     3,     4,     3,
       4,     3,     4,     5,     6,     5,     6,     7,     8,     3,
       1,     2,     1,     1,     1,     1,     1,     1,     1,     1,
       2,     2,     2,     0,    10,    10,     8,     8,     7,     7,
       5,     5,     4,     3,     1,     4,     2,     2,     3
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


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

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



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

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*-----------------------------------.
| Print this symbol's value on YYO.  |
`-----------------------------------*/

static void
yy_symbol_value_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyoutput = yyo;
  YYUSE (yyoutput);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyo, yytoknum[yytype], *yyvaluep);
# endif
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}


/*---------------------------.
| Print this symbol on YYO.  |
`---------------------------*/

static void
yy_symbol_print (FILE *yyo, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyo, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyo, yytype, yyvaluep);
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
yy_reduce_print (yy_state_t *yyssp, YYSTYPE *yyvsp, int yyrule)
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
                       yystos[+yyssp[yyi + 1 - yynrhs]],
                       &yyvsp[(yyi + 1) - (yynrhs)]
                                              );
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
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
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


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen(S) (YY_CAST (YYPTRDIFF_T, strlen (S)))
#  else
/* Return the length of YYSTR.  */
static YYPTRDIFF_T
yystrlen (const char *yystr)
{
  YYPTRDIFF_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYPTRDIFF_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYPTRDIFF_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            else
              goto append;

          append:
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (yyres)
    return yystpcpy (yyres, yystr) - yyres;
  else
    return yystrlen (yystr);
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYPTRDIFF_T *yymsg_alloc, char **yymsg,
                yy_state_t *yyssp, int yytoken)
{
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat: reported tokens (one for the "unexpected",
     one per "expected"). */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Actual size of YYARG. */
  int yycount = 0;
  /* Cumulated lengths of YYARG.  */
  YYPTRDIFF_T yysize = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[+*yyssp];
      YYPTRDIFF_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
      yysize = yysize0;
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYPTRDIFF_T yysize1
                    = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
                    yysize = yysize1;
                  else
                    return 2;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
    default: /* Avoid compiler warnings. */
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    /* Don't count the "%s"s in the final size, but reserve room for
       the terminator.  */
    YYPTRDIFF_T yysize1 = yysize + (yystrlen (yyformat) - 2 * yycount) + 1;
    if (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM)
      yysize = yysize1;
    else
      return 2;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          ++yyp;
          ++yyformat;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
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
    yy_state_fast_t yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yy_state_t yyssa[YYINITDEPTH];
    yy_state_t *yyss;
    yy_state_t *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYPTRDIFF_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYPTRDIFF_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
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

  if (yyss + yystacksize - 1 <= yyssp)
#if !defined yyoverflow && !defined YYSTACK_RELOCATE
    goto yyexhaustedlab;
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
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yy_state_t *yyss1 = yyss;
        union yyalloc *yyptr =
          YY_CAST (union yyalloc *,
                   YYSTACK_ALLOC (YY_CAST (YYSIZE_T, YYSTACK_BYTES (yystacksize))));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
# undef YYSTACK_RELOCATE
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

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
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
  case 2:
#line 235 "parser.y"
                                                                          { }
#line 1918 "y.tab.c"
    break;

  case 3:
#line 240 "parser.y"
                                             {}
#line 1924 "y.tab.c"
    break;

  case 4:
#line 241 "parser.y"
                                                   {}
#line 1930 "y.tab.c"
    break;

  case 5:
#line 244 "parser.y"
                                             {}
#line 1936 "y.tab.c"
    break;

  case 6:
#line 245 "parser.y"
                                             {}
#line 1942 "y.tab.c"
    break;

  case 7:
#line 249 "parser.y"
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
#line 1958 "y.tab.c"
    break;

  case 8:
#line 260 "parser.y"
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
#line 1978 "y.tab.c"
    break;

  case 9:
#line 279 "parser.y"
                                            { }
#line 1984 "y.tab.c"
    break;

  case 10:
#line 280 "parser.y"
                                          { }
#line 1990 "y.tab.c"
    break;

  case 11:
#line 283 "parser.y"
                                      { }
#line 1996 "y.tab.c"
    break;

  case 12:
#line 284 "parser.y"
                                      { }
#line 2002 "y.tab.c"
    break;

  case 13:
#line 287 "parser.y"
                   {
                              if (TLookup(TTableHead, (yyvsp[0].node)->varname)) {
                                  fprintf(stderr, "Error: Class '%s' conflicts with an existing user-defined type at line %d\\n", (yyvsp[0].node)->varname, yylineNum);
                                  exit(1);
                              }
                              CTableHead = CInstall(CTableHead, (yyvsp[0].node)->varname, NULL);
                              (yyval.string) = strdup((yyvsp[0].node)->varname);
                         }
#line 2015 "y.tab.c"
    break;

  case 14:
#line 295 "parser.y"
                              {
                              if (TLookup(TTableHead, (yyvsp[-2].node)->varname)) {
                                  fprintf(stderr, "Error: Class '%s' conflicts with an existing user-defined type at line %d\\n", (yyvsp[-2].node)->varname, yylineNum);
                                  exit(1);
                              }
                              CTableHead = CInstall(CTableHead, (yyvsp[-2].node)->varname, (yyvsp[0].node)->varname);
                              (yyval.string) = strdup((yyvsp[-2].node)->varname);
                         }
#line 2028 "y.tab.c"
    break;

  case 15:
#line 305 "parser.y"
                                {
                              currentClass = CLookup(CTableHead, (yyvsp[-1].string));
                              if (!currentClass) stage7Error("Internal error: class header not installed");
                              hasClassDefinitions = 1;
                              seenMethodDecl = 0;
                         }
#line 2039 "y.tab.c"
    break;

  case 16:
#line 312 "parser.y"
                         {
                              for (struct MemberFunc *m = currentClass->methods; m; m = m->next) {
                                  if (!m->isDefined) {
                                      fprintf(stderr, "Error: Method '%s' declared but not defined in class '%s'\\n", m->name, currentClass->name);
                                      exit(1);
                                  }
                              }
                              currentClass = NULL; currentMethod = NULL;
                         }
#line 2053 "y.tab.c"
    break;

  case 17:
#line 323 "parser.y"
                                              { }
#line 2059 "y.tab.c"
    break;

  case 18:
#line 324 "parser.y"
                                             { }
#line 2065 "y.tab.c"
    break;

  case 19:
#line 327 "parser.y"
                                     {
                              pendingMemberName = strdup((yyvsp[0].node)->varname);
                         }
#line 2073 "y.tab.c"
    break;

  case 21:
#line 332 "parser.y"
                      { currentType = T_INT; currentClassType = NULL; }
#line 2079 "y.tab.c"
    break;

  case 22:
#line 333 "parser.y"
                      { currentType = T_STR; currentClassType = NULL; }
#line 2085 "y.tab.c"
    break;

  case 23:
#line 334 "parser.y"
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
#line 2100 "y.tab.c"
    break;

  case 24:
#line 346 "parser.y"
                            {
                              if (seenMethodDecl) stage7Error("Class fields must be declared before methods");
                              Class_Finstall(currentClass,
                                  currentClassType ? currentClassType->name : (currentType ? currentType->name : ""),
                                  pendingMemberName, currentType, currentClassType);
                         }
#line 2111 "y.tab.c"
    break;

  case 25:
#line 352 "parser.y"
                                              {
                              if (currentClassType != NULL || currentType == NULL)
                                  stage7Error("Class methods must have a non-class return type");
                              seenMethodDecl = 1;
                              Class_Minstall(currentClass, pendingMemberName, currentType, (yyvsp[-2].paramList), methodFlabel++);
                         }
#line 2122 "y.tab.c"
    break;

  case 26:
#line 360 "parser.y"
                                        { }
#line 2128 "y.tab.c"
    break;

  case 27:
#line 361 "parser.y"
                                         { }
#line 2134 "y.tab.c"
    break;

  case 28:
#line 362 "parser.y"
                                         { }
#line 2140 "y.tab.c"
    break;

  case 29:
#line 365 "parser.y"
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
#line 2169 "y.tab.c"
    break;

  case 30:
#line 389 "parser.y"
                                    {
                         currentFunc->bodyAST = (yyvsp[-1].node);
                         currentFunc->Lentry = Lhead;
                         funcCodeGen(currentFunc, output);
                         currentFunc = NULL;
                         currentMethod = NULL;
                         Lhead = NULL;
                     }
#line 2182 "y.tab.c"
    break;

  case 31:
#line 401 "parser.y"
                                                {   
                                                    Ghead = insertTable(Ghead,"F0",T_INT,false,0,0); 
                                                    currentFunc = find(Ghead, "F0");
                                                    if(currentFunc == NULL) {
                                                        printf("Error: Function '%s' not found in symbol table\n", "main");
                                                        exit(1);
                                                    }
                                                }
#line 2195 "y.tab.c"
    break;

  case 32:
#line 409 "parser.y"
                                    {
                                        currentFunc->bodyAST = (yyvsp[-1].node);
                                        currentFunc->Lentry = Lhead;

                                        printf("\nAST:\n");
                                        printAST(currentFunc->bodyAST, 0);

                                        funcCodeGen(currentFunc, output);
                                        currentFunc = NULL;
                                        Lhead = NULL;
                                    }
#line 2211 "y.tab.c"
    break;

  case 33:
#line 424 "parser.y"
                     { bindAddress = STACK_BASE + 8 * classCount(CTableHead); }
#line 2217 "y.tab.c"
    break;

  case 35:
#line 425 "parser.y"
                     { bindAddress = STACK_BASE + 8 * classCount(CTableHead); }
#line 2223 "y.tab.c"
    break;

  case 37:
#line 426 "parser.y"
                { bindAddress = STACK_BASE + 8 * classCount(CTableHead); }
#line 2229 "y.tab.c"
    break;

  case 38:
#line 429 "parser.y"
                                            {}
#line 2235 "y.tab.c"
    break;

  case 39:
#line 430 "parser.y"
                                            {}
#line 2241 "y.tab.c"
    break;

  case 40:
#line 433 "parser.y"
                                            {}
#line 2247 "y.tab.c"
    break;

  case 41:
#line 434 "parser.y"
                                            {}
#line 2253 "y.tab.c"
    break;

  case 42:
#line 437 "parser.y"
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
#line 2273 "y.tab.c"
    break;

  case 43:
#line 452 "parser.y"
                                                {   currTupleType = NULL ;}
#line 2279 "y.tab.c"
    break;

  case 44:
#line 455 "parser.y"
                                               {}
#line 2285 "y.tab.c"
    break;

  case 45:
#line 456 "parser.y"
                                               {}
#line 2291 "y.tab.c"
    break;

  case 46:
#line 459 "parser.y"
                                            {    Ghead = insertTable(Ghead, ((yyvsp[0].node))->varname, currTupleType, false, currTupleType->size, -1);}
#line 2297 "y.tab.c"
    break;

  case 47:
#line 460 "parser.y"
                                            {    Ghead = insertTable(Ghead, ((yyvsp[0].node))->varname, currTupleType, true, currTupleType->size, -1);}
#line 2303 "y.tab.c"
    break;

  case 48:
#line 465 "parser.y"
      {
          struct Fieldlist *temp = (yyvsp[-2].flist);

          while (temp->next)
              temp = temp->next;

          (yyvsp[0].flist)->fieldIndex = temp->fieldIndex + 1;
          temp->next = (yyvsp[0].flist);
          (yyval.flist) = (yyvsp[-2].flist);
      }
#line 2318 "y.tab.c"
    break;

  case 49:
#line 476 "parser.y"
      {
          struct Fieldlist *temp = (yyvsp[-2].flist);

          while (temp->next)
              temp = temp->next;

          (yyvsp[0].flist)->fieldIndex = temp->fieldIndex + 1;
          temp->next = (yyvsp[0].flist);
          (yyval.flist) = (yyvsp[-2].flist);
      }
#line 2333 "y.tab.c"
    break;

  case 50:
#line 487 "parser.y"
      {
          (yyval.flist) = (yyvsp[-1].flist);
      }
#line 2341 "y.tab.c"
    break;

  case 51:
#line 491 "parser.y"
      {
          (yyvsp[0].flist)->fieldIndex = 0;
          (yyval.flist) = (yyvsp[0].flist);
      }
#line 2350 "y.tab.c"
    break;

  case 52:
#line 497 "parser.y"
                                            {   (yyval.flist) = createFieldlistNode(((yyvsp[0].node))->varname,currentType,0); }
#line 2356 "y.tab.c"
    break;

  case 53:
#line 501 "parser.y"
                                            {   currentType = T_INT ;}
#line 2362 "y.tab.c"
    break;

  case 54:
#line 502 "parser.y"
                                            {   currentType = T_STR ;}
#line 2368 "y.tab.c"
    break;

  case 55:
#line 503 "parser.y"
                                            {   
                                                struct TypeTable * udt = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                if(udt == NULL) {
                                                    printf("Error: Undefined type '%s' in field definition at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                                currentType = udt ;
                                            }
#line 2381 "y.tab.c"
    break;

  case 56:
#line 513 "parser.y"
                                            {}
#line 2387 "y.tab.c"
    break;

  case 57:
#line 514 "parser.y"
                                            {}
#line 2393 "y.tab.c"
    break;

  case 58:
#line 517 "parser.y"
                                            {
                                                Ghead = insertTable(Ghead,((yyvsp[0].node))->varname,currentType,false,currentClassType ? 2 : 1,-1);
                                                if (currentClassType) find(Ghead,((yyvsp[0].node))->varname)->Ctype = currentClassType;
                                            }
#line 2402 "y.tab.c"
    break;

  case 59:
#line 521 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Arrays of class objects are not supported");
                                                Ghead = insertTable(Ghead,((yyvsp[-3].node))->varname,currentType,false,(yyvsp[-1].number),-1);
                                            }
#line 2411 "y.tab.c"
    break;

  case 60:
#line 525 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Functions cannot return class objects in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,((yyvsp[-3].node))->varname,currentType,false,0,flabel++);
                                                insertParamListToGST(Ghead,((yyvsp[-3].node))->varname,(yyvsp[-1].paramList));
                                            }
#line 2421 "y.tab.c"
    break;

  case 61:
#line 530 "parser.y"
                                            {
                                                if (currentClassType) stage7Error("Pointers to class objects are not supported in Stage 7 Task 1");
                                                Ghead = insertTable(Ghead,((yyvsp[0].node))->varname,currentType == T_INT ? T_INT_PTR : T_STR_PTR,true,1,-1);
                                            }
#line 2430 "y.tab.c"
    break;

  case 62:
#line 536 "parser.y"
                           {}
#line 2436 "y.tab.c"
    break;

  case 63:
#line 537 "parser.y"
               {}
#line 2442 "y.tab.c"
    break;

  case 64:
#line 540 "parser.y"
                                                {
                                                    (yyval.node) = connectNode((yyvsp[-3].node),(yyvsp[-2].node));
                                                }
#line 2450 "y.tab.c"
    break;

  case 65:
#line 543 "parser.y"
                                                {
                                                    (yyval.node) = (yyvsp[-2].node);
                                                }
#line 2458 "y.tab.c"
    break;

  case 66:
#line 546 "parser.y"
                                                {
                                                    (yyval.node) = connectNode((yyvsp[-2].node),(yyvsp[-1].node));
                                                }
#line 2466 "y.tab.c"
    break;

  case 67:
#line 549 "parser.y"
                                               {
                                                    (yyval.node) = (yyvsp[-1].node);
                                                }
#line 2474 "y.tab.c"
    break;

  case 68:
#line 554 "parser.y"
                                            {   (yyval.paramList) = insertToParamList((yyvsp[-2].paramList),((yyvsp[0].paramList))->name,((yyvsp[0].paramList))->type,((yyvsp[0].paramList))->isPtr,((yyvsp[0].paramList))->Ctype) ; }
#line 2480 "y.tab.c"
    break;

  case 69:
#line 555 "parser.y"
                                            {   (yyval.paramList) = insertToParamList(NULL,((yyvsp[0].paramList))->name,((yyvsp[0].paramList))->type,((yyvsp[0].paramList))->isPtr,((yyvsp[0].paramList))->Ctype) ;}
#line 2486 "y.tab.c"
    break;

  case 70:
#line 556 "parser.y"
                                            {   (yyval.paramList) = NULL ;}
#line 2492 "y.tab.c"
    break;

  case 71:
#line 559 "parser.y"
                                            {   (yyval.paramList) = createParamStructNode(strdup((yyvsp[0].node)->varname),paramType,false,paramClassType); }
#line 2498 "y.tab.c"
    break;

  case 72:
#line 560 "parser.y"
                                            {   (yyval.paramList) = createParamStructNode(strdup((yyvsp[0].node)->varname),paramType == T_INT ? T_INT_PTR : T_STR_PTR,true,NULL); }
#line 2504 "y.tab.c"
    break;

  case 73:
#line 563 "parser.y"
                                            {   
                                                (yyval.node) = returnNode(currentFunc,RETURN_NODE,(yyvsp[-1].node)); 
                                            }
#line 2512 "y.tab.c"
    break;

  case 74:
#line 567 "parser.y"
                                            {   localBinding = 1; }
#line 2518 "y.tab.c"
    break;

  case 75:
#line 568 "parser.y"
                                            {}
#line 2524 "y.tab.c"
    break;

  case 76:
#line 571 "parser.y"
                                            {}
#line 2530 "y.tab.c"
    break;

  case 77:
#line 572 "parser.y"
                                            {}
#line 2536 "y.tab.c"
    break;

  case 78:
#line 575 "parser.y"
                                            {}
#line 2542 "y.tab.c"
    break;

  case 79:
#line 576 "parser.y"
                                            {}
#line 2548 "y.tab.c"
    break;

  case 80:
#line 579 "parser.y"
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
#line 2567 "y.tab.c"
    break;

  case 81:
#line 593 "parser.y"
                                                {   currTupleType = NULL ;}
#line 2573 "y.tab.c"
    break;

  case 82:
#line 596 "parser.y"
                                                {}
#line 2579 "y.tab.c"
    break;

  case 83:
#line 597 "parser.y"
                                                {}
#line 2585 "y.tab.c"
    break;

  case 84:
#line 600 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead, ((yyvsp[0].node))->varname, currTupleType,false,currTupleType->size);}
#line 2591 "y.tab.c"
    break;

  case 85:
#line 601 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead, ((yyvsp[0].node))->varname, currTupleType,true,1);}
#line 2597 "y.tab.c"
    break;

  case 86:
#line 605 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead,((yyvsp[0].node))->varname,currLocalType,false,currLocalClassType ? 2 : 1); if(currLocalClassType) findLocalSymbol(Lhead,((yyvsp[0].node))->varname)->Ctype=currLocalClassType;}
#line 2603 "y.tab.c"
    break;

  case 87:
#line 606 "parser.y"
                                            {   Lhead = insertToLocalTable(Lhead,((yyvsp[0].node))->varname,currLocalType,false,currLocalClassType ? 2 : 1); if(currLocalClassType) findLocalSymbol(Lhead,((yyvsp[0].node))->varname)->Ctype=currLocalClassType;}
#line 2609 "y.tab.c"
    break;

  case 88:
#line 609 "parser.y"
                                            { currentType = T_INT; currentClassType = NULL; }
#line 2615 "y.tab.c"
    break;

  case 89:
#line 610 "parser.y"
                                            { currentType = T_STR; currentClassType = NULL; }
#line 2621 "y.tab.c"
    break;

  case 90:
#line 611 "parser.y"
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
#line 2637 "y.tab.c"
    break;

  case 91:
#line 624 "parser.y"
                                            { paramType = T_INT; paramClassType = NULL; }
#line 2643 "y.tab.c"
    break;

  case 92:
#line 625 "parser.y"
                                            { paramType = T_STR; paramClassType = NULL; }
#line 2649 "y.tab.c"
    break;

  case 93:
#line 626 "parser.y"
                                            {
                                                paramType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                paramClassType = NULL;
                                                if (!paramType) paramClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                if (!paramType && !paramClassType) {
                                                    fprintf(stderr, "Error: Undefined parameter type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2663 "y.tab.c"
    break;

  case 94:
#line 637 "parser.y"
                                            { currLocalType = T_INT; currLocalClassType = NULL; }
#line 2669 "y.tab.c"
    break;

  case 95:
#line 638 "parser.y"
                                            { currLocalType = T_STR; currLocalClassType = NULL; }
#line 2675 "y.tab.c"
    break;

  case 96:
#line 639 "parser.y"
                                            {
                                                currLocalType = TLookup(TTableHead, (yyvsp[0].node)->varname);
                                                currLocalClassType = NULL;
                                                if (!currLocalType) currLocalClassType = CLookup(CTableHead, (yyvsp[0].node)->varname);
                                                if (!currLocalType && !currLocalClassType) {
                                                    fprintf(stderr, "Error: Undefined local type '%s' at line %d\n", (yyvsp[0].node)->varname, yylineNum);
                                                    exit(1);
                                                }
                                            }
#line 2689 "y.tab.c"
    break;

  case 97:
#line 651 "parser.y"
                                            {   (yyval.node) = exprNode(MINUS_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2695 "y.tab.c"
    break;

  case 98:
#line 652 "parser.y"
                                            {   (yyval.node) = exprNode(UNARY_MINUS_NODE, (yyvsp[0].node), NULL); }
#line 2701 "y.tab.c"
    break;

  case 99:
#line 653 "parser.y"
                                            {   (yyval.node) = exprNode(ADD_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2707 "y.tab.c"
    break;

  case 100:
#line 654 "parser.y"
                                            {   (yyval.node) = exprNode(MUL_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2713 "y.tab.c"
    break;

  case 101:
#line 655 "parser.y"
                                            {   (yyval.node) = exprNode(DIV_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2719 "y.tab.c"
    break;

  case 102:
#line 656 "parser.y"
                                            {   (yyval.node) = exprNode(MOD_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2725 "y.tab.c"
    break;

  case 103:
#line 657 "parser.y"
                                            {   (yyval.node) = exprNode(AND_NODE,(yyvsp[-2].node), (yyvsp[0].node)); }
#line 2731 "y.tab.c"
    break;

  case 104:
#line 658 "parser.y"
                                            {   (yyval.node) = exprNode(OR_NODE,(yyvsp[-2].node), (yyvsp[0].node)); }
#line 2737 "y.tab.c"
    break;

  case 105:
#line 659 "parser.y"
                                            {   (yyval.node) = exprNode(NOT_NODE,(yyvsp[0].node), NULL);}
#line 2743 "y.tab.c"
    break;

  case 106:
#line 660 "parser.y"
                                            {   (yyval.node) = exprNode(LE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2749 "y.tab.c"
    break;

  case 107:
#line 661 "parser.y"
                                            {   (yyval.node) = exprNode(LT_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2755 "y.tab.c"
    break;

  case 108:
#line 662 "parser.y"
                                            {   (yyval.node) = exprNode(GT_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2761 "y.tab.c"
    break;

  case 109:
#line 663 "parser.y"
                                            {   (yyval.node) = exprNode(GE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2767 "y.tab.c"
    break;

  case 110:
#line 664 "parser.y"
                                            {   (yyval.node) = exprNode(EQ_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2773 "y.tab.c"
    break;

  case 111:
#line 665 "parser.y"
                                            {   (yyval.node) = exprNode(NE_NODE, (yyvsp[-2].node), (yyvsp[0].node)); }
#line 2779 "y.tab.c"
    break;

  case 112:
#line 666 "parser.y"
                                            {   (yyval.node) = (yyvsp[-1].node) ;}
#line 2785 "y.tab.c"
    break;

  case 113:
#line 667 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,(yyvsp[0].number),T_INT,NULL,NUM_NODE,NULL,NULL,NULL,NULL); }
#line 2791 "y.tab.c"
    break;

  case 114:
#line 668 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,NO_VAL,T_STR,(yyvsp[0].string),STR_NODE,NULL,NULL,NULL,NULL); }
#line 2797 "y.tab.c"
    break;

  case 115:
#line 669 "parser.y"
                                            {   (yyval.node) = (yyvsp[0].node) ; }
#line 2803 "y.tab.c"
    break;

  case 116:
#line 670 "parser.y"
                                            {   (yyval.node) = createTreeNode(0,0,T_VOID,"null",NULL_NODE,NULL,NULL,NULL,NULL); }
#line 2809 "y.tab.c"
    break;

  case 117:
#line 671 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_VOID,NULL,ALLOC_NODE,NULL,NULL,NULL,NULL); }
#line 2815 "y.tab.c"
    break;

  case 118:
#line 672 "parser.y"
                                            {
                                                struct Classtable *ct = CLookup(CTableHead, (yyvsp[-1].node)->varname);
                                                if (!ct) { fprintf(stderr, "Error: Unknown class '%s' in new at line %d\n", (yyvsp[-1].node)->varname, yylineNum); exit(1); }
                                                (yyval.node) = createTreeNode(0,NO_VAL,T_VOID,strdup((yyvsp[-1].node)->varname),ALLOC_NODE,NULL,NULL,NULL,NULL);
                                                (yyval.node)->Ctype = ct;
                                            }
#line 2826 "y.tab.c"
    break;

  case 119:
#line 678 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_INT,NULL,INIT_NODE,NULL,NULL,NULL,NULL); }
#line 2832 "y.tab.c"
    break;

  case 120:
#line 679 "parser.y"
                                           {   (yyval.node) = createTreeNode(0,NO_VAL,T_INT,NULL,FREE_NODE,NULL,(yyvsp[-1].node),NULL,NULL); }
#line 2838 "y.tab.c"
    break;

  case 121:
#line 680 "parser.y"
                                           {   
                                                (yyval.node) = funcNode(Ghead,(yyvsp[-2].node),NULL);
                                            }
#line 2846 "y.tab.c"
    break;

  case 122:
#line 683 "parser.y"
                                            {   (yyval.node) = funcNode(Ghead,(yyvsp[-3].node),(yyvsp[-1].node));}
#line 2852 "y.tab.c"
    break;

  case 123:
#line 684 "parser.y"
                                           {
                                                struct MemberFunc *m = lookupMethodForCall(currentClass, (yyvsp[-2].node)->varname, NULL);
                                                (yyval.node) = makeMethodCall(m, NULL, makeSelfReceiver());
                                            }
#line 2861 "y.tab.c"
    break;

  case 124:
#line 688 "parser.y"
                                            {
                                                struct MemberFunc *m = lookupMethodForCall(currentClass, (yyvsp[-3].node)->varname, (yyvsp[-1].node));
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), makeSelfReceiver());
                                            }
#line 2870 "y.tab.c"
    break;

  case 125:
#line 692 "parser.y"
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
#line 2889 "y.tab.c"
    break;

  case 126:
#line 706 "parser.y"
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
#line 2908 "y.tab.c"
    break;

  case 127:
#line 720 "parser.y"
                                           {
                                                struct ClassField *f = Class_Flookup(currentClass, (yyvsp[-4].node)->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = lookupMethodForCall(f->ctype, (yyvsp[-2].node)->varname, NULL);
                                                (yyval.node) = makeMethodCall(m, NULL, makeSelfFieldReceiver(f));
                                            }
#line 2919 "y.tab.c"
    break;

  case 128:
#line 726 "parser.y"
                                                   {
                                                struct ClassField *f = Class_Flookup(currentClass, (yyvsp[-5].node)->varname);
                                                if (!f || !f->ctype) stage7Error("Nested method receiver is not a class-valued field");
                                                struct MemberFunc *m = lookupMethodForCall(f->ctype, (yyvsp[-3].node)->varname, (yyvsp[-1].node));
                                                (yyval.node) = makeMethodCall(m, (yyvsp[-1].node), makeSelfFieldReceiver(f));
                                            }
#line 2930 "y.tab.c"
    break;

  case 129:
#line 734 "parser.y"
                                            {
                                                (yyval.node) = appendArg((yyvsp[-2].node),(yyvsp[0].node));
                                            }
#line 2938 "y.tab.c"
    break;

  case 130:
#line 737 "parser.y"
                                            {   
                                                (yyval.node) = (yyvsp[0].node);
                                                (yyval.node)->argList = NULL ;
                                            }
#line 2947 "y.tab.c"
    break;

  case 131:
#line 743 "parser.y"
                                                {   (yyval.node) = connectNode((yyvsp[-1].node),(yyvsp[0].node)); }
#line 2953 "y.tab.c"
    break;

  case 132:
#line 744 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2959 "y.tab.c"
    break;

  case 133:
#line 747 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2965 "y.tab.c"
    break;

  case 134:
#line 748 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2971 "y.tab.c"
    break;

  case 135:
#line 749 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2977 "y.tab.c"
    break;

  case 136:
#line 750 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2983 "y.tab.c"
    break;

  case 137:
#line 751 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2989 "y.tab.c"
    break;

  case 138:
#line 752 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 2995 "y.tab.c"
    break;

  case 139:
#line 753 "parser.y"
                                                {   (yyval.node) = (yyvsp[0].node) ; }
#line 3001 "y.tab.c"
    break;

  case 140:
#line 754 "parser.y"
                                                {   (yyval.node) = flowControlNode(BREAK_NODE); }
#line 3007 "y.tab.c"
    break;

  case 141:
#line 755 "parser.y"
                                                {   (yyval.node) = flowControlNode(CONTINUE_NODE); }
#line 3013 "y.tab.c"
    break;

  case 142:
#line 756 "parser.y"
                                                {   (yyval.node) = exitNode(EXIT_NODE); }
#line 3019 "y.tab.c"
    break;

  case 143:
#line 759 "parser.y"
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
#line 3035 "y.tab.c"
    break;

  case 144:
#line 770 "parser.y"
                                                {
                                                    currentFunc->bodyAST = (yyvsp[-1].node) ;
                                                    currentFunc->Lentry = Lhead ;
                                                    
                                                    funcCodeGen(currentFunc,output);                                       
                                                    currentFunc = NULL ;
                                                    Lhead = NULL ;
                                                }
#line 3048 "y.tab.c"
    break;

  case 145:
#line 780 "parser.y"
                                                                                {   
                                                                                    
                                                                                    (yyval.node) = ifelseNode(IF_NODE,(yyvsp[-7].node),(yyvsp[-4].node),(yyvsp[-2].node));
                                                                                }
#line 3057 "y.tab.c"
    break;

  case 146:
#line 784 "parser.y"
                                                                                {
                                                                                    
                                                                                    (yyval.node) = ifelseNode(IF_NODE,(yyvsp[-5].node),(yyvsp[-2].node),NULL);
                                                                                }
#line 3066 "y.tab.c"
    break;

  case 147:
#line 790 "parser.y"
                                                                                {
                                                                                    (yyval.node) = loopNode(WHILE_NODE,(yyvsp[-5].node),(yyvsp[-2].node));
                                                                                }
#line 3074 "y.tab.c"
    break;

  case 148:
#line 795 "parser.y"
                                                                                {   (yyval.node) = loopNode(DOWHILE_NODE,(yyvsp[-2].node),(yyvsp[-5].node)); }
#line 3080 "y.tab.c"
    break;

  case 149:
#line 798 "parser.y"
                                                                                {   (yyval.node) = loopNode(REPEAT_NODE,(yyvsp[-2].node),(yyvsp[-5].node)); }
#line 3086 "y.tab.c"
    break;

  case 150:
#line 801 "parser.y"
                                                                                {   (yyval.node) = ipOpNode(READ_NODE,(yyvsp[-2].node)); }
#line 3092 "y.tab.c"
    break;

  case 151:
#line 804 "parser.y"
                                                                                {   (yyval.node) = ipOpNode(WRITE_NODE,(yyvsp[-2].node)); }
#line 3098 "y.tab.c"
    break;

  case 152:
#line 807 "parser.y"
                                                                                {   (yyval.node) = assignNode((yyvsp[-3].node),(yyvsp[-1].node)); }
#line 3104 "y.tab.c"
    break;

  case 153:
#line 810 "parser.y"
                                                                              {
                                                                                    if (!currentClass) stage7Error("self can only be used inside a class method");
                                                                                    struct ClassField *f = Class_Flookup(currentClass, (yyvsp[0].node)->varname);
                                                                                    if (!f) { fprintf(stderr, "Error: Field '%s' not found in class '%s' at line %d\n", (yyvsp[0].node)->varname, currentClass->name, yylineNum); exit(1); }
                                                                                    (yyval.node) = createTreeNode(0, f->fieldIndex, f->type, strdup(f->name), SELF_FIELD_NODE, NULL, NULL, NULL, NULL);
                                                                                    (yyval.node)->Ctype = f->ctype;
                                                                                    (yyval.node)->Lentry = findLocalSymbol(Lhead, "self");
                                                                                    if (!(yyval.node)->Lentry) stage7Error("Internal error: self is missing from method local table");
                                                                                }
#line 3118 "y.tab.c"
    break;

  case 154:
#line 819 "parser.y"
                                                                                  {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    (yyval.node) = node;
                                                                                }
#line 3128 "y.tab.c"
    break;

  case 155:
#line 824 "parser.y"
                                                                                {   
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[-3].node),(yyvsp[-1].node),NULL);
                                                                                    node->nodetype = ARR_NODE ;
                                                                                    (yyval.node) = node;
                                                                                }
#line 3138 "y.tab.c"
    break;

  case 156:
#line 829 "parser.y"
                                                                                {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE;                              
                                                                                    (yyval.node) = pointerNode(Ghead,DEREF_NODE,node,NULL,NULL);
                                                                                }
#line 3148 "y.tab.c"
    break;

  case 157:
#line 834 "parser.y"
                                                                                {
                                                                                    struct tnode * node = setTypeId(Ghead,Lhead,(yyvsp[0].node),NULL,NULL);
                                                                                    node->nodetype = ID_NODE ;
                                                                                    (yyval.node) = pointerNode(Ghead,ADDR_NODE,node,NULL,NULL);
                                                                                }
#line 3158 "y.tab.c"
    break;

  case 158:
#line 839 "parser.y"
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
#line 3179 "y.tab.c"
    break;


#line 3183 "y.tab.c"

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
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

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
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = YY_CAST (char *, YYSTACK_ALLOC (YY_CAST (YYSIZE_T, yymsg_alloc)));
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
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

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
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
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;


/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;


#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif


/*-----------------------------------------------------.
| yyreturn -- parsing is finished, return the result.  |
`-----------------------------------------------------*/
yyreturn:
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
                  yystos[+*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 857 "parser.y"


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
