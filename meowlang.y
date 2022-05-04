%code requires {

#include "meowlang_ast.h"
#include <stdio.h>

/* Compile function error codes */
#define MEOW_STREXPR_BAD         -1  /* bad argument expression NULL or not a string */
#define MEOW_BUFFER_EXHAUSTED    -2  /* bad or insufficient buffer */
#define MEOW_MEMORY_ERROR        -3  /* heap allocation failure */
#define MEOW_TRANSLATION_ERROR   -4  /* syntax/parser/lexer error */
#define MEOW_BAD_GETTERS_TABLE   -5  /* bad getter table entry */

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

int mwl_mk_AST( char * strexpr
              , const struct mwl_Definitions * definitions
              , FILE * dbgStream );

}

%union {
    const char * strID;
    mwl_OpCode_t opCode;
    struct mwl_ConstVal constval;
    struct mwl_ASTNode astNode;
    /* ... */
}

%token<strID> L_STR
%token T_LBC T_RBC T_LSQBC T_RSQBC T_LCRLBC T_RCRLBC
%token T_DOT T_REV_SLASH T_TILDE T_EXCLMM

%token<opCode> T_EXP_BINOP T_MULTIPLIC_BINOP T_ADDITIVE_BINOP T_BTWS_SHIFT
%token<opCode> T_LG_COMPARISON T_NEQ_COMPARISON
%token<opCode> T_BTWS_BINOP_AND T_BTWS_BINOP_OR T_BTWS_BINOP_XOR
%token<opCode> T_LOG_BINOP_AND T_LOG_BINOP_OR

%type<astNode> expr logicOr logicAnd bitwiseOr bitwiseXor bitwiseAnd logicNeq
%type<astNode> gtCmp btwsShift addBinop multBinop expBinop value

%token T_UNKNOWN_IDENTIFIER T_INVALID_VALUE T_INVALID_OPERATOR T_FOREIGN_VALUE T_FOREIGN_CALL
%token<constval> T_CONSTVAL_TOKEN T_STRING_LITERAL

//%left T_GT T_GTE T_LT T_LTE T_EQ T_NE
//%left T_LBC T_RBC

%start toplev

%%

     toplev : error
            { ws->root = NULL; return MEOW_TRANSLATION_ERROR; }
            | expr { ws->root = mwl_shallow_copy_node(&($1)); }
            ;

       expr : logicOr
            ;

    logicOr : logicOr T_LOG_BINOP_OR logicAnd
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | logicAnd
            ;

   logicAnd : logicAnd T_LOG_BINOP_AND bitwiseOr
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | bitwiseOr
            ;

  bitwiseOr : bitwiseOr T_BTWS_BINOP_OR bitwiseXor
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | bitwiseXor
            ;

 bitwiseXor : bitwiseXor T_BTWS_BINOP_XOR bitwiseAnd
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | bitwiseAnd
            ;

 bitwiseAnd : bitwiseAnd T_BTWS_BINOP_AND logicNeq
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | logicNeq
            ;

   logicNeq : logicNeq T_NEQ_COMPARISON gtCmp
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | gtCmp
            ;

      gtCmp : gtCmp T_LG_COMPARISON btwsShift
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | btwsShift
            ;

  btwsShift : btwsShift T_BTWS_SHIFT addBinop
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | addBinop
            ;

   addBinop : addBinop T_ADDITIVE_BINOP multBinop
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | multBinop
            ;

  multBinop : multBinop T_MULTIPLIC_BINOP expBinop
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | expBinop
            ;

   expBinop : expBinop T_EXP_BINOP value
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | T_LBC expr T_RBC {$$ = $2;}
            | value
            ;

      value : T_CONSTVAL_TOKEN {
                $$.pl.asConstVal = $1;
                $$.dataType = $1.dataType;
                $$.nodeType = mwl_kConstValue;
            }
            //| foreignCall T_LBC expr T_RBC
            //| foreignValue
            ;

//    princOp : cmpOp             { $$ = $1; }
//            | T_PLUS            { $$ = kOp_ArithAdd; }
//            | T_MINUS           { $$ = kOp_ArithSub; }
//            | T_ASTERISK        { $$ = kOp_ArithMult; }
//            | T_SLASH           { $$ = kOp_ArithDiv; }
//            | T_CIRCUMFLEX      { $$ = kOp_BtwsXor; }
//            | T_AMP             { $$ = kOp_BtwsAnd; }
//            | T_PIPE            { $$ = kOp_BtwsOr; }
//            | T_ASTERISK2       { $$ = kOp_ArithPow; }
//            | logicOp           { $$ = $1; }
//            ;
//
//      cmpOp : T_GT              { $$ = kOp_CmpGT; }
//            | T_GTE             { $$ = kOp_CmpGE; }
//            | T_LT              { $$ = kOp_CmpLT; }
//            | T_LTE             { $$ = kOp_CmpLE; }
//            | T_EQ              { $$ = kOp_CmpEq; }
//            | T_NE              { $$ = kOp_CmpNE; }
//            ;
//
//    logicOp : T_AMP2            { $$ = kOp_LogicAnd; }
//            | T_PIPE2           { $$ = kOp_LogicOr; }
//            ;
//
//   binOpSym : princOp                           { $$ = $1; }
//            | T_DOT princOp                     { $$ = kOp_FS_NoLCompl | $2 ; }
//            | princOp T_DOT                     { $$ =                   $1 | kOp_FS_NoRCompl; }
//            | T_DOT princOp T_DOT               { $$ = kOp_FS_NoLCompl | $2 | kOp_FS_NoRCompl; }
//            | T_REV_SLASH princOp               { $$ = kOp_FS_LvLCompl | $2 ; }
//            | princOp T_REV_SLASH               { $$ =                   $1 | kOp_FS_LvRCompl; }
//            | T_REV_SLASH princOp T_REV_SLASH   { $$ = kOp_FS_LvLCompl | $2 | kOp_FS_LvRCompl; }
//            | T_DOT princOp T_REV_SLASH         { $$ = kOp_FS_NoLCompl | $2 | kOp_FS_LvRCompl; }
//            | T_REV_SLASH princOp T_DOT         { $$ = kOp_FS_LvLCompl | $2 | kOp_FS_NoRCompl; }
//            ;


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
mwl_mk_AST( char * strexpr
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
               , "Parser failure: %s\n"
               , ws.errMsg );
    }
    yy_delete_buffer(buffer, scannerPtr);
    yylex_destroy(scannerPtr);

    mwl_dump_AST(dbgStream, ws.root, 2);

    //if( rc ) return rc;
    //*rootPtr = ws.root;
    return rc;
}

