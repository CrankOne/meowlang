#include "mwl-types.h"

#include <stdio.h>
#include <string.h>

const mwl_TypeCode_t mwl_kFIsForeign        = 0x1;
const mwl_TypeCode_t mwl_kFIsCollection     = 0x1 << 1;
const mwl_TypeCode_t mwl_kFIsMap            = 0x1 << 2;
const mwl_TypeCode_t mwl_kFIsScalar         = 0x1 << 3;
const mwl_TypeCode_t mwl_kFIsNumeric        = 0x1 << 4;
const mwl_TypeCode_t mwl_kFIsFloat          = 0x1 << 5;
const mwl_TypeCode_t mwl_kMScalarType       = ~( mwl_kFIsForeign
                                               | mwl_kFIsScalar
                                               | mwl_kFIsNumeric
                                               | mwl_kFIsFloat
                                               );

const mwl_TypeCode_t mwl_kTpLogic     = mwl_kFIsScalar;
const mwl_TypeCode_t mwl_kTpInteger   = mwl_kFIsScalar | mwl_kFIsNumeric;
const mwl_TypeCode_t mwl_kTpFloat     = mwl_kFIsScalar | mwl_kFIsNumeric | mwl_kFIsFloat;
const mwl_TypeCode_t mwl_kTpString    = mwl_kFIsCollection;
/* ... other types */

char *
mwl_to_str_type( char * bf
               , size_t bl
               , mwl_TypeCode_t code
               ) {
    if( mwl_kTpLogic == code )      return strncpy(bf, "bool", bl);
    if( mwl_kTpInteger == code )    return strncpy(bf, "integer", bl);
    if( mwl_kTpFloat == code )      return strncpy(bf, "float", bl);
    if( mwl_kTpString == code )     return strncpy(bf, "string", bl);

    if( mwl_kFIsForeign & code ) {
        snprintf(bf, bl, "foreign#%03d", code);
    } else if( mwl_kFIsCollection & code ) {
        if( mwl_kFIsMap & code ) {
            char keyBuf[32]
               , valBuf[32];
            snprintf( bf, bl, "{%s:%s}"
                    , mwl_to_str_type( valBuf, sizeof(valBuf), 0xff & code & ~(mwl_kFIsCollection | mwl_kFIsMap) )
                    , mwl_to_str_type( keyBuf, sizeof(keyBuf), code >> 8 )
                    );
        } else {
            char valBuf[32];
            snprintf( bf, bl
                    , "{%s}"
                    , mwl_to_str_type(valBuf, sizeof(valBuf), code & ~(mwl_kFIsCollection))
                    );
        }
    } else {
        snprintf(bf, bl, "?%#x?", code);
    }
    return bf;
}

mwl_TypeCode_t
mwl_combine_map_type( mwl_TypeCode_t keyType
                    , mwl_TypeCode_t valType
                    ) {
    return (keyType | (mwl_kFIsMap | mwl_kFIsCollection))
         | (valType << 8)
         ;
}

