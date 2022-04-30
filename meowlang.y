%code requires {

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
}

%define api.pure full
%define parse.error verbose
%locations

%lex-param {Workspace_t * ws}
%lex-param {yyscan_t yyscanner}

%parse-param {Workspace_t * ws}
%parse-param {yyscan_t yyscanner}

%code provides {

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

}

%code {
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
}

%union {
    const char * strID;
    cmp_f_t cmpOpFunc;
    binop_f_t binOpFunc;
    ValueReference_t valRef;
    ComparisonExpression_t * cmpExpr;
    SelExpr_t * selectionBinOp;
}

%token<strID> L_STR
%token T_LBC T_RBC T_EXCLMM
%token T_GT T_GTE T_LT T_LTE T_EQ T_NE
%token T_AMP T_PIPE T_CAP
%token T_UNKNOWN_IDENTIFIER T_INVALID_VALUE
%token<valRef> T_VAL_REF_TOKEN

%type<cmpOpFunc> cmpOp;
%type<cmpExpr> ftCompExpr;
%type<selectionBinOp> selExpr;

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
            { ws->root = NULL; return NA64DP_DSUL_TRANSLATION_ERROR; }
            | selExpr
            { ws->root = $1; }
            ;

    selExpr : ftCompExpr
            {   $$ = new_expr(ws, (SelExpr_t *) $1, _uo_f_dereference, 0x0);
                if( NULL == $$ ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
            | T_LBC selExpr T_RBC
            {   $$ = $2; }
            | T_EXCLMM selExpr
            {   $$ = new_expr(ws, (SelExpr_t *) $2, _uo_f_negate, 0x0);
                if( NULL == $$ ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
            | selExpr T_AMP selExpr
            {   $$ = new_expr(ws, $1, _bo_f_and, $3);
                if( NULL == $$ ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
            | selExpr T_PIPE selExpr
            {   $$ = new_expr(ws, $1, _bo_f_or, $3);
                if( NULL == $$ ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
            | selExpr T_CAP selExpr
            { 
                $$ = new_expr(ws, $1, _bo_f_xor, $3);
                if( NULL == $$ ) {
                    yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                    return NA64DP_DSUL_BUFFER_EXHAUSTED;
                }
            }
            ;

 ftCompExpr : T_VAL_REF_TOKEN cmpOp T_VAL_REF_TOKEN
            { 
              if( ws->cursor + sizeof(ComparisonExpression_t) > ws->bufferEnd ) {
                yyerror( &yylloc, ws, NULL, "buffer exhausted" );
                return NA64DP_DSUL_BUFFER_EXHAUSTED;
              }
              $$ = (ComparisonExpression_t*) ws->cursor;
              ws->cursor += sizeof(ComparisonExpression_t);
              $$->left = $1;
              $$->cmp_f = $2;
              $$->right = $3;
            }
            ;

      cmpOp : T_GT      { $$ = _cmp_f_gt; }
            | T_GTE     { $$ = _cmp_f_ge; }
            | T_LT      { $$ = _cmp_f_lt; }
            | T_LTE     { $$ = _cmp_f_le; }
            | T_EQ      { $$ = _cmp_f_eq; }
            | T_NE      { $$ = _cmp_f_ne; }
            ;
%%

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

