#ifndef H_MWL_TYPES_H
#define H_MWL_TYPES_H

#include "mwl-config.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Integer type used to identify operations */
typedef uint32_t mwl_OpCode_t;
/** Integer type used to identify node types */
typedef uint32_t mwl_TypeCode_t;

/** C-type of "integer" */
typedef int32_t mwl_Integer_t;
#define MWL_INTEGER_MAX INT32_MAX
#define MWL_INTEGER_MIN INT32_MIN
/** C-type of "float" */
typedef float mwl_Float_t;

/* Type code features: */
extern const mwl_TypeCode_t mwl_kFIsNumeric;
extern const mwl_TypeCode_t mwl_kFIsVisited;

extern const mwl_TypeCode_t mwl_kTpLogic;
extern const mwl_TypeCode_t mwl_kTpInteger;
extern const mwl_TypeCode_t mwl_kTpFloat;
extern const mwl_TypeCode_t mwl_kTpString;
/* ... other types? map, set, etc */

const char * mwl_to_str_type( mwl_TypeCode_t );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  /* H_MWL_TYPES_H */
