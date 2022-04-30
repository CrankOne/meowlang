/* A Bison parser, made by GNU Bison 3.1.  */

/* Bison interface for Yacc-like parsers in C

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
#line 1 "ds-dsl.y" /* yacc.c:1913  */


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

#line 155 "ds-dsl.tab.h" /* yacc.c:1913  */

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
#line 164 "ds-dsl.y" /* yacc.c:1913  */

    const char * strID;
    cmp_f_t cmpOpFunc;
    binop_f_t binOpFunc;
    ValueReference_t valRef;
    ComparisonExpression_t * cmpExpr;
    SelExpr_t * selectionBinOp;

#line 195 "ds-dsl.tab.h" /* yacc.c:1913  */
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
#line 122 "ds-dsl.y" /* yacc.c:1913  */


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


#line 242 "ds-dsl.tab.h" /* yacc.c:1913  */

#endif /* !YY_NA64DPSU_DS_DSL_TAB_H_INCLUDED  */
