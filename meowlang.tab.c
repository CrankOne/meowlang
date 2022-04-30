/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018 Free Software Foundation, Inc.

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

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.1"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 2

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1


/* Substitute the variable and function names.  */
#define yyparse         na64dpsu_parse
#define yylex           na64dpsu_lex
#define yyerror         na64dpsu_error
#define yydebug         na64dpsu_debug
#define yynerrs         na64dpsu_nerrs


/* Copy the first part of user declarations.  */

#line 73 "ds-dsl.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 1
#endif

/* In a future release of Bison, this section will be replaced
   by #include "ds-dsl.tab.h".  */
#ifndef YY_NA64DPSU_DS_DSL_TAB_H_INCLUDED
# define YY_NA64DPSU_DS_DSL_TAB_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int na64dpsu_debug;
#endif
/* "%code requires" blocks.  */
#line 1 "ds-dsl.y" /* yacc.c:355  */


#include "na64detID/detectorIDSelect.h"

/* Shortcuts */
typedef na64dp_dsul_CodeVal_t CodeVal_t;
typedef na64dp_dsul_DetIDSemanticGetter Getter;

/** Boolean type for */
typedef int Bool_t;

/* Comparison function callback -- returns 1 or 0 */
typedef Bool_t (*cmp_f_t)(CodeVal_t, CodeVal_t);

/* Value reference structures are the subject of two-staged lifecycle:
 *  1. At stage of lexical analysis and parsing, value references keep only the
 *  offset on the symbols tables.
 *  2. Once expression is being fully parsed, the value references will be
 *  resolved into particular offsets within data arrays of the following types:
 *      - literals (immutable values)
 *      - constants (rarely mutable values defined by extern context substitution)
 *      - static values (higly mutable values defined by expression argument)
 *      - dynamic getters (direct function pointers referencing the)
 *  2. At the stage of evaluation symbol the value reference has be considered
 *  by its numerical value, which, if corrsponds to the NULL page has to be
 *  interpreted as an absolute offset within data (literals-constants-statics)
 *  block or as direct getter function callback.
 *
 * Here we exploit a hack of interpreting NULL address page as an integer
 * value assuming that addresses < NA64DP_DETID_MAX_DICT_OFFSET can not
 * refer to any real function.
 */
typedef union ValueReference {
    CodeVal_t symbOffset;
    Getter valAddr;
} ValueReference_t;

/* Defines single comparison expression struct (kin=MM, wire>=12, etc) */
typedef struct ComparisonExpression {
    /* comparison function: eq, ne, gt, etc. */
    cmp_f_t cmp_f;
    /* offset in features dictionary (left arg) */
    ValueReference_t left;
    /* sample number to which comparison is performed (right arg) */
    ValueReference_t right;
} ComparisonExpression_t;

struct SelExpr;  // fwd
typedef const struct SelExpr* SelExprPtr_t;

// Binary logic operator callback -- returns 1 or 0
typedef Bool_t (* binop_f_t)(const CodeVal_t *, SelExprPtr_t, SelExprPtr_t);

typedef struct SelExpr {
    binop_f_t bo;
    struct SelExpr * a
                 , * b
                 ;
} SelExpr_t;

/* TODO: rename
 *
 * The symbol is an elementary link between the particular value reference and
 * corresponding getter within a table. This is a temporary structure and will
 * be removed at the "baking" stage.
 * */
struct Symbol {
    unsigned short defIdx;
    CodeVal_t value;
    char * name;  /* Only used for externally-resolved symbols */
    CodeVal_t offset;  /* offset within a data block, initialized on baking */
};

/** Temporary workspace structure
 *
 * This structure contains the entire parsing and lexical analysis context of
 * the selector expression.
 * */
typedef struct Workspace {
    /** Memory pool to keep evaluation tree */
    char * bufferStart
       , * cursor
       , * bufferEnd
       ;
    /** Pointer to getters table */
    const struct na64dpsu_SymDefinition * gettersTable;
    /** Error message buffer */
    char * errMsg;
    int errMsgSize;
    /* Collected symbols; pointer to heap-allocated array */
    struct Symbol * symbols;
    /* Expression root node's */
    SelExpr_t * root;
    /* Data ptr for symbol resolve */
    CodeVal_t * dataBlockBgn;
    /* Pointer to the external resolver (if any) */
    void * externResolverPtr;
    /* Debug messages stream */
    FILE * dbgStream;
} Workspace_t;

/* Compile function error codes */
#define NA64DP_DSUL_STREXPR_BAD         -1  /* bad argument expression NULL or not a string */
#define NA64DP_DSUL_BUFFER_EXHAUSTED    -2  /* bad or insufficient buffer */
#define NA64DP_DSUL_MEMORY_ERROR        -3  /* heap allocation failure */
#define NA64DP_DSUL_TRANSLATION_ERROR   -4  /* syntax/parser/lexer error */
#define NA64DP_DSUL_BAD_GETTERS_TABLE   -5  /* bad getter table entry */

typedef void *yyscan_t;  /* circumvent circular dep: YACC/BISON does not know this type */

#line 214 "ds-dsl.tab.c" /* yacc.c:355  */

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    L_STR = 258,
    T_LBC = 259,
    T_RBC = 260,
    T_EXCLMM = 261,
    T_GT = 262,
    T_GTE = 263,
    T_LT = 264,
    T_LTE = 265,
    T_EQ = 266,
    T_NE = 267,
    T_AMP = 268,
    T_PIPE = 269,
    T_CAP = 270,
    T_UNKNOWN_IDENTIFIER = 271,
    T_INVALID_VALUE = 272,
    T_VAL_REF_TOKEN = 273
  };
#endif

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 164 "ds-dsl.y" /* yacc.c:355  */

    const char * strID;
    cmp_f_t cmpOpFunc;
    binop_f_t binOpFunc;
    ValueReference_t valRef;
    ComparisonExpression_t * cmpExpr;
    SelExpr_t * selectionBinOp;

#line 254 "ds-dsl.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif

/* Location type.  */
#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE YYLTYPE;
struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
};
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif



int na64dpsu_parse (Workspace_t * ws, yyscan_t yyscanner);
/* "%code provides" blocks.  */
#line 122 "ds-dsl.y" /* yacc.c:355  */


# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <assert.h>

#define YY_DECL \
    int na64dpsu_lex( YYSTYPE* yylval_param \
             , YYLTYPE* yylloc_param \
             , Workspace_t * ws \
             , yyscan_t yyscanner )
YY_DECL;

void na64dpsu_error( struct YYLTYPE *
                   , Workspace_t *
                   , yyscan_t yyscanner
                   , const char* );


#line 301 "ds-dsl.tab.c" /* yacc.c:355  */

#endif /* !YY_NA64DPSU_DS_DSL_TAB_H_INCLUDED  */

/* Copy the second part of user declarations.  */

#line 307 "ds-dsl.tab.c" /* yacc.c:358  */
/* Unqualified %code blocks.  */
#line 143 "ds-dsl.y" /* yacc.c:359  */

