%code requires {

#include "meowlang_ast.h"
#include <stdio.h>

/** Temporary workspace structure
 *
 * This structure contains the entire parsing and lexical analysis context of
 * the expression.
 * */
typedef struct mwl_Workspace {
    /** Error message buffer */
    char * errMsg;
    unsigned int errMsgSize;
    /** Problematic fragment: first line, column, last line, column */
    uint32_t errPos[2][2];
    /* Expression root node's */
    struct mwl_ASTNode * root;
    /* Parser debug messages stream (may be nullptr) */
    FILE * dbgStream;
} mwl_Workspace_t;

/* Compile function error codes */
#define MEOW_STREXPR_BAD         -1  /* bad argument expression NULL or not a string */
#define MEOW_BUFFER_EXHAUSTED    -2  /* bad or insufficient buffer */
#define MEOW_MEMORY_ERROR        -3  /* heap allocation failure */
#define MEOW_TRANSLATION_ERROR   -4  /* syntax/parser/lexer error */
#define MEOW_BAD_GETTERS_TABLE   -5  /* bad getter table entry */

#define MWL_INCOMPATIBLE_TYPE    -100   /* incompatible types */

typedef void *yyscan_t;  /* circumvent circular dep: YACC/BISON does not know this type */
}

%define api.pure full
%define parse.error verbose
%locations

%lex-param {mwl_Workspace_t * ws}
%lex-param {yyscan_t yyscanner}

%parse-param {mwl_Workspace_t * ws}
%parse-param {yyscan_t yyscanner}

%code provides {

#include "meowlang_ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define YY_DECL                             \
    int meowlang_lex( YYSTYPE* yylval_param \
                    , YYLTYPE* yylloc_param \
                    , mwl_Workspace_t * ws \
                    , yyscan_t yyscanner )
YY_DECL;

void meowlang_error( struct YYLTYPE *
                   , mwl_Workspace_t *
                   , yyscan_t yyscanner
                   , const char* );

}

%union {
    const char * strID;
    mwl_OpCode_t opCode;
    struct mwl_ConstVal constval;
    struct mwl_ASTNode astNode;
    /* ... */
}

%token<strID> L_STR
/* Brackets */
%token T_LBC T_RBC T_LSQBC T_RSQBC T_LCRLBC T_RCRLBC
/* Math */
%token T_PLUS T_MINUS T_ASTERISK T_SLASH T_ASTERISK2
/* Logic */
%token T_AMP2 T_PIPE2 T_EXCLMM
/* Bitwise logic */
%token T_NE T_AMP T_PIPE T_CIRCUMFLEX T_TILDE
/* Comparison */
%token T_GT T_GE T_LT T_LE T_EQ
/* Misc grammar and operations */
%token T_DOT T_REV_SLASH
%type<opCode> princOp binOpSym cmpOp logicOp
%type<astNode> value expr //selectExpr baseSelExpr
%token T_UNKNOWN_IDENTIFIER T_INVALID_VALUE T_FOREIGN_VALUE T_FOREIGN_CALL
%token<constval> T_CONSTVAL_TOKEN T_STRING_LITERAL

%nonassoc T_GT T_GTE T_LT T_LTE
%left T_EXCLMM
%left T_PIPE
%left T_CAP
%left T_AMP
//%left T_GT T_GTE T_LT T_LTE T_EQ T_NE
//%left T_LBC T_RBC

%start toplev

