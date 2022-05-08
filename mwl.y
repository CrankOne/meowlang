%code requires {

#include "mwl-ast.h"
#include "mwl-ws.h"
#include "mwl-ops.h"
#include <assert.h>

/* Compile function error codes */
#define MWL_UNRESOLVED_IDENTIFIER_ERROR     -1  /* failed to resolve identifier */
#define MWL_TRANSLATION_ERROR               -2  /* syntax/parser/lexer error */
#define MWL_RESULT_TYPE_ERROR               -3  /* failed to infer return type */

typedef void *yyscan_t;  /* circumvent circular dep: YACC/BISON does not know this type */

}

%define api.pure full
%define parse.error verbose
%locations

%lex-param {struct mwl_Workspace * ws}
%lex-param {yyscan_t yyscanner}

%parse-param {struct mwl_Workspace * ws}
%parse-param {yyscan_t yyscanner}

%code provides {

#define YY_DECL                             \
    int mwl_lex( YYSTYPE* yylval_param      \
               , YYLTYPE* yylloc_param      \
               , struct mwl_Workspace * ws       \
               , yyscan_t yyscanner )
YY_DECL;

void mwl_error( struct YYLTYPE *
              , struct mwl_Workspace *
              , yyscan_t yyscanner
              , const char* );

#ifdef __cplusplus
extern "C"
#endif
struct mwl_ASTNode *
mwl_mk_AST( char * strexpr
          , const struct mwl_Definitions * definitions
          , FILE * dbgStream );

}

%union {
    char * strID;
    mwl_OpCode_t opCode;
    struct mwl_ConstVal      constval;
    struct mwl_ASTNode       astNode;
    struct mwl_ArgsList      argsList;
    struct mwl_MapPairList   mapPairsList;
    /* ... */
}

%destructor { free($$); } <strID>

%token<strID> L_STR
%token T_LBC T_RBC T_LSQBC T_RSQBC T_LCRLBC T_RCRLBC
%token T_DOT T_COMMA T_REV_SLASH T_TILDE T_EXCLMM T_COLON

%token<opCode> T_EXP_BINOP T_MULTIPLIC_BINOP T_ADDITIVE_BINOP T_BTWS_SHIFT
%token<opCode> T_LG_COMPARISON T_NEQ_COMPARISON
%token<opCode> T_BTWS_BINOP_AND T_BTWS_BINOP_OR T_BTWS_BINOP_XOR
%token<opCode> T_LOG_BINOP_AND T_LOG_BINOP_OR

%type<astNode> expr logicOr logicAnd bitwiseOr bitwiseXor bitwiseAnd logicNeq
%type<astNode> gtCmp btwsShift addBinop multBinop expBinop negateUnOp unaryOp value
%type<astNode> foreignVal

%type<argsList> arguments
%type<mapPairsList> mapValues

%token T_INVALID_VALUE T_INVALID_OPERATOR T_FOREIGN_VALUE T_FOREIGN_CALL
%token<constval> T_CONSTVAL_TOKEN T_STRING_LITERAL
%token<strID> T_UNKNOWN_IDENTIFIER

//%left T_GT T_GTE T_LT T_LTE T_EQ T_NE
//%left T_LBC T_RBC

%start toplev

