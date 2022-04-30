%option noyywrap reentrant 8bit yylineno
%option bison-locations
%option noinput nounput

D           [0-9]
L           [a-zA-Z_\\]
UL          [A-Z_]
DTNAME      {UL}


%{
#include <stdio.h>
#include "ds-dsl.tab.h"

#define YY_USER_ACTION                              \
    yylloc->first_line = yylloc->last_line;         \
    yylloc->first_column = yylloc->last_column;     \
    for(int i = 0; yytext[i] != '\0'; i++) {        \
        if(yytext[i] == '\n') {                     \
            yylloc->last_line++;                    \
            yylloc->last_column = 0;                \
        } else {                                    \
            yylloc->last_column++;                  \
        }                                           \
    }

static struct Symbol *
push_symbol( struct Symbol * syms, CodeVal_t getterOffset, const char * name ) {
    if( NA64DP_DSUL_MAX_GETTERS != getterOffset ) { ++getterOffset; }
    /* Iterate over defined symbols till either the end of symbol sequence
     * is met (marked by getterOffset =0), or the same symbol is met
     * (getterOffset) matches. The getterOffset == NA64DP_DSUL_MAX_GETTERS
     * corresponds to literals */
    while( syms->defIdx ) {
        if( getterOffset == NA64DP_DSUL_MAX_GETTERS ) {
            ++syms;
            continue;  /* skip (do not consider) literals */
        }
        if( name && syms->name ) {
            if( !strcmp(name, syms->name) ) break; /* static symbol match -- take this one*/
        } else if( syms->defIdx == getterOffset ) {
            break;
        }
        ++syms;
    }
    /* mark next as vacant as well if new symbol added */
    if(getterOffset == NA64DP_DSUL_MAX_GETTERS || syms->defIdx != getterOffset) {
        syms->defIdx = getterOffset;
        (syms+1)->defIdx = 0x0;
        /* since cleaning routine for externally-resolved has to free the name
         * we have to initialize it */
        syms->name = NULL;
    }
    return syms;
}

%}

%%

[ \t\n]         ; // ignore all whitespaces
">"             {return T_GT;}
"<"             {return T_LT;}
">="            {return T_GTE;}
"<="            {return T_LTE;}
"=="            {return T_EQ;}
"("             {return T_LBC;}
")"             {return T_RBC;}
"!="            {return T_NE;}
"&&"            {return T_AMP;}
"||"            {return T_PIPE;}
"^^"            {return T_CAP;}
"!"             {return T_EXCLMM;}
[0-9]+          {   long val = strtol( yytext, NULL, 0 );
                    if( val > NA64DP_DSUL_CODE_VAL_MAX || val < 0 ) {
                        na64dpsu_error( yylloc, ws, yyscanner
                                      , "integer value is out of range" );
                        return T_INVALID_VALUE;
                    }
                    /* Define new symbol for const literal */
                    struct Symbol * ncsPtr = push_symbol( ws->symbols
                                                        , NA64DP_DSUL_MAX_GETTERS
                                                        , NULL );
                    ncsPtr->value = val;
                    bzero( &(yylval->valRef), sizeof(union ValueReference) );
                    yylval->valRef.symbOffset = (CodeVal_t) (ncsPtr - ws->symbols);
                    if( ws->dbgStream ) {
                        fprintf( ws->dbgStream
                              , "New RefVal for literal %d: -> %d on sym #%d\n"
                              , ncsPtr->value
                              , ncsPtr->defIdx, yylval->valRef.symbOffset );
                    }
                    return T_VAL_REF_TOKEN;
                }
{L}({L}|{D})*   {   /* Lookup for name in "getters table" */
                    for( const struct na64dpsu_SymDefinition * gte = ws->gettersTable
                       ; gte->name
                       ; ++gte ) {
                        if( NA64DP_DSUL_EXTERN_RESOLVER_CHAR == gte->name[0] ) {
                            /* value resolver */
                            CodeVal_t cVal;
                            int checkResult = gte->callback.resolve_name( yytext
                                                                        , ws->externResolverPtr
                                                                        , &cVal );
                            if( !checkResult ) continue;
                            struct Symbol * ncsPtr = push_symbol( ws->symbols
                                                                , gte - ws->gettersTable
                                                                , yytext );
                            ncsPtr->name = strdup(yytext);
                            bzero( &(yylval->valRef), sizeof(union ValueReference) );
                            yylval->valRef.symbOffset = (CodeVal_t) (ncsPtr - ws->symbols);
                            if( ws->dbgStream ) {
                                fprintf( ws->dbgStream
                                      , "New RefVal (extern) \"%s\": -> %d on sym #%d\n"
                                      , yytext
                                      , ncsPtr->defIdx, yylval->valRef.symbOffset );
                            }
                            return T_VAL_REF_TOKEN;
                        } else if( !strcmp(gte->name + 1, yytext) ) { /* Got name match */
                            /* Add new symbol / retrieve existing symbol */
                            struct Symbol * ncsPtr = push_symbol( ws->symbols
                                                                , gte - ws->gettersTable
                                                                , NULL );
                            /* Set symbol reference in value reference */
                            bzero( &(yylval->valRef), sizeof(union ValueReference) );
                            yylval->valRef.symbOffset = (CodeVal_t) (ncsPtr - ws->symbols);
                            if( ws->dbgStream ) {
                                fprintf( ws->dbgStream
                                      , "New RefVal \"%s\": -> %d on sym #%d\n"
                                      , gte->name
                                      , ncsPtr->defIdx, yylval->valRef.symbOffset );
                            }
                            return T_VAL_REF_TOKEN;
                        }
                    }
                    char errbf[128];
                    snprintf(errbf, 128, "unknown identifier \"%s\"", yytext );
                    na64dpsu_error( yylloc, ws, yyscanner, errbf );
                    return T_UNKNOWN_IDENTIFIER;
                }

%%