static Bool_t _cmp_f_eq( CodeVal_t a, CodeVal_t b );  /* =  */
static Bool_t _cmp_f_ne( CodeVal_t a, CodeVal_t b );  /* /= */
static Bool_t _cmp_f_gt( CodeVal_t a, CodeVal_t b );  /* >  */
static Bool_t _cmp_f_ge( CodeVal_t a, CodeVal_t b );  /* <= */
static Bool_t _cmp_f_lt( CodeVal_t a, CodeVal_t b );  /* <  */
static Bool_t _cmp_f_le( CodeVal_t a, CodeVal_t b );  /* <= */
/*static Bool_t _cmp_f_any( CodeVal_t a, CodeVal_t b ); <anything> */

static Bool_t _bo_f_and(const CodeVal_t *, SelExprPtr_t, SelExprPtr_t );  // &
static Bool_t _bo_f_or( const CodeVal_t *, SelExprPtr_t, SelExprPtr_t );  // |
static Bool_t _bo_f_xor(const CodeVal_t *, SelExprPtr_t, SelExprPtr_t );  // ^
static Bool_t _uo_f_negate(const CodeVal_t *, SelExprPtr_t, SelExprPtr_t );  // !
static Bool_t _uo_f_dereference(const CodeVal_t *, SelExprPtr_t, SelExprPtr_t ); // leaf

/* Allocates new binary expression operator */
static SelExpr_t * new_expr( Workspace_t *, SelExpr_t *, binop_f_t, SelExpr_t * );
/* Retrieves numerical value from values array according to getter */
static CodeVal_t get_value( const CodeVal_t * values, Getter g );

#line 330 "ds-dsl.tab.c" /* yacc.c:359  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

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

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
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
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
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
         || (defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL \
             && defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
  YYLTYPE yyls_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE) + sizeof (YYLTYPE)) \
      + 2 * YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  17
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   30

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  19
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  5
/* YYNRULES -- Number of rules.  */
#define YYNRULES  16
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  26

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   273

#define YYTRANSLATE(YYX)                                                \
  ((unsigned) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
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
      15,    16,    17,    18
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   196,   196,   198,   202,   209,   211,   218,   225,   232,
     242,   256,   257,   258,   259,   260,   261
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 1
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "L_STR", "T_LBC", "T_RBC", "T_EXCLMM",
  "T_GT", "T_GTE", "T_LT", "T_LTE", "T_EQ", "T_NE", "T_AMP", "T_PIPE",
  "T_CAP", "T_UNKNOWN_IDENTIFIER", "T_INVALID_VALUE", "T_VAL_REF_TOKEN",
  "$accept", "toplev", "selExpr", "ftCompExpr", "cmpOp", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273
};
# endif

#define YYPACT_NINF -5

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-5)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  0

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int8 yypact[] =
{
       1,    -5,     2,     2,    14,     3,    -4,    -5,    -1,    -4,
      -5,    -5,    -5,    -5,    -5,    -5,    -3,    -5,     2,     2,
       2,    -5,    -5,    -5,    15,    16
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,     2,     0,     0,     0,     0,     3,     4,     0,     6,
      11,    12,    13,    14,    15,    16,     0,     1,     0,     0,
       0,     5,    10,     7,     8,     9
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
      -5,    -5,    -2,    -5,    -5
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int8 yydefgoto[] =
{
      -1,     5,     6,     7,    16
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_uint8 yytable[] =
{
       8,     9,     1,    17,    21,     2,     2,     3,     3,    18,
      19,    20,    18,    19,    20,    22,    23,    24,    25,     4,
       4,    10,    11,    12,    13,    14,    15,     0,    18,    18,
      20
};

static const yytype_int8 yycheck[] =
{
       2,     3,     1,     0,     5,     4,     4,     6,     6,    13,
      14,    15,    13,    14,    15,    18,    18,    19,    20,    18,
      18,     7,     8,     9,    10,    11,    12,    -1,    13,    13,
      15
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     1,     4,     6,    18,    20,    21,    22,    21,    21,
       7,     8,     9,    10,    11,    12,    23,     0,    13,    14,
      15,     5,    18,    21,    21,    21
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    19,    20,    20,    21,    21,    21,    21,    21,    21,
      22,    23,    23,    23,    23,    23,    23
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     1,     3,     2,     3,     3,     3,
       3,     1,     1,     1,     1,     1,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
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
      yyerror (&yylloc, ws, yyscanner, YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256


/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

#ifndef YYLLOC_DEFAULT
# define YYLLOC_DEFAULT(Current, Rhs, N)                                \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).first_line   = YYRHSLOC (Rhs, 1).first_line;        \
          (Current).first_column = YYRHSLOC (Rhs, 1).first_column;      \
          (Current).last_line    = YYRHSLOC (Rhs, N).last_line;         \
          (Current).last_column  = YYRHSLOC (Rhs, N).last_column;       \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).first_line   = (Current).last_line   =              \
            YYRHSLOC (Rhs, 0).last_line;                                \
          (Current).first_column = (Current).last_column =              \
            YYRHSLOC (Rhs, 0).last_column;                              \
        }                                                               \
    while (0)
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K])


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


/* YY_LOCATION_PRINT -- Print the location on the stream.
   This macro was not mandated originally: define only if we know
   we won't break user code: when these are the locations we know.  */

#ifndef YY_LOCATION_PRINT
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL

/* Print *YYLOCP on YYO.  Private, do not rely on its existence. */

YY_ATTRIBUTE_UNUSED
static unsigned
yy_location_print_ (FILE *yyo, YYLTYPE const * const yylocp)
{
  unsigned res = 0;
  int end_col = 0 != yylocp->last_column ? yylocp->last_column - 1 : 0;
  if (0 <= yylocp->first_line)
    {
      res += YYFPRINTF (yyo, "%d", yylocp->first_line);
      if (0 <= yylocp->first_column)
        res += YYFPRINTF (yyo, ".%d", yylocp->first_column);
    }
  if (0 <= yylocp->last_line)
    {
      if (yylocp->first_line < yylocp->last_line)
        {
          res += YYFPRINTF (yyo, "-%d", yylocp->last_line);
          if (0 <= end_col)
            res += YYFPRINTF (yyo, ".%d", end_col);
        }
      else if (0 <= end_col && yylocp->first_column < end_col)
        res += YYFPRINTF (yyo, "-%d", end_col);
    }
  return res;
 }

#  define YY_LOCATION_PRINT(File, Loc)          \
  yy_location_print_ (File, &(Loc))

