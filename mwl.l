%option noyywrap reentrant 8bit yylineno
%option bison-locations
%option noinput nounput

D           [0-9]
L           [a-zA-Z_]
UL          [A-Z_]
M           [.!]

%{
#include <stdio.h>
#include "mwl.tab.h"

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

%}

%%

[ \t\n]         ; // ignore all whitespaces

-?[0-9]+        {   long val = strtol( yytext, NULL, 0 );
                    if( val > MWL_INTEGER_MAX || val < MWL_INTEGER_MIN ) {
                        mwl_error( yylloc, ws, yyscanner
                                 , "integer value is out of permitted range" );
                        return T_INVALID_VALUE;
                    }
                    yylval->constval.pl.asInteger = (mwl_Integer_t) val;
                    yylval->constval.dataType = mwl_kTpInteger;
                    return T_CONSTVAL_TOKEN;
                }
[-+]?{D}*\.?{D}+([eE][-+]?{D}+)? {
                  char * endptr = NULL;
                  double v = strtod(yytext, &endptr);
                  if( (!endptr) || '\0' != *endptr ) return T_INVALID_VALUE;
                  yylval->constval.pl.asFloat = (mwl_Float_t) v;
                  yylval->constval.dataType = mwl_kTpFloat;
                  return T_CONSTVAL_TOKEN;
                }
"true"          { yylval->constval.pl.asInteger = 1;
                  yylval->constval.dataType = mwl_kTpLogic;
                  return T_CONSTVAL_TOKEN;
                }
"false"         { yylval->constval.pl.asInteger = 0;
                  yylval->constval.dataType = mwl_kTpLogic;
                  return T_CONSTVAL_TOKEN;
                }

{M}?\*\*{M}?        {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_EXP_BINOP;
                        return T_INVALID_OPERATOR;}
{M}?[*/%]{M}?       {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_MULTIPLIC_BINOP;
                        return T_INVALID_OPERATOR;}
{M}?[+-]{M}?        {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_ADDITIVE_BINOP;
                        return T_INVALID_OPERATOR;}
{M}?(<<)|(>>){M}?   {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_BTWS_SHIFT;
                        return T_INVALID_OPERATOR;}
{M}?[><]=?{M}?      {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_LG_COMPARISON;
                        return T_INVALID_OPERATOR;}
{M}?!?={M}?         {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_NEQ_COMPARISON;
                        return T_INVALID_OPERATOR;}
{M}?&{M}?           {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_BTWS_BINOP_AND;
                        return T_INVALID_OPERATOR;}
{M}?^{M}?           {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_BTWS_BINOP_XOR;
                        return T_INVALID_OPERATOR;}
{M}?\|{M}?          {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_BTWS_BINOP_OR;
                        return T_INVALID_OPERATOR;}
{M}?&&{M}?          {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_LOG_BINOP_AND;
                        return T_INVALID_OPERATOR;}
{M}?\|\|{M}?        {if(!!(yylval->opCode = mwl_to_opcode(yytext, ws))) return T_LOG_BINOP_OR;
                        return T_INVALID_OPERATOR;}

"("                 { return T_LBC;}
")"                 { return T_RBC;}
"["                 { return T_LSQBC;}
"]"                 { return T_RSQBC;}
"~"                 { return T_TILDE;}
"!"                 { return T_EXCLMM;}
"."                 { return T_DOT;}
","                 { return T_COMMA;}

\"(\\.|[^"\\])*\"   { return T_STRING_LITERAL; /*TODO: copy*/ }
{L}({L}|{D})*       { yylval->strID = strdup( yytext );
                      return T_UNKNOWN_IDENTIFIER; }

%%
