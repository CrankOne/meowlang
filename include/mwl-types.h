#ifndef H_MWL_TYPES_H
#define H_MWL_TYPES_H

#include "mwl-config.h"

#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Types are classified by following features:
 *  - we have the scalar (bool, int, float), complex types (string, homogeneous
 *    set, homogeneous map) and foreign (namespace) types
 *    + scalar types are: bool (logic), int (ordinary integer), and floating
 *      point
 *    + complex types, except for string, are usually defined based on scalar
 *      type. Set contains a single type that refers to the collected element
 *      type, that can be a scalar or foreign type. Map type code contains
 *      a type of scalar used as a key and type code of collected elements
 *      (scalar or foreign)
 *    + foreign type code is the ID used for external resolver to address and
 *      arbitrary type. Foreign types are not the subject of any arithmetics.
 * Also note, that code type is practically shorter by 1 bit than the declared
 * type since we do use it in `mwl_ASTNode` as `isVisited` flag.
 *
 * This way full bit structure used to identify and encode a type information.
 * When foreign bit is set in section, the rest 7 bits are interpreted as
 * foreign ID. If it is not set, their meaning is following:
 *                                     logic   int     float    string     set     map foreign
 *  [  =1] "is foreign" flag               0     0         0         0       0       0       1
 *  [  =2] "is scalar" flag                1     1         1         0       x       x       x
 *  [  =3] "is numeric" flag               0     1         1         0       x       x       x
 *  [  =4] "is float" flag                 0     0         1         0       x       x       x
 *  [  =5] "is collection" flag            0     0         0         1       1       1       x
 *  [  =6] "is map"                        0     0         0         0       0       1       x
 *  [ 7-8] no meaning, reserved            x     x         x         x       x       x       x
 * 
 * For all types except for maps the type ID is of one byte (8 bits).
 * For maps the second byte is reserved for "mapped value type".
 */

/** Integer type used to identify node types */
typedef uint32_t mwl_TypeCode_t;

/** C-type of "integer" */
typedef int32_t mwl_Integer_t;
#define MWL_INTEGER_MAX INT32_MAX
#define MWL_INTEGER_MIN INT32_MIN
/** C-type of "float" */
typedef float mwl_Float_t;

/* Type code features: */
extern const mwl_TypeCode_t mwl_kFIsForeign;
extern const mwl_TypeCode_t mwl_kFIsScalar;
extern const mwl_TypeCode_t mwl_kFIsNumeric;
extern const mwl_TypeCode_t mwl_kFIsFloat;
extern const mwl_TypeCode_t mwl_kFIsCollection;
extern const mwl_TypeCode_t mwl_kFIsMap;
/* Bit mask to apply to type to get the scalar type ID */
extern const mwl_TypeCode_t mwl_kMScalarType;

/* Scalar type codes */
extern const mwl_TypeCode_t mwl_kTpLogic;
extern const mwl_TypeCode_t mwl_kTpInteger;
extern const mwl_TypeCode_t mwl_kTpFloat;
extern const mwl_TypeCode_t mwl_kTpString;
/* ... other types? map, set, etc */

char * mwl_to_str_type( char *, size_t, mwl_TypeCode_t );

/** Combines key and value type into a single type descriptor */
mwl_TypeCode_t
mwl_combine_map_type( mwl_TypeCode_t keyType
                    , mwl_TypeCode_t valType
                    );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  /* H_MWL_TYPES_H */