# else
#  define YY_LOCATION_PRINT(File, Loc) ((void) 0)
# endif
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value, Location, ws, yyscanner); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, Workspace_t * ws, yyscan_t yyscanner)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  YYUSE (yylocationp);
  YYUSE (ws);
  YYUSE (yyscanner);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep, YYLTYPE const * const yylocationp, Workspace_t * ws, yyscan_t yyscanner)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  YY_LOCATION_PRINT (yyoutput, *yylocationp);
  YYFPRINTF (yyoutput, ": ");
  yy_symbol_value_print (yyoutput, yytype, yyvaluep, yylocationp, ws, yyscanner);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
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
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, YYLTYPE *yylsp, int yyrule, Workspace_t * ws, yyscan_t yyscanner)
{
  unsigned long yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                       , &(yylsp[(yyi + 1) - (yynrhs)])                       , ws, yyscanner);
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, yylsp, Rule, ws, yyscanner); \
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
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
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
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
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
            /* Fall through.  */
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

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
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
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

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
      int yyn = yypact[*yyssp];
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
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
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
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
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
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep, YYLTYPE *yylocationp, Workspace_t * ws, yyscan_t yyscanner)
{
  YYUSE (yyvaluep);
  YYUSE (yylocationp);
  YYUSE (ws);
  YYUSE (yyscanner);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/*----------.
| yyparse.  |
`----------*/

int
yyparse (Workspace_t * ws, yyscan_t yyscanner)
{
/* The lookahead symbol.  */
int yychar;


/* The semantic value of the lookahead symbol.  */
/* Default value used for initialization, for pacifying older GCCs
   or non-GCC compilers.  */
YY_INITIAL_VALUE (static YYSTYPE yyval_default;)
YYSTYPE yylval YY_INITIAL_VALUE (= yyval_default);

/* Location data for the lookahead symbol.  */
static YYLTYPE yyloc_default
# if defined YYLTYPE_IS_TRIVIAL && YYLTYPE_IS_TRIVIAL
  = { 1, 1, 1, 1 }
# endif
;
YYLTYPE yylloc = yyloc_default;

    /* Number of syntax errors so far.  */
    int yynerrs;

    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.
       'yyls': related to locations.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    /* The location stack.  */
    YYLTYPE yylsa[YYINITDEPTH];
    YYLTYPE *yyls;
    YYLTYPE *yylsp;

    /* The locations where the error started and ended.  */
    YYLTYPE yyerror_range[3];

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;
  YYLTYPE yyloc;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N), yylsp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yylsp = yyls = yylsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  yylsp[0] = yylloc;
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;
        YYLTYPE *yyls1 = yyls;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yyls1, yysize * sizeof (*yylsp),
                    &yystacksize);

        yyls = yyls1;
        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
        YYSTACK_RELOCATE (yyls_alloc, yyls);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;
      yylsp = yyls + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

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
      yychar = yylex (&yylval, &yylloc, ws, yyscanner);
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

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END
  *++yylsp = yylloc;
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
| yyreduce -- Do a reduction.  |
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

  /* Default location. */
  YYLLOC_DEFAULT (yyloc, (yylsp - yylen), yylen);
  yyerror_range[1] = yyloc;
  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 197 "ds-dsl.y" /* yacc.c:1651  */
    { ws->root = NULL; return NA64DP_DSUL_TRANSLATION_ERROR; }
#line 1522 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 3:
#line 199 "ds-dsl.y" /* yacc.c:1651  */
    { ws->root = (yyvsp[0].selectionBinOp); }
