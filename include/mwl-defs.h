#ifndef H_MWL_DEFINITIONS_H
#define H_MWL_DEFINITIONS_H

#include "mwl-types.h"
#include "mwl-func.h"

#ifdef __cplusplus
#include <unordered_map>

extern "C" {
#endif

struct mwl_ArgsList;

/** Payload of AST node representing constant value */
struct mwl_ConstVal {
    mwl_TypeCode_t dataType;
    union {
        mwl_Integer_t asInteger;
        mwl_Float_t asFloat;
        char * asString;
    } pl;
};

struct mwl_Parameter {
    // ...
};

struct mwl_Variable {
    // ...
};

struct mwl_Definition {
    enum {
        mwl_kDefNamespace,
        mwl_kDefConstval,
        mwl_kDefForeignCall,
        //kForeignVal,
        //kVariable,
    } type;
    mwl_TypeCode_t dataType;
    union {
        struct mwl_Definitions * asNamespacePtr;  // used on kSection
        struct mwl_ConstVal asConstVal;  // used for kConstval
        struct mwl_FuncDef  asFuncdef;   // used for kForeignCall
        // struct mwl_ForeignVal;   // used for kForeignVal
        // struct mwl_VariableDef;
    } pl;
};

struct mwl_Definitions;

#ifdef __cplusplus
struct mwl_Definitions : public std::unordered_map< std::string
                                                  , struct mwl_Definition > {
    typedef std::unordered_map<std::string, struct mwl_Definition> Parent;

    Parent::iterator define_int_constval(const std::string &, mwl_Integer_t value=0);
    Parent::iterator define_float_constval(const std::string &, mwl_Float_t value=0.0);
 
    /**\brief Defines a namespace.
     *
     * Note, that created namespace will have its `dataType` field undefined.
     * */
    Parent::iterator define_namespace(const std::string &);

    /// Recursively deletes all subsections
    ~mwl_Definitions();
};

namespace mwl {
namespace error {
struct DuplicatingDefinitionError : public std::runtime_error {
    const mwl_Definitions::const_iterator another;
    DuplicatingDefinitionError( mwl_Definitions::const_iterator another_ )
        : std::runtime_error("Duplicating definition")
        , another(another_) {}
};
}  // namespace ::mwl::error
}  // namespace mwl
#endif

const struct mwl_Definition *
mwl_find_definition( const struct mwl_Definitions *
                   , const char * identifier );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
