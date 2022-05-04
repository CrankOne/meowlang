%option noyywrap reentrant 8bit yylineno
%option bison-locations
%option noinput nounput

D           [0-9]
L           [a-zA-Z_\\]
UL          [A-Z_]
LOG_CMP     [><]=?

%{
#include <stdio.h>
#include "meowlang.tab.h"

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
"**"            {return T_ASTERISK2;}
"&&"            {return T_AMP2;}
"||"            {return T_PIPE2;}
"+"             {return T_PLUS;}
"-"             {return T_MINUS;}
"*"             {return T_ASTERISK;}
"/"             {return T_SLASH;}
"\\"            { return T_REV_SLASH;}
">"             {return T_GT;}
">="            {return T_GTE;}
"<="            {return T_LTE;}
"=="            {return T_EQ;}
"!="            {return T_NE;}
"<"             {return T_LT;}
"("             { return T_LBC;}
")"             { return T_RBC;}
"["             { return T_LSQBC;}
"]"             { return T_RSQBC;}
"{"             { return T_LCRLBC;}
"}"             { return T_RCRLBC;}
"~"             { return T_TILDE;}
"&"             {return T_AMP;}
"|"             {return T_PIPE;}
"^"             {return T_CIRCUMFLEX;}
"!"             {return T_EXCLMM;}
"."             { return T_DOT;}
\"(\\.|[^"\\])*\"  {return T_STRING_LITERAL; /*TODO: copy*/ }
-?[0-9]+        {   long val = strtol( yytext, NULL, 0 );
                    if( val > MWL_INTEGER_MAX || val < MWL_INTEGER_MIN ) {
                        meowlang_error( yylloc, ws, yyscanner
                                      , "integer value is out of permitted range" );
                        return T_INVALID_VALUE;
                    }
                    yylval->constval.pl.asInteger = (mwl_Integer_t) val;
                    yylval->constval.valueType = mwl_kInteger;
                    return T_CONSTVAL_TOKEN;
                }
{L}({L}|{D})*   {   /* Lookup for name in symbols table:
                     * - return "T_CONSTVAL_TOKEN" for constants
                     * - return "T_FOREIGN_VALUE" for external parameters
                     * - return "T_FOREIGN_CALL" for functions
                     */
                    return T_UNKNOWN_IDENTIFIER;
                }

%%
