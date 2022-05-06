#include "mwl-types.h"

#include <stdio.h>

const mwl_TypeCode_t mwl_kFIsNumeric  = 0x1;
const mwl_TypeCode_t mwl_kFIsVisited  = (0x1 << 31);

const mwl_TypeCode_t mwl_kTpLogic     = (1 << 1);
const mwl_TypeCode_t mwl_kTpInteger   = (2 << 1) | mwl_kFIsNumeric;
const mwl_TypeCode_t mwl_kTpFloat     = (3 << 1) | mwl_kFIsNumeric;
const mwl_TypeCode_t mwl_kTpString    = 0x10 | 0x1;
/* ... other types */

const char * mwl_to_str_type( mwl_TypeCode_t code ) {
    if( mwl_kTpLogic == code )    return "bool";
    if( mwl_kTpInteger == code )  return "integer";
    if( mwl_kTpFloat == code )    return "float";
    if( mwl_kTpString == code )   return "string";
    static char bf[32];
    snprintf(bf, sizeof(bf), "?%#x?", code);
    return bf;
}