%%

     toplev : error
            { ws->root = NULL; return MWL_TRANSLATION_ERROR; }
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

   expBinop : expBinop T_EXP_BINOP negateUnOp
            { int rc = mwl_init_op_node(&$$, &$1, $2, &$3, ws); if(rc) return rc; }
            | negateUnOp
            ;

 negateUnOp : T_EXCLMM unaryOp
            { int rc = mwl_init_op_node(&$$, &$2, kOp_LogicNegate, NULL, ws); if(rc) return rc; }
            | T_TILDE unaryOp
            { int rc = mwl_init_op_node(&$$, &$2, kOp_BtwsNegate, NULL, ws); if(rc) return rc; }
            | unaryOp
            ;

    unaryOp : T_ADDITIVE_BINOP value
            { int rc = mwl_init_op_node(&$$, &$2, $1 | kOp_FUnary, NULL, ws); if(rc) return rc; }
            | value
            ;

      value : T_CONSTVAL_TOKEN {
                $$.pl.asConstVal = $1;
                $$.dataType = $1.dataType;
                $$.nodeType = mwl_kConstValue;
                $$.isVisited = 0;
                $$.userdata = NULL;
                $$.selector = NULL;
            }
            | T_LCRLBC arguments T_RCRLBC {  // a set
                $$.nodeType = mwl_kSet;

                $$.isVisited = 0;
                $$.userdata = NULL;
                $$.selector = NULL;

                $$.pl.asSet.dataType = mwl_induce_set_type(&($2), ws);
                if( ! $$.pl.asSet.dataType ) {
                    yyerror( &yylloc, ws, NULL, "Failed to infer set type." );
                    return MWL_RESULT_TYPE_ERROR;
                }
                $$.pl.asSet.dataType |= mwl_kFIsCollection;
                $$.dataType = $$.pl.asSet.dataType;
                $$.pl.asSet.values = $2;
            }
            | T_LCRLBC mapValues T_RCRLBC {
                $$.nodeType = mwl_kMap;

                $$.isVisited = 0;
                $$.userdata = NULL;
                $$.selector = NULL;

                $$.pl.asMap.dataType = mwl_induce_map_type(&($2), ws);
                if( ! $$.pl.asMap.dataType ) {
                    yyerror( &yylloc, ws, NULL, "Failed to infer associative array type." );
                    return MWL_RESULT_TYPE_ERROR;
                }
                $$.pl.asMap.dataType |= mwl_kFIsCollection | mwl_kFIsMap;
                $$.dataType = $$.pl.asMap.dataType;
                $$.pl.asMap.values = $2;
            }
            | T_LBC expr T_RBC {$$ = $2;}
            //| foreignCall T_LBC expr T_RBC
            | foreignVal
            | value T_LSQBC expr T_RSQBC {
                int rc;
                char bf[256], bf1[64];
                if( !(mwl_kFIsCollection & $1.dataType) ) {
                    snprintf(bf, sizeof(bf), "selector expression applied to"
                        " `%s' data type which is not a collection"
                        , mwl_to_str_type(bf1, sizeof(bf1), $1.dataType) );
                    yyerror(&yylloc, ws, NULL, bf );
                    return MWL_RESULT_TYPE_ERROR;
                }
                if( 0x0 == $3.dataType ) {
                    rc = mwl_resolve_selector_context( &($3), &($1) );
                    if( rc ) {
                        snprintf(bf, sizeof(bf), "failed to resolve type of"
                            " selector within the `%s' type's context"
                            , mwl_to_str_type(bf1, sizeof(bf1), $1.dataType) );
                        yyerror(&yylloc, ws, NULL, bf );
                        return MWL_RESULT_TYPE_ERROR;
                    }
                    assert($3.dataType);
                }
                $1.selector = mwl_shallow_copy_node(&($3));
            }
            ;

 foreignVal : T_UNKNOWN_IDENTIFIER {
                int rc = mwl_resolve_identifier_to_ast(&($$), ws->defs, $1
                    , yylloc.first_line, yylloc.first_column
                    , yylloc.last_line, yylloc.last_column );
                if( 0 == rc ) free($1);
                else if( rc != 1 ) return MWL_UNRESOLVED_IDENTIFIER_ERROR;
            }
            | foreignVal T_DOT T_UNKNOWN_IDENTIFIER {
                if( $1.nodeType != mwl_kNamespace ) {
                    char errbf[128];
                    snprintf( errbf, sizeof(errbf)
                            , "can not resolve \"%s\" (not within a namespace)"
                            , $3);
                    yyerror( &yylloc, ws, NULL, errbf );
                    free($3);
                    return MWL_UNRESOLVED_IDENTIFIER_ERROR;
                }
                int rc = mwl_resolve_identifier_to_ast(&($$), $1.pl.asNamespace, $3
                    , yylloc.first_line, yylloc.first_column
                    , yylloc.last_line, yylloc.last_column );
                if( 0 == rc ) free($3);  /* identifier was resolved */
                else if( rc != 1 ) return MWL_UNRESOLVED_IDENTIFIER_ERROR;
            }
            | foreignVal T_LBC arguments T_RBC {
                if($1.nodeType != mwl_kFunction) {
                    yyerror( &yylloc, ws, NULL, "Call to not a function." );
                    return MWL_RESULT_TYPE_ERROR;
                }
                $$ = $1;
                $$.dataType = mwl_resolve_func_return_type(
                        $1.pl.asFunction.funcdef, &($3) );
                if( ! $$.dataType ) {
                    yyerror(&yylloc, ws, NULL, "Failed to resolve function"
                        " result type.");
                    return MWL_RESULT_TYPE_ERROR;
                }
                $$.pl.asFunction.argsList = $3;
                $$.isVisited = 0;
                $$.userdata = 0;
            }
            ;

  arguments : expr {
                $$.self = mwl_shallow_copy_node(&($1));
                $$.next = NULL;
            }
            | arguments T_COMMA expr {
                $$.self = mwl_shallow_copy_node(&($3));
                $$.next = mwl_shallow_copy_args(&($1));
            }
            ;

  mapValues : expr T_COLON expr {
                $$.key = mwl_shallow_copy_node(&($1));
                $$.value = mwl_shallow_copy_node(&($3));
                $$.next = NULL;
            }
            | mapValues T_COMMA expr T_COLON expr {
                $$.key = mwl_shallow_copy_node(&($3));
                $$.value = mwl_shallow_copy_node(&($5));
                $$.next = mwl_shallow_copy_map_entries(&($1));
            }
            ;

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
       , struct mwl_Workspace * ws
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
struct mwl_ASTNode *
mwl_mk_AST( char * strexpr
          , const struct mwl_Definitions * definitions
          , FILE * dbgStream ) {
    struct mwl_Workspace ws;
    ws.dbgStream = dbgStream;
    ws.defs = definitions;

    char errMsgBuf[512] = "";
    ws.errMsg = errMsgBuf;
    ws.errMsgSize = sizeof(errMsgBuf);
    ws.root = NULL;
    memset(ws.errPos, 0, sizeof(ws.errPos));

    /* Parse expression */
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

    return ws.root;
}

