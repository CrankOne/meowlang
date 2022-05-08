#include "mwl-defs.h"
#include "mwl-types.h"

#include <cassert>

const struct mwl_Definition *
mwl_find_definition( const struct mwl_Definitions * defs
                   , const char * identifier ) {
    assert(defs);
    mwl_Definitions::const_iterator it
        = defs->find(identifier);
    if(defs->end() == it) return NULL;
    return &(it->second);
}

mwl_Definitions::Parent::iterator
mwl_Definitions::define_int_constval( const std::string & name
                                    , mwl_Integer_t value ) {
    auto ir = emplace( name
            , mwl_Definition{ mwl_Definition::mwl_kDefConstval
                            , mwl_kTpInteger });
    if( !ir.second )
        throw mwl::error::DuplicatingDefinitionError(ir.first);
    ir.first->second.pl.asConstVal.dataType = mwl_kTpInteger;
    ir.first->second.pl.asConstVal.pl.asInteger = value;
    return ir.first;
}

mwl_Definitions::Parent::iterator
mwl_Definitions::define_float_constval( const std::string & name
                                      , mwl_Float_t value ) {
    auto ir = emplace( name
            , mwl_Definition{ mwl_Definition::mwl_kDefConstval
                            , mwl_kTpFloat });
    if( !ir.second )
        throw mwl::error::DuplicatingDefinitionError(ir.first);
    ir.first->second.pl.asConstVal.dataType = mwl_kTpFloat;
    ir.first->second.pl.asConstVal.pl.asFloat = value;
    return ir.first;
}

mwl_Definitions::Parent::iterator
mwl_Definitions::define_namespace( const std::string & name ) {
    auto ir = emplace( name
            , mwl_Definition{ mwl_Definition::mwl_kDefNamespace
                            , 0x0  //< NOTE: data type
                            });
    if( !ir.second )
        throw mwl::error::DuplicatingDefinitionError(ir.first);
    ir.first->second.pl.asNamespacePtr = new mwl_Definitions(foreignTypes);
    return ir.first;
}

mwl_Definitions::~mwl_Definitions() {
    for(auto p : *this) {
        if( mwl_Definition::mwl_kDefNamespace == p.second.type ) {
            delete p.second.pl.asNamespacePtr;
            p.second.pl.asNamespacePtr = nullptr;
        }
    }
}