%%

     toplev : error
            { ws->root = NULL; return MEOW_TRANSLATION_ERROR; }
            | expr { ws->root = mwl_shallow_copy_node(&($1)); }
            ;

       expr : value
            | expr binOpSym value
            { struct mwl_ASTNode * a = mwl_shallow_copy_node(&($1))
                               , * b = mwl_shallow_copy_node(&($3))
                               ;
              $$.pl.asOp.a = a;
              $$.pl.asOp.code = $2;
              $$.pl.asOp.b = b;
              $$.nodeType = mwl_kOperation;
              char errBuf[128];
              $$.dataType = mwl_infer_type( a->dataType
                                          , $$.pl.asOp.code
                                          , b->dataType
                                          , errBuf, sizeof(errBuf)
                                          );
              if( ! $$.dataType ) {
                  yyerror( &yylloc, ws, NULL, errBuf );
                  return MWL_INCOMPATIBLE_TYPE;
              }
            }
            | T_LBC expr T_RBC { $$ = $2; }
            | T_EXCLMM T_LBC expr T_RBC { /* Logic negate */
              $$.pl.asOp.a = mwl_shallow_copy_node(&($3));
              $$.pl.asOp.b = NULL;
              $$.pl.asOp.code = kOp_LogicNegate;
              $$.nodeType = mwl_kOperation;
              /*if(  ) ... TODO: check that type can be logically-negated */
              $$.dataType = $$.pl.asOp.a->dataType;
            }
            | T_MINUS  T_LBC expr T_RBC { /* Arithmetic negate */
              $$.pl.asOp.a = mwl_shallow_copy_node(&($3));
              $$.pl.asOp.b = NULL;
              $$.pl.asOp.code = kOp_ArithNegate;
              $$.nodeType = mwl_kOperation;
              /*if(  ) ... TODO: check that type can be arith-negated */
              $$.dataType = $$.pl.asOp.a->dataType;
            }
            | T_TILDE  T_LBC expr T_RBC { /* Bitwise negate */
              $$.pl.asOp.a = mwl_shallow_copy_node(&($3));
              $$.pl.asOp.b = NULL;
              $$.pl.asOp.code = kOp_BtwsNegate;
              $$.nodeType = mwl_kOperation;
              /*if(  ) ... TODO: check that type can be bitwise-negated */
              $$.dataType = mwl_kInteger;
            }
            //| T_FOREIGN_CALL T_LBC expr T_RBC { $$ = $3; /*TODO: call*/ }
            ;

      value : T_CONSTVAL_TOKEN
            { $$.pl.asConstVal = $1;
              $$.nodeType = mwl_kConstValue;
              $$.dataType = $$.pl.asConstVal.valueType;
            }
            //| T_FOREIGN_VALUE   { TODO }
            | T_STRING_LITERAL  // TODO: shall we distinct string literal from other const val?
            { $$.pl.asConstVal = $1;
              $$.nodeType = mwl_kConstValue;
              $$.dataType = $$.pl.asConstVal.valueType;
            }
            | T_EXCLMM value
            { struct mwl_ASTNode * aCpy = mwl_shallow_copy_node(&($2));
              $$.pl.asOp.b = NULL;
              $$.pl.asOp.code = kOp_LogicNegate;
              $$.pl.asOp.a = aCpy;
              $$.nodeType = mwl_kOperation;
              $$.dataType = aCpy->dataType;
            }
            | T_TILDE value
            { if( $2.dataType != mwl_kInteger ) {
                  yyerror( &yylloc, ws, NULL, "bitwise negate applied to non-integer value" );
                  return MWL_INCOMPATIBLE_TYPE;
              }
              struct mwl_ASTNode * aCpy = mwl_shallow_copy_node(&($2));
              $$.pl.asOp.b = NULL;
              $$.pl.asOp.code = kOp_BtwsNegate;
              $$.pl.asOp.a = aCpy;
              $$.nodeType = mwl_kOperation;
              $$.dataType = mwl_kInteger;
            }
            //| foreignVal
            ;

    princOp : cmpOp             { $$ = $1; }
            | T_PLUS            { $$ = kOp_ArithAdd; }
            | T_MINUS           { $$ = kOp_ArithSub; }
            | T_ASTERISK        { $$ = kOp_ArithMult; }
            | T_SLASH           { $$ = kOp_ArithDiv; }
            | T_CIRCUMFLEX      { $$ = kOp_BtwsXor; }
            | T_AMP             { $$ = kOp_BtwsAnd; }
            | T_PIPE            { $$ = kOp_BtwsOr; }
            | T_ASTERISK2       { $$ = kOp_ArithPow; }
            | logicOp           { $$ = $1; }
            ;

      cmpOp : T_GT              { $$ = kOp_CmpGT; }
            | T_GTE             { $$ = kOp_CmpGE; }
            | T_LT              { $$ = kOp_CmpLT; }
            | T_LTE             { $$ = kOp_CmpLE; }
            | T_EQ              { $$ = kOp_CmpEq; }
            | T_NE              { $$ = kOp_CmpNE; }
            ;

    logicOp : T_AMP2            { $$ = kOp_LogicAnd; }
            | T_PIPE2           { $$ = kOp_LogicOr; }
            ;

   binOpSym : princOp                           { $$ = $1; }
            | T_DOT princOp                     { $$ = kOp_FS_NoLCompl | $2 ; }
            | princOp T_DOT                     { $$ =                   $1 | kOp_FS_NoRCompl; }
            | T_DOT princOp T_DOT               { $$ = kOp_FS_NoLCompl | $2 | kOp_FS_NoRCompl; }
            | T_REV_SLASH princOp               { $$ = kOp_FS_LvLCompl | $2 ; }
            | princOp T_REV_SLASH               { $$ =                   $1 | kOp_FS_LvRCompl; }
            | T_REV_SLASH princOp T_REV_SLASH   { $$ = kOp_FS_LvLCompl | $2 | kOp_FS_LvRCompl; }
            | T_DOT princOp T_REV_SLASH         { $$ = kOp_FS_NoLCompl | $2 | kOp_FS_LvRCompl; }
            | T_REV_SLASH princOp T_DOT         { $$ = kOp_FS_LvLCompl | $2 | kOp_FS_NoRCompl; }
            ;


