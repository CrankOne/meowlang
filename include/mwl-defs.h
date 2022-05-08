#ifndef H_MWL_DEFINITIONS_H
#define H_MWL_DEFINITIONS_H

#include "mwl-types.h"
#include "mwl-func.h"

#ifdef __cplusplus

#include <cassert>
#include <unordered_map>
#include <map>
#include <cstring>
#include <typeinfo>
#include <typeindex>

extern "C" {
#endif

struct mwl_ArgsList;
struct mwl_Definitions;

/** Payload of AST node representing constant value */
struct mwl_ConstVal {
    mwl_TypeCode_t dataType;
    union {
        mwl_Integer_t asInteger;
        mwl_Float_t asFloat;
        char * asString;
    } pl;
};

/**\brief Foreign type definition
 * */
struct mwl_ForeignTypeTable {
    /** A C++ `type_info` ptr, for assertions */
    const void * cppTypeID;

    /** If type is a set or map of objects, this type is non-zero and defines
     * collected key types */
    mwl_TypeCode_t keyType
                 , valueType
                 ;

    /** If type provides a selector, this attribute is set to definitions of
     * special context available only within a selector expressions. */
    struct mwl_Definitions * selectorDefinitions;
    // ...
};

struct mwl_Parameter {
    // ...
};

struct mwl_Variable {
    /** Data type of the variable */
    mwl_TypeCode_t dataType;
    /** Selector expression */
    struct mwl_ASTNode * selector;
    // ...
};

struct mwl_Definition {
    enum {
        mwl_kDefNamespace,
        mwl_kDefConstval,
        mwl_kDefForeignCall,
        mwl_kDefForeignType,
        //kForeignVal,
        //kVariable,
    } type;
    mwl_TypeCode_t dataType;
    union {
        struct mwl_Definitions *      asNamespacePtr;  // used on kSection
        struct mwl_ConstVal           asConstVal;  // used for kConstval
        struct mwl_FuncDef            asFuncdef;   // used for kForeignCall
        struct mwl_ForeignTypeTable * asForeignTypeTable;
        // struct mwl_ForeignVal;   // used for kForeignVal
        // struct mwl_VariableDef;
    } pl;
};

struct mwl_Definitions;

#ifdef __cplusplus
}  // extern "C"

//
// Errors

namespace mwl {
namespace error {
struct DuplicatingDefinitionError : public std::runtime_error {
    const std::unordered_map< std::string
                            , struct mwl_Definition >::const_iterator another;
    DuplicatingDefinitionError( std::unordered_map< std::string
                                                  , struct mwl_Definition >::const_iterator another_ )
        : std::runtime_error("Duplicating definition")
        , another(another_) {}
};
}  // namespace ::mwl::error
}  // namespace mwl

template<typename T, typename=void> struct ConstValTraits;

namespace mwl {
/// Auxiliary class instance keeping index of foreign types
///
/// This dictionary contains foreign types only by their order number
/// (that is the part of `mwl_TypeCode_t` withut bit responsible for
/// "is foreign" feature).
class ForeignTypes : protected std::map<uint16_t, mwl_ForeignTypeTable> {
public:
    /// Helper alias to parent type
    typedef std::map<uint16_t, mwl_ForeignTypeTable> Parent;
private:
    /// To prevent double regitration and perform C++ lookup
    std::unordered_map<std::type_index, Parent::iterator> _registered;
protected:
    /// Defines new foreign type, assures some basic validity and return ptr
    /// to table.
    template<typename T>
    std::pair<Parent::iterator, bool>
    define() {
        auto it = _registered.find(typeid(T));
        if( it != _registered.end() ) {
            return std::pair<Parent::iterator, bool>(it->second, false);
        }
        // otherwise, insert new
        if( size() == FOREIGN_TYPES_MAX_ID ) {
            throw std::runtime_error("Max number of foreign types exceed.");
            //^^^ TODO: throw mwl::error::MaxForeignTypeIDExceedError( nm, FOREIGN_TYPES_MAX_ID );
        }
        // Register new foreign type ID
        auto ir = Parent::emplace( Parent::size()
                                 , mwl_ForeignTypeTable()
                                 );
        // reset all method pointers in table
        memset(&(ir.first->second), 0, sizeof(mwl_ForeignTypeTable));
        ir.first->second.cppTypeID = &typeid(T);
        // emplace new type in registered
        _registered.emplace(typeid(T), ir.first);
        return std::pair<Parent::iterator, bool>(ir.first, true);
    }
public:
    ForeignTypes() {}
    friend struct ::mwl_Definitions;
};
}  // namespace mwl