#line 1528 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 4:
#line 203 "ds-dsl.y" /* yacc.c:1651  */
    {   (yyval.selectionBinOp) = new_expr(ws, (SelExpr_t *) (yyvsp[0].cmpExpr), _uo_f_dereference, 0x0);
                if( NULL == (yyval.selectionBinOp) ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
#line 1539 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 5:
#line 210 "ds-dsl.y" /* yacc.c:1651  */
    {   (yyval.selectionBinOp) = (yyvsp[-1].selectionBinOp); }
#line 1545 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 6:
#line 212 "ds-dsl.y" /* yacc.c:1651  */
    {   (yyval.selectionBinOp) = new_expr(ws, (SelExpr_t *) (yyvsp[0].selectionBinOp), _uo_f_negate, 0x0);
                if( NULL == (yyval.selectionBinOp) ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
#line 1556 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 7:
#line 219 "ds-dsl.y" /* yacc.c:1651  */
    {   (yyval.selectionBinOp) = new_expr(ws, (yyvsp[-2].selectionBinOp), _bo_f_and, (yyvsp[0].selectionBinOp));
                if( NULL == (yyval.selectionBinOp) ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
#line 1567 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 8:
#line 226 "ds-dsl.y" /* yacc.c:1651  */
    {   (yyval.selectionBinOp) = new_expr(ws, (yyvsp[-2].selectionBinOp), _bo_f_or, (yyvsp[0].selectionBinOp));
                if( NULL == (yyval.selectionBinOp) ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
#line 1578 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 9:
#line 233 "ds-dsl.y" /* yacc.c:1651  */
    { 
                (yyval.selectionBinOp) = new_expr(ws, (yyvsp[-2].selectionBinOp), _bo_f_xor, (yyvsp[0].selectionBinOp));
                if( NULL == (yyval.selectionBinOp) ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
#line 1590 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 10:
#line 243 "ds-dsl.y" /* yacc.c:1651  */
    { 
              if( ws->cursor + sizeof(ComparisonExpression_t) > ws->bufferEnd ) {
                yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                return NA64DP_DSUL_BUFFER_EXHAUSTED;
              }
              (yyval.cmpExpr) = (ComparisonExpression_t*) ws->cursor;
              ws->cursor += sizeof(ComparisonExpression_t);
              (yyval.cmpExpr)->left = (yyvsp[-2].valRef);
              (yyval.cmpExpr)->cmp_f = (yyvsp[-1].cmpOpFunc);
              (yyval.cmpExpr)->right = (yyvsp[0].valRef);
            }
#line 1606 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 11:
#line 256 "ds-dsl.y" /* yacc.c:1651  */
    { (yyval.cmpOpFunc) = _cmp_f_gt; }
#line 1612 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 12:
#line 257 "ds-dsl.y" /* yacc.c:1651  */
    { (yyval.cmpOpFunc) = _cmp_f_ge; }
#line 1618 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 13:
#line 258 "ds-dsl.y" /* yacc.c:1651  */
    { (yyval.cmpOpFunc) = _cmp_f_lt; }
#line 1624 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 14:
#line 259 "ds-dsl.y" /* yacc.c:1651  */
    { (yyval.cmpOpFunc) = _cmp_f_le; }
#line 1630 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 15:
#line 260 "ds-dsl.y" /* yacc.c:1651  */
    { (yyval.cmpOpFunc) = _cmp_f_eq; }
#line 1636 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;

  case 16:
#line 261 "ds-dsl.y" /* yacc.c:1651  */
    { (yyval.cmpOpFunc) = _cmp_f_ne; }
#line 1642 "ds-dsl.tab.c" /* yacc.c:1651  */
    break;


#line 1646 "ds-dsl.tab.c" /* yacc.c:1651  */
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
  *++yylsp = yyloc;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

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
      yyerror (&yylloc, ws, yyscanner, YY_("syntax error"));
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
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
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
        yyerror (&yylloc, ws, yyscanner, yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }

  yyerror_range[1] = yylloc;

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
                      yytoken, &yylval, &yylloc, ws, yyscanner);
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

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

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

      yyerror_range[1] = *yylsp;
      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp, yylsp, ws, yyscanner);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  yyerror_range[2] = yylloc;
  /* Using YYLLOC is tempting, but would change the location of
     the lookahead.  YYLOC is available though.  */
  YYLLOC_DEFAULT (yyloc, yyerror_range, 2);
  *++yylsp = yyloc;

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
  yyerror (&yylloc, ws, yyscanner, YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval, &yylloc, ws, yyscanner);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp, yylsp, ws, yyscanner);
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
#line 263 "ds-dsl.y" /* yacc.c:1910  */


#include "lex.yy.h"

/*
 * Comparison operators
 * ******************/
static Bool_t _cmp_f_eq( CodeVal_t a, CodeVal_t b ) { return a == b; }
static Bool_t _cmp_f_ne( CodeVal_t a, CodeVal_t b ) { return a != b; }
static Bool_t _cmp_f_gt( CodeVal_t a, CodeVal_t b ) { return a >  b; }
static Bool_t _cmp_f_ge( CodeVal_t a, CodeVal_t b ) { return a >= b; }
static Bool_t _cmp_f_lt( CodeVal_t a, CodeVal_t b ) { return a <  b; }
static Bool_t _cmp_f_le( CodeVal_t a, CodeVal_t b ) { return a <= b; }
/*static Bool_t _cmp_f_any( CodeVal_t a, CodeVal_t b ) { return 1; }*/

/* Performs feature comparison as defined within FeatureSelection node */
//static Bool_t eval_feature_cmp(const CodeVal_t * values, const ComparisonExpression_t * n) {
//    return n->cmp_f( get_value(values, n->valueCode), n->number );
//}
/* Performs evaluation of an expression node */
static Bool_t eval_expression( const CodeVal_t * data
                             , const SelExpr_t * expr) {
    return expr->bo( data, expr->a, expr->b);
}

/* Implements "AND" operation */
static Bool_t _bo_f_and(const CodeVal_t *v, const SelExpr_t * a, const SelExpr_t * b ) {
    return eval_expression(v, a) && eval_expression(v, b);
}
/* Implements "OR" operation */
static Bool_t _bo_f_or( const CodeVal_t *v, const SelExpr_t * a, const SelExpr_t * b ) {
    return eval_expression(v, a) || eval_expression(v, b);
}
/* Implements "XOR" operation */
static Bool_t _bo_f_xor(const CodeVal_t *v, const SelExpr_t * a, const SelExpr_t * b ) {
    return eval_expression(v, a) != eval_expression(v, b);
}
/* Implements unary "NOT" operation */
static Bool_t _uo_f_negate(const CodeVal_t *v, const SelExpr_t *a, const SelExpr_t *u ) {
    return ! eval_expression(v, a);
}
/* Implements leaf dereference operation */
static Bool_t _uo_f_dereference( const CodeVal_t * v
                               , const SelExpr_t * a
                               , const SelExpr_t * u /* unused */
                               ) {
    const ComparisonExpression_t * cex
        = (const ComparisonExpression_t *) a;
    CodeVal_t l = get_value( v, cex->left.valAddr )
            , r = get_value( v, cex->right.valAddr )
            ;
    return cex->cmp_f( l, r);
}

/* Allocates leaf node of AST from given feature selection */
static SelExpr_t *
new_expr( Workspace_t * ws
        , SelExpr_t * a
        , binop_f_t f
        , SelExpr_t * b ) {
    long offs = ws->cursor - ws->bufferStart;
    assert(offs);
    ws->cursor += sizeof(SelExpr_t);
    if( ws->cursor >= ws->bufferEnd ) {
        return 0x0;
    }

    SelExpr_t * sep = (SelExpr_t *)(ws->bufferStart + offs);
    sep->a  = a;
    sep->bo = f;
    sep->b  = b;

    return sep;
}

static CodeVal_t
get_value( const CodeVal_t * values, Getter g ) {
    if( (uintptr_t) g > NA64DP_DETID_MAX_DICT_OFFSET ) {
        /* is a function pointer (dynamic variable case) */
        /* TODO: how to get principal here? */
        return g(*(((const na64dp_dsul_PrincipalValue_t *) values) - 1));
    } else {
        /* is an index */
        return values[(uintptr_t) g];
    }
}

/* Definces length of the principal value block size in the units of CodeVal_t
 * type size. TODO: align it? */
#define PRINCIPAL_VALUE_OFFSET sizeof(na64dp_dsul_PrincipalValue_t)

/* Root node pointer */
#define NA64DP_HDR_ROOT(bgn) *((SelExpr_t**) bgn)
/* Extern functions begin pointer */
#define NA64DP_HDR_EX_F(bgn) *(na64dp_dsul_DetIDExternResolver**)           \
                             (bgn + sizeof(SelExpr_t*))
/* Static functions begin pointer */
#define NA64DP_HDR_ST_F(bgn) *((na64dp_dsul_DetIDSemanticGetter**)           \
                             (bgn + sizeof(SelExpr_t*)                      \
                                  + sizeof(na64dp_dsul_DetIDExternResolver*)))
/* Data block begin pointer (literals start) */
#define NA64DP_HDR_DATA(bgn) *((CodeVal_t**)                                \
                             (bgn + sizeof(SelExpr_t*)                      \
                                  + sizeof(na64dp_dsul_DetIDExternResolver*) \
                                  + sizeof(na64dp_dsul_DetIDSemanticGetter*)))
/* Extern values data block begin */
#define NA64DP_HDR_EXVL(bgn) *((CodeVal_t**)                                \
                             (bgn + sizeof(SelExpr_t*)                      \
                                  + sizeof(na64dp_dsul_DetIDExternResolver*) \
                                  + sizeof(na64dp_dsul_DetIDSemanticGetter*) \
                                  + sizeof(CodeVal_t*) ))
/* Static values data block begin */
#define NA64DP_HDR_STVL(bgn) *((CodeVal_t**)                                \
                             (bgn + sizeof(SelExpr_t*)                      \
                                  + sizeof(na64dp_dsul_DetIDExternResolver*) \
                                  + sizeof(na64dp_dsul_DetIDSemanticGetter*) \
                                  + 2*sizeof(CodeVal_t*) ))
/* Buffer end pointer */
#define NA64DP_HDR_ENDP(bgn) *((char**) (bgn + sizeof(SelExpr_t*)           \
                                  + sizeof(na64dp_dsul_DetIDExternResolver*) \
                                  + sizeof(na64dp_dsul_DetIDSemanticGetter*) \
                                  + 3*sizeof(CodeVal_t*) ))

static void
dump_ft_arg_unbaked( FILE * stream
                   , ValueReference_t vRef
                   , const struct Symbol * symbols
                   , const struct na64dpsu_SymDefinition * getters ) {
    const struct Symbol * cSymPtr = symbols + vRef.symbOffset;
    if( NA64DP_DSUL_MAX_GETTERS == cSymPtr->defIdx ) {
        fprintf( stream, "[value=%d]"
               , cSymPtr->value );
        return;
    }
    const struct na64dpsu_SymDefinition * gte
                = getters + (cSymPtr->defIdx - 1);
    fprintf( stream, "[symbol #%d -> \"%s\"]"
           , vRef.symbOffset
           , gte->name );
}

static void
dump_ft_arg_baked( FILE * stream
                 , ValueReference_t vRef
                 , const struct Symbol * symbols
                 , const struct na64dpsu_SymDefinition * getters ) {
    if( (uintptr_t) vRef.valAddr > NA64DP_DETID_MAX_DICT_OFFSET ) {
        fprintf( stream, "[dyn.getter %p]", vRef.valAddr );
    } else {
        fprintf( stream, "[value #%lu]", (uintptr_t) vRef.valAddr );
    }
}

/** Dumps the composed predicate table to given stream */
static void
selexpr_dump( FILE * stream
            , const SelExpr_t * node
            , int cOffset
            , void (*nf)(FILE *
                        , ValueReference_t
                        , const struct Symbol *
                        , const struct na64dpsu_SymDefinition * )
            , const struct Symbol * symbols
            , const struct na64dpsu_SymDefinition * getters ) {
    if( !node ) return;
    for(int i = 0; i < cOffset; ++i ) { fputc(' ', stream); }
    fputs("+- ", stream);
    if( node->bo == _bo_f_and ) {
        fputs("AND", stream);
    } else if( node->bo == _bo_f_or ) {
        fputs("OR", stream);
    } else if( node->bo == _bo_f_xor ) {
        fputs("XOR", stream);
    } else if( node->bo == _uo_f_negate ) {
        fputs("NOT", stream);
    } else if( node->bo == _uo_f_dereference ) {
        fputs("EVAL ", stream);
    } else {
        fputs("??", stream);
    }
    if( _uo_f_dereference != node->bo ) {
        fputc('\n', stream);
        if( node->a ) {
            selexpr_dump( stream, node->a, cOffset + 1, nf, symbols, getters );
        }
        if( node->b ) {
            selexpr_dump( stream, node->b, cOffset + 1, nf, symbols, getters );
        }
        return;  /* exit binary op dump */
    }
    /* Dump dereference (terminative) node */
    assert( NULL == node->b );
    const ComparisonExpression_t * fs = (const ComparisonExpression_t *) node->a;
    nf( stream, fs->left, symbols, getters );
    if( fs->cmp_f == _cmp_f_eq ) {
        fputs(" `==` ", stream);
    } else if( fs->cmp_f == _cmp_f_ne ) {
        fputs(" `!=` ", stream);
    } else if( fs->cmp_f == _cmp_f_gt ) {
        fputs(" `>`  ", stream);
    } else if( fs->cmp_f == _cmp_f_ge ) {
        fputs(" `>=` ", stream);
    } else if( fs->cmp_f == _cmp_f_lt ) {
        fputs(" `<`  ", stream);
    } else if( fs->cmp_f == _cmp_f_le ) {
        fputs(" `<=` ", stream);
    } /*else if( fs->cmp_f == _cmp_f_any ) {
        fputs(" ~~ ", stream);
    } */else {
        fputs(" ?? ", stream);
    }
    nf( stream, fs->right, symbols, getters );
    fputc('\n', stream);
}

static void
dump_evaluation_buffer( FILE * stream
                      , const char * bgn, const char * bufferEnd ) {
    const char * buffer = bgn;
    SelExpr_t * root;
    na64dp_dsul_DetIDExternResolver * constGetters;
    na64dp_dsul_DetIDSemanticGetter * statGetters;
    CodeVal_t * literalsBegin
            , * constValsBegin
            , * statValsBegin, * statValsEnd;
    char * meaningfulBufferEnd;

    /* 1. Header block */
    fprintf( stream, "%04ld Header (%p):\n", buffer - bgn, bgn );
    fprintf( stream, "%04ld     root node: %p\n"
           , buffer - bgn
           , root = *((SelExpr_t**) buffer) );
    buffer += sizeof(SelExpr_t*);

    constGetters = *((na64dp_dsul_DetIDExternResolver**) buffer);
    fprintf( stream, "%04ld     extern-f begin: %p (%04ld)\n"
           , buffer - bgn, constGetters, ((char*) constGetters) - bgn );
    buffer += sizeof(na64dp_dsul_DetIDExternResolver*);

    statGetters = *((na64dp_dsul_DetIDSemanticGetter**) buffer);
    fprintf( stream, "%04ld     static-f begin: %p (%04ld)\n"
           , buffer - bgn, statGetters, ((char*) statGetters) - bgn );
    buffer += sizeof(na64dp_dsul_DetIDSemanticGetter*);

    literalsBegin = *((CodeVal_t**) buffer);
    fprintf( stream, "%04ld     literals begin: %p (%04ld)\n"
           , buffer - bgn, literalsBegin, ((char*) literalsBegin) - bgn );
    buffer += sizeof(CodeVal_t*);

    constValsBegin = *((CodeVal_t**) buffer);
    fprintf( stream, "%04ld     extern vals. begin: %p (%04ld)\n"
           , buffer - bgn, constValsBegin, ((char*) constValsBegin) - bgn );
    buffer += sizeof(CodeVal_t*);

    statValsBegin = *((CodeVal_t**) buffer);
    fprintf( stream, "%04ld     static vals. begin: %p (%04ld)\n"
           , buffer - bgn, statValsBegin, ((char*) statValsBegin) - bgn );
    buffer += sizeof(CodeVal_t*);

    meaningfulBufferEnd = *((char**) buffer);
    fprintf( stream, "%04ld     buffer meaningful data end: %p (%04ld)\n"
           , buffer - bgn, meaningfulBufferEnd, ((char*)meaningfulBufferEnd) - bgn );
    buffer += sizeof(char*);

    if(bufferEnd) {
        /* check header for some obvious errors that may cause segfaults */
        #define ASSURE_PTR_IS_IN_RANGE( nm ) {                  \
            const char * ptr = (const char *) nm;               \
            if( ptr < bgn || ptr >= bufferEnd ) {               \
                fprintf( stream, "Bad value for " #nm "\n" );   \
                return;                                         \
            }                                                   \
        }
        ASSURE_PTR_IS_IN_RANGE( root );
        assert( root == NA64DP_HDR_ROOT(bgn) );
        ASSURE_PTR_IS_IN_RANGE( constGetters );
        assert( constGetters == NA64DP_HDR_EX_F(bgn) );
        ASSURE_PTR_IS_IN_RANGE( statGetters );
        assert( statGetters == NA64DP_HDR_ST_F(bgn) );
        ASSURE_PTR_IS_IN_RANGE( literalsBegin );
        assert( literalsBegin == NA64DP_HDR_DATA(bgn) );
        ASSURE_PTR_IS_IN_RANGE( constValsBegin );
        assert( constValsBegin == NA64DP_HDR_EXVL(bgn) );
        ASSURE_PTR_IS_IN_RANGE( statValsBegin );
        assert( statValsBegin == NA64DP_HDR_STVL(bgn) );
        ASSURE_PTR_IS_IN_RANGE( meaningfulBufferEnd );
        assert( meaningfulBufferEnd == NA64DP_HDR_ENDP(bgn) );
        #undef ASSURE_PTR_IS_IN_RANGE
    }
    /* 2. Nodes */
    fprintf( stream, "%04ld Nodes:\n", buffer - bgn );
    selexpr_dump( stream, root, 5, dump_ft_arg_baked
                , NULL, NULL );
    /* 3. Functions */
    buffer = (const char *) constGetters;
    fprintf( stream, "%04ld Functions:\n"
           , ((const char *) constGetters) - bgn );
    /* 3.1 Extern getters */
    size_t vOffset = 0;
    for( na64dp_dsul_DetIDExternResolver * cConstGetter = constGetters
       ; (void *) cConstGetter < (void *) statGetters
       ; ++cConstGetter
       , ++vOffset
       , buffer += sizeof(na64dp_dsul_DetIDExternResolver *) + sizeof(char*) ) {
        const char * varNameAddr = *((char **) (cConstGetter+1));
        fprintf( stream, "%04ld     extern getter for \"%s\"[%04lu]: <%p> :: %p (sets %04ld)\n"
               , ((const char *) cConstGetter) - bgn
               , varNameAddr
               , varNameAddr - bgn
               , *cConstGetter
               , constValsBegin + vOffset
               , ((const char *) (constValsBegin + vOffset)) - bgn );
        cConstGetter = (na64dp_dsul_DetIDExternResolver *) ( ((char*) (cConstGetter)) + sizeof(char*) );
    }
    /* 3.2 Static getters */
    statValsEnd = statValsBegin;
    for( na64dp_dsul_DetIDSemanticGetter * cStaticGetter = statGetters
       ; *cStaticGetter
       ; ++cStaticGetter
       , ++statValsEnd
       , ++vOffset
       , buffer += sizeof(na64dp_dsul_DetIDSemanticGetter *) ) {
        fprintf( stream, "%04ld     stat. getter: <%p> :: %p (sets %04ld)\n"
               , ((const char *) cStaticGetter) - bgn
               , *cStaticGetter
               , statValsBegin + vOffset
               , ((const char *) (statValsBegin + vOffset)) - bgn );
    }
    fprintf( stream, "%04ld NULL (functions block sentinel)\n"
           , buffer - bgn );
    buffer += sizeof(na64dp_dsul_DetIDSemanticGetter *);

    fprintf( stream, "%04ld Principal value\n", buffer - bgn );
    buffer += PRINCIPAL_VALUE_OFFSET;

    if( (void *) buffer != (void *) literalsBegin ) {
        fprintf( stream
               , "Integrity error:"
                 " literals block start written in header (%p, %04ld)"
                 " does not match computed offset (%p, %04ld).\n"
               , literalsBegin, ((const char *) literalsBegin) - bgn
               , buffer, buffer - bgn );
        return;
    }
    /* Data block */
    /* 4 Principal value */
    fprintf( stream, "%04ld Data (%p):\n", buffer - bgn, buffer );
    /* 5.1 Literals */
    for( CodeVal_t * cLitPtr = (CodeVal_t *) buffer
       ; cLitPtr != constValsBegin
       ; ++cLitPtr, buffer += sizeof(CodeVal_t) ) {
        fprintf( stream, "%04ld     Value #%ld -- literal `%d'\n"
            , buffer - bgn
            , cLitPtr - literalsBegin
            , *cLitPtr );
    }
    /* 5.2 Extern values */
    for( CodeVal_t * ccValPtr = constValsBegin
       ; ccValPtr != statValsBegin
       ; ++ccValPtr, buffer += sizeof(CodeVal_t) ) {
        fprintf( stream, "%04ld     Value #%ld -- extern `%d'\n"
            , buffer - bgn
            , ccValPtr - literalsBegin
            , *ccValPtr );
    }
    /* 5.3 Static values */
    for( CodeVal_t * csValPtr = statValsBegin
       ; csValPtr < statValsEnd
       ; ++csValPtr, buffer += sizeof(CodeVal_t) ) {
        fprintf( stream, "%04ld     Value #%ld -- static `%d'\n"
               , ((const char *) csValPtr) - bgn
               , csValPtr - literalsBegin
               , *csValPtr );
    }
    /* 6. Extern values names */
    fprintf( stream, "%04ld Extern values (%p):\n", buffer - bgn, buffer );
    while( buffer < meaningfulBufferEnd ) {
        fprintf( stream, "%04ld     Extern symbol name \"%s\"\n"
               , buffer - bgn
               , buffer );
        buffer += strlen(buffer) + 1;
    }
    fprintf( stream, "%04ld end\n"
           , meaningfulBufferEnd - bgn );
}

void
na64dp_dsul_dump( const char * buffer, FILE * stream ) {
    dump_evaluation_buffer( stream, buffer, NULL );
}


static void
resolve_value_references( ValueReference_t * ve
                        , Workspace_t * ws ) {
    /* Up to this point, each symbol must have valid offset value */
    struct Symbol * cSymPtr = ws->symbols + ve->symbOffset;
    const struct na64dpsu_SymDefinition * gte = NULL;
    if( cSymPtr->defIdx == NA64DP_DSUL_MAX_GETTERS ) {
        /* Copy literal value
         * TODO: consider removing this redundant copying or putting assertion
         * here as we already did it in main function initialization */
        ((CodeVal_t *) (ws->dataBlockBgn))[cSymPtr->offset]
            = cSymPtr->value;
    } else {
        gte = ws->gettersTable + (cSymPtr->defIdx - 1);
    }
    if( gte && NA64DP_DSUL_DYNAMIC_GETTER_CHAR == gte->name[0] ) {
        /* For dynamic getter -- set callback address */
        ve->valAddr = gte->callback.get;
    } else {
        /* otherwise, for literals, const and static -- set offset relative
         * to data block begin */
        ve->symbOffset = cSymPtr->offset;
    }
}

static void
resolve_value_references_in_expr( SelExpr_t * node
                                , Workspace_t * ws ) {
    if( _uo_f_dereference != node->bo ) {
        if( node->a ) {
            resolve_value_references_in_expr( node->a, ws );
        }
        if( node->b ) {
            resolve_value_references_in_expr( node->b, ws );
        }
        return;
    }
    /* Resolve symbols on terminative node */
    ComparisonExpression_t * fs = (ComparisonExpression_t *) node->a;
    resolve_value_references( &(fs->left), ws );
    resolve_value_references( &(fs->right), ws );
}

static void
resolve_nodes( Workspace_t * ws ) {
    resolve_value_references_in_expr( ws->root, ws );
}


/* Main expression translate and "bake" function */
int
na64dp_dsul_compile_detector_selection( char * strexpr
             , const struct na64dpsu_SymDefinition * gettersTable
             , void * externalResolver
             , char * wsBuffer, int bufferSize
             , char * errorMsgBuffer, int errMsgBufferSize
             , FILE * dbgStream ) {
    #define ASSURE_BUFFER if( ws.cursor >= ws.bufferEnd ) \
        return NA64DP_DSUL_BUFFER_EXHAUSTED  /* ERROR: bad or insufficient buffer */

    if( !strexpr ) return NA64DP_DSUL_STREXPR_BAD;  /* ERROR: bad argument expression */
    if( !wsBuffer || !bufferSize ) return NA64DP_DSUL_BUFFER_EXHAUSTED;  /* ERROR: bad or insufficient buffer */

    /*
     * Initialize workspace
     */
    Workspace_t ws;
    ws.dbgStream = dbgStream;
    ws.bufferStart = ws.cursor = wsBuffer;
    ws.externResolverPtr = externalResolver;
    /* Reserve space for header */
    #define DECLARE_HEADER_FIELD( type, varName )   \
        type * varName = (type*) (ws.cursor);      \
        ws.cursor += sizeof(type)
    DECLARE_HEADER_FIELD( SelExpr_t*, rootPtr );
    DECLARE_HEADER_FIELD( na64dp_dsul_DetIDExternResolver, constGettersStartPtr );
    DECLARE_HEADER_FIELD( na64dp_dsul_DetIDSemanticGetter, staticGettersStartPtr );
    DECLARE_HEADER_FIELD( CodeVal_t*, literalsStartPtr );
    DECLARE_HEADER_FIELD( CodeVal_t*, externValsStartPtr );
    DECLARE_HEADER_FIELD( CodeVal_t*, statValsStartPtr );
    DECLARE_HEADER_FIELD( char*, bufferEndPtr );
    #undef DECLARE_HEADER_FIELD
    char * cExternSymbolName;
    ws.bufferEnd = wsBuffer + bufferSize;
    ASSURE_BUFFER;

    ws.gettersTable = gettersTable;
    ws.root = NULL;
    if( errMsgBufferSize ) {
        ws.errMsg = errorMsgBuffer;
        ws.errMsgSize = errMsgBufferSize;
        ws.errMsg[0] = '\0';
    } else {
        ws.errMsg = NULL;
        ws.errMsgSize = 0;
    }
    /* Allocate static values temporary cache */
    ws.symbols = malloc( NA64DP_DETID_MAX_DICT_OFFSET*sizeof(struct Symbol) );
    bzero(ws.symbols, NA64DP_DETID_MAX_DICT_OFFSET*sizeof(struct Symbol));
    if( NULL == ws.symbols ) return NA64DP_DSUL_MEMORY_ERROR;  /* ERROR: internal temporary heap allocation error */

    /*
     * Parse expression
     */
    void * scannerPtr;
    yylex_init(&scannerPtr);
    struct yy_buffer_state * buffer = yy_scan_string(strexpr, scannerPtr);
    /* - The value returned by yyparse is 0 if parsing was successful (return
     * is due to end-of-input).
     * - The value is 1 if parsing failed because of invalid input, i.e.,
     * input that contains a syntax error or that causes YYABORT to be
     * invoked.
     * - The value is 2 if parsing failed due to memory exhaustion. */
    int rc = yyparse(&ws, scannerPtr);
    if( dbgStream && rc ) {
        fprintf( dbgStream
               , "Failed to translate selector expression because of %s\n"
               , ws.errMsg );
    }
    yy_delete_buffer( buffer, scannerPtr );
    yylex_destroy(scannerPtr);
    if( rc ) return rc;
    /* After lexical analysis is done, we have to "bake" the workspace object:
     *  by resolving all the symbols gathered in a table */
    if( dbgStream ) {
        fprintf( dbgStream
               , "After translation %zu bytes remain in buffer\n"
               , ws.bufferEnd - ws.cursor );
    }
    *rootPtr = ws.root;

    /*
     * Bake workspace
     */
    /* Count objects */
    size_t nLiterals = 0
         , nExternSyms = 0
         , nStatSyms = 0
         , nDynSyms = 0
         ;
    for( struct Symbol * cSymPtr = ws.symbols
       ; 0 != cSymPtr->defIdx
       ; ++cSymPtr ) {
        /* if it is a constant literal */
        if( NA64DP_DSUL_MAX_GETTERS == cSymPtr->defIdx ) {
            /* Literals offsets are not affected by principal value block */
            cSymPtr->offset = nLiterals++;
            continue;
        }
        assert( cSymPtr->defIdx < NA64DP_DSUL_MAX_GETTERS );
        /* resolve getters table entry by its index corresponding to
         * current symbol; here we do -1 to obtain REAL getter index
         * as zero was reserved for special meaning */
        const struct na64dpsu_SymDefinition * gte
                = ws.gettersTable + (cSymPtr->defIdx - 1);
        if( NA64DP_DSUL_EXTERN_RESOLVER_CHAR == gte->name[0] ) {
            ++nExternSyms;
        } else if( NA64DP_DSUL_STATIC_GETTER_CHAR == gte->name[0] ) {
            ++nStatSyms;
        } else if( NA64DP_DSUL_DYNAMIC_GETTER_CHAR == gte->name[0] ) {
            ++nDynSyms;
        } else {
            return NA64DP_DSUL_BAD_GETTERS_TABLE;  /* ERROR: bad type marker in getters table */
        }
    }

    if( dbgStream ) {
        fprintf( dbgStream
               , "Symbol stats: %zu literals, %zu extern, %zu static"
                 " getters, %zu dynamic getters. Nodes tree before baking:\n"
               , nLiterals, nExternSyms, nStatSyms, nDynSyms );
        selexpr_dump( dbgStream
                    , ws.root
                    , 4
                    , dump_ft_arg_unbaked
                    , ws.symbols
                    , ws.gettersTable );
    }

    /*
     * Resolve nodes
     */
    /* 1.4 - Data block begin pointer */
    ws.dataBlockBgn
        = *literalsStartPtr
        = (CodeVal_t *) (ws.cursor
                        + (sizeof(na64dp_dsul_DetIDExternResolver) + sizeof(const char*))*nExternSyms
                        + sizeof(na64dp_dsul_DetIDSemanticGetter)*(nStatSyms+1)
                        + PRINCIPAL_VALUE_OFFSET
                        );
    /* 1.5 - Extern values data block start */
    *externValsStartPtr = *literalsStartPtr + nLiterals;
    /* 1.6 - Static values data block start */
    *statValsStartPtr = *externValsStartPtr + nExternSyms;
    /* 1.7 - Buffer end */
    *bufferEndPtr = cExternSymbolName
            = (char*) (*statValsStartPtr + nStatSyms);
    /*
     * Fill functions lists selected for the expression
     */
    CodeVal_t dataBlockValueOffset = (CodeVal_t) nLiterals;
    /* 3.1 - Set external resolving getters wrt "symbols" table */
    *constGettersStartPtr = (na64dp_dsul_DetIDExternResolver) ws.cursor;
    for( struct Symbol * cSymPtr = ws.symbols
       ; 0 != cSymPtr->defIdx
       ; ++cSymPtr ) {
        if( NA64DP_DSUL_MAX_GETTERS == cSymPtr->defIdx ) continue;  /* skip literals */
        const struct na64dpsu_SymDefinition * gte
                = ws.gettersTable + (cSymPtr->defIdx - 1);
        if( NA64DP_DSUL_EXTERN_RESOLVER_CHAR == gte->name[0] ) {
            *((na64dp_dsul_DetIDExternResolver*) ws.cursor) = gte->callback.resolve_name;
            ws.cursor += sizeof(na64dp_dsul_DetIDExternResolver);
            ASSURE_BUFFER;
            cSymPtr->offset = dataBlockValueOffset++;
            assert( cSymPtr->name );
            size_t nameLength = strlen(cSymPtr->name) + 1;
            if( cExternSymbolName + nameLength > ws.bufferEnd ) {
                return NA64DP_DSUL_BUFFER_EXHAUSTED; /* ERROR: bad or insufficient buffer */
            }
            *((const char **) ws.cursor) = strcpy( cExternSymbolName, cSymPtr->name );
            cExternSymbolName += nameLength;
            *bufferEndPtr += nameLength;
            free( cSymPtr->name );
            cSymPtr->name = NULL;
            ws.cursor += sizeof(char*);
            ASSURE_BUFFER;
        }
    }
    /* 3.2 - Set static getters wrt "symbols" table */
    *staticGettersStartPtr = (na64dp_dsul_DetIDSemanticGetter) ws.cursor;
    for( struct Symbol * cSymPtr = ws.symbols
       ; 0 != cSymPtr->defIdx
       ; ++cSymPtr ) {
        if( NA64DP_DSUL_MAX_GETTERS == cSymPtr->defIdx ) continue;  /* skip literals */
        const struct na64dpsu_SymDefinition * gte
                = ws.gettersTable + (cSymPtr->defIdx - 1);
        if( NA64DP_DSUL_STATIC_GETTER_CHAR == gte->name[0] ) {
            *((na64dp_dsul_DetIDSemanticGetter*) ws.cursor) = gte->callback.get;
            ws.cursor += sizeof(na64dp_dsul_DetIDSemanticGetter);
            ASSURE_BUFFER;
            cSymPtr->offset = dataBlockValueOffset++;
            if( dbgStream ) {
                fprintf( dbgStream, "#%ld symbol's offset set to %d\n"
                       , cSymPtr - ws.symbols, cSymPtr->offset );
            }
        }
    }
    /* Write terminative NULL ptr for static getter block */
    *((na64dp_dsul_DetIDSemanticGetter*) ws.cursor) = NULL;
    ws.cursor += sizeof(na64dp_dsul_DetIDSemanticGetter);
    ASSURE_BUFFER;
    
    /*
     * Fill data block
     */

    /* 4 - Principal */
    ws.cursor += PRINCIPAL_VALUE_OFFSET;
    ASSURE_BUFFER;
    /* Copy literal values */
    assert(*literalsStartPtr == (CodeVal_t*) ws.cursor);
    for( struct Symbol * cSymPtr = ws.symbols
       ; 0 != cSymPtr->defIdx
       ; ++cSymPtr ) {
        if( NA64DP_DSUL_MAX_GETTERS != cSymPtr->defIdx ) continue;  /* skip non-literals */
        *((CodeVal_t*) ws.cursor) = cSymPtr->value;
        if( dbgStream ) {
            fprintf( dbgStream, "Writing literal of symbol #%ld (`%d') at %04lu-th byte\n"
                   , cSymPtr - ws.symbols
                   , cSymPtr->value
                   , ws.cursor - wsBuffer );
        }
        ws.cursor += sizeof(CodeVal_t);
        ASSURE_BUFFER;
    }

    resolve_nodes( &ws );
    free(ws.symbols);

    if( dbgStream ) {
        dump_evaluation_buffer( dbgStream, wsBuffer, ws.bufferEnd );
    }

    return 0;  /* ERROR: ok */
}

void
yyerror( YYLTYPE * yylloc
       , Workspace_t * ws
       , yyscan_t yyscanner
       , const char* s
       ) {
    if( ws->errMsgSize && '\0' != ws->errMsg[0] ) {
        fprintf( stderr
               , "Last DSuL translation error message replaces previous one: \"%s\".\n"
               , ws->errMsg );
    }
    if( ws->errMsgSize ) {
        snprintf( ws->errMsg, ws->errMsgSize
                , "error at %d,%d:%d,%d: %s"
                , yylloc->first_line, yylloc->first_column
                , yylloc->last_line, yylloc->last_column
                , s );
    } else {
        fprintf( stderr, "DSuL translation error at %d,%d:%d,%d: %s\n"
               , yylloc->first_line, yylloc->first_column
               , yylloc->last_line, yylloc->last_column
               , s );
    }
}

/* 
 * Context mutators
 */

void
na64dp_dsul_update_extern_symbols( char * evalBuffer
                                 , const void * obj ) {
    char * eg = (char*) NA64DP_HDR_EX_F(evalBuffer)
       , * sg = (char*) NA64DP_HDR_ST_F(evalBuffer);
    CodeVal_t * values = NA64DP_HDR_EXVL(evalBuffer);
    for(
       ; eg < sg
       ; eg += sizeof(na64dp_dsul_DetIDExternResolver*)
             + sizeof(char*)
       , ++values ) {
        na64dp_dsul_DetIDExternResolver getter
            = *((na64dp_dsul_DetIDExternResolver *) eg);
        char * symbolName
            = *((char**) (eg + sizeof(na64dp_dsul_DetIDExternResolver*)));
        #if 0
        if( dbgStream ) {
            fprintf( dbgStream
                  , "Extern getter %p of token \"%s\"[%04lu] updates %04lu\n"
                  , getter
                  , symbolName
                  , symbolName - evalBuffer
                  , ((char *) values) - evalBuffer );
        }
        #endif
        getter( symbolName, obj, values );
    }
}

void
na64dp_dsul_update_static_symbols( char * evalBuffer
                                 , na64dp_dsul_PrincipalValue_t pv ) {
    char * sg = (char *) NA64DP_HDR_ST_F(evalBuffer);
    CodeVal_t * value = NA64DP_HDR_STVL(evalBuffer);

    /* Set principal */
    *(((na64dp_dsul_PrincipalValue_t*) NA64DP_HDR_DATA(evalBuffer)) - 1) = pv;

    for( na64dp_dsul_DetIDSemanticGetter * sgPtr = (na64dp_dsul_DetIDSemanticGetter *) sg
       ; *sgPtr
       ; ++sgPtr, ++value ) {
        *value = (*sgPtr)( pv );
        #if 0
        fprintf( stdout
               , "xxx Static getter %p updated %04lu: %d\n"
               , *sgPtr
               , ((char *) value) - evalBuffer
               , *value );
        #endif
    }
}

int
na64dp_dsul_eval( const char * buffer ) {
    return eval_expression( NA64DP_HDR_DATA(buffer)
                          , NA64DP_HDR_ROOT(buffer) );
}