// foreignVal : frgnValTok
//            | foreignVal T_DOT frgnValTok
//            ;
//
// frgnValTok : T_FOREIGN_VALUE
//            | T_FOREIGN_VALUE T_LSQBC selectExpr T_RSQBC
//            ;
//
// selectExpr : baseSelExpr
//            | selectExpr logicOp baseSelExpr
//            | T_LBC selectExpr T_RBC
//            | T_EXCLMM T_LBC selectExpr T_RBC
//            ;
//
//baseSelExpr : T_FOREIGN_VALUE cmpOp T_FOREIGN_VALUE
//            ;
%%

#include "lex.yy.h"

void
yyerror( YYLTYPE * yylloc
       , mwl_Workspace_t * ws
       , yyscan_t yyscanner
       , const char* s
       ) {
    if( ws->errMsgSize && '\0' != ws->errMsg[0] ) {
        fprintf( stderr
               , "Last translation error message replaces previous one: \"%s\".\n"
               , ws->errMsg );
    }
    if( ws->errMsgSize ) {
        snprintf( ws->errMsg, ws->errMsgSize
                , "error at %d,%d:%d,%d: %s"
                , yylloc->first_line, yylloc->first_column
                , yylloc->last_line, yylloc->last_column
                , s );
    } else {
        fprintf( stderr, "Translation error from (%d, %d) to (%d, %d): %s\n"
               , yylloc->first_line, yylloc->first_column
               , yylloc->last_line, yylloc->last_column
               , s );
    }
}

/* Main expression translate and "bake" function */
int
mwl_mk_ast( char * strexpr
          , const struct mwl_Definitions * definitions
          , FILE * dbgStream ) {
    mwl_Workspace_t ws;
    ws.dbgStream = dbgStream;

    char errMsgBuf[512];
    ws.errMsg = errMsgBuf;
    ws.errMsgSize = sizeof(errMsgBuf);
    ws.root = NULL;
    memset(ws.errPos, 0, sizeof(ws.errPos));

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
               , "Failed to translate expression because of %s\n"
               , ws.errMsg );
    }
    yy_delete_buffer(buffer, scannerPtr);
    yylex_destroy(scannerPtr);

    mwl_dump_AST(dbgStream, ws.root, 2);

    //if( rc ) return rc;
    //*rootPtr = ws.root;
    return rc;
}