struct mwl_Definitions : public std::unordered_map< std::string
                                                  , struct mwl_Definition > {
    /// Reference to table of foreign types
    mwl::ForeignTypes & foreignTypes;

    /// Creates new (sub)scope of definitions
    mwl_Definitions(mwl::ForeignTypes & ftRef) : foreignTypes(ftRef) {}

    /// Alias for parent type
    typedef std::unordered_map<std::string, struct mwl_Definition> Parent;

    //
    // Constvals

    /// Explicitly defines integer constant
    Parent::iterator define_int_constval(const std::string &, mwl_Integer_t value=0);
    /// Explicitly defines floating point constant
    Parent::iterator define_float_constval(const std::string &, mwl_Float_t value=0.0);
    /// Explicitly defines string constant
    Parent::iterator define_str_constval(const std::string &, const std::string &);
 
    /// Resolves to int/float/string values
    template<typename T> Parent::iterator
    define_constval(const std::string & nm, T value) {
        return ConstValTraits<T>::define(*this, nm, value);
    }

    //
    // Foreign types

    /**\brief Introduces new foreign type or returns iterator to defined eponymous
     *
     * Foreign types are the subjects of operations, selectors and further
     * dereferencing. They are more difficult to maintain since they require
     * explicit definition of appliable operations. */
    template<typename T> Parent::iterator
    define_foreign_type( const std::string & nm ) {
        auto fir = foreignTypes.define<T>();
        auto ir = emplace( nm
            , mwl_Definition{ mwl_Definition::mwl_kDefForeignType
                            , 0x0  // init foreign type data ID later on here
                            });
        if( ! ir.second ) {
            if( ! fir.second ) {
                // same type, same name -- just return existing entry
                return ir.first;
            }
            // different types, same name -- consider it as an error
            throw mwl::error::DuplicatingDefinitionError(ir.first);
        }
        mwl_Definition & def = ir.first->second;
        def.dataType = fir.first->first;
        def.pl.asForeignTypeTable = &(fir.first->second);
        assert(def.pl.asForeignTypeTable);
        return ir.first;
    }

    //
    // Namespaces

    /**\brief Defines a namespace.
     *
     * Note, that created namespace will have its `dataType` field undefined.
     * Namespaces can not be the subject of operations or selectors and are
     * introduced just for convenience -- to avoid name conflicts, etc. They
     * always have to be the subject of further dereferencing before they
     * participate in expressions.
     *
     * For instance `m` namespace for math functions and constants (`sin()`,
     * `log()`, `pi`, `exp()`, etc).
     * */
    Parent::iterator define_namespace(const std::string &);

    /// Recursively deletes all subsections
    ~mwl_Definitions();
};

//
// Tratis specialization

template<typename T>
struct ConstValTraits<T, typename std::enable_if<std::is_integral<T>::value>::type> {
    static std::unordered_map<std::string, struct mwl_Definition>::iterator
    define(mwl_Definitions & defs, const std::string & nm, T value) {
        return defs.define_int_constval(nm, value);
    }
};

template<typename T>
struct ConstValTraits<T, typename std::enable_if<std::is_floating_point<T>::value>::type> {
    static std::unordered_map<std::string, struct mwl_Definition>::iterator
    define(mwl_Definitions & defs, const std::string & nm, T value) {
        return defs.define_float_constval(nm, value);
    }
};

template<>
struct ConstValTraits<std::string, void> {
    static std::unordered_map<std::string, struct mwl_Definition>::iterator
    define(mwl_Definitions & defs, const std::string & nm, const std::string & value) {
        return defs.define_str_constval(nm, value);
    }
};

extern "C" {
#endif

const struct mwl_Definition *
mwl_find_definition( const struct mwl_Definitions *
                   , const char * identifier );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif
