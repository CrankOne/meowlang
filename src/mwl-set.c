#include "mwl-set.h"
#include "mwl-ast.h"
#include "mwl-ws.h"

#include <assert.h>
#include <string.h>

mwl_TypeCode_t
mwl_induce_set_type( const struct mwl_ArgsList * args
                   , struct mwl_Workspace * ws
                   ) {
    char bf1[64];
    size_t n = 0;
    mwl_TypeCode_t r = 0;
    for( const struct mwl_ArgsList * c = args
       ; c
       ; c = c->next, ++n ) {
        mwl_TypeCode_t thisType = c->self->dataType;
        /* TODO: if it is a foreign type? */
        #warning "FIXME: foreign type is not considered by set type deduction"
        /* check general validity: we can not use sets and maps within a set */
        if( thisType & mwl_kFIsCollection ) {
            snprintf( ws->errMsg, ws->errMsgSize
                    , "failed to infer type of the homeogeneous set;"
                      " entry #%zu has type `%s' (set can not contain collections)"
                    , n+1
                    , mwl_to_str_type(bf1, sizeof(bf1), thisType)
                    );
            return 0;
        }
        if( thisType == mwl_kTpString ) {
            if( !r ) {
                r = mwl_kTpString;
                continue;
            }
            if( r != mwl_kTpString ) {
                snprintf( ws->errMsg, ws->errMsgSize
                        , "failed to infer type of the homeogeneous set;"
                          " entry #%zu has type `%s' instead of string as previous element(s)"
                        , n+1
                        , mwl_to_str_type(bf1, sizeof(bf1), thisType)
                        );
                return 0;
            }
            continue;
        }
        if( thisType & mwl_kFIsNumeric ) {
            if( !r ) {
                r = thisType;
                continue;
            }
            if( r == thisType ) continue;
            if( r == mwl_kTpInteger && thisType == mwl_kTpFloat ) {
                r = mwl_kTpFloat;
                continue;
            }
            assert( (r == thisType && thisType == mwl_kTpFloat)
                 || (r == mwl_kTpFloat && thisType == mwl_kTpInteger ));
            continue;
        }
        snprintf( ws->errMsg, ws->errMsgSize
                , "failed to infer type of the homeogeneous set;"
                  " entry #%zu has type `%s' instead of string as previous element(s)"
                , n+1
                , mwl_to_str_type(bf1, sizeof(bf1), thisType)
                );
        return 0;
    }
    if(!r) {
        strncpy( ws->errMsg
               , "failed to infer type of empty set"
               , ws->errMsgSize );
    }
    return r;
}

mwl_TypeCode_t
mwl_induce_map_type( const struct mwl_MapPairList * pairs
                   , struct mwl_Workspace * ws
                   ) {
    char bf1[64], bf2[64];
    size_t n = 0;
    mwl_TypeCode_t valueType = 0;
    struct mwl_ArgsList * argsList = NULL
                      , * prev = NULL;
    /* Build a set and use set induction method for keys (we want the key type
     * to be hashable or something; also try to find the common value type */
    for( const struct mwl_MapPairList * c = pairs; c; c = c->next, ++n ) {
        struct mwl_ArgsList * ca = malloc(sizeof(struct mwl_ArgsList));
        ca->self = c->key;
        ca->next = NULL;
        if( argsList ) {
            prev->next = ca;
            prev = ca;
        } else {
            argsList = prev = ca;
        }
        /* Consider value type */
        mwl_TypeCode_t thisType = c->value->dataType;
        if( !valueType ) {
            valueType = c->value->dataType;
            assert(valueType);
            continue;
        }
        if( valueType == thisType ) continue;
        /* For case of not equal value types we have only one exception, for
         * numerical type */
        if( (valueType == mwl_kTpInteger && thisType == mwl_kTpFloat)
         || (valueType == mwl_kTpFloat   && thisType == mwl_kTpInteger)) {
            valueType = mwl_kTpFloat;
            continue;
        }
        snprintf( ws->errMsg, ws->errMsgSize
                , "failed to infer value (mapped) type of the map;"
                  " entry #%zu has type `%s' instead of `%s' as previous"
                  " element(s)"
                , n+1
                , mwl_to_str_type(bf1, sizeof(bf1), thisType)
                , mwl_to_str_type(bf2, sizeof(bf2), valueType)
                );
        mwl_free_args_list(argsList);
        return 0;
    }
    mwl_TypeCode_t keyType = mwl_induce_set_type(argsList, ws);
    if(!keyType) return 0;
    mwl_free_args_list(argsList);
    if( keyType & (mwl_kFIsCollection | mwl_kFIsForeign | mwl_kFIsMap) ) {
        snprintf( ws->errMsg, ws->errMsgSize
                , "can not use type `%s' as key type for the map"
                , mwl_to_str_type(bf1, sizeof(bf1), keyType)
                );
        return 0x0;
    }
    return mwl_combine_map_type(keyType, valueType);
}


struct mwl_MapPairList *
mwl_shallow_copy_map_entries( struct mwl_MapPairList * o ) {
    struct mwl_MapPairList * r = malloc(sizeof(struct mwl_MapPairList));
    *r = *o;
    return r;
}

