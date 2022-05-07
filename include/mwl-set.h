#ifndef H_MWL_SET_H
#define H_MWL_SET_H

#include "mwl-types.h"
#include "mwl-func.h"

struct mwl_ASTNode;
struct mwl_Workspace;

struct mwl_Set {
    mwl_TypeCode_t dataType;
    struct mwl_ArgsList values;
};

mwl_TypeCode_t
mwl_induce_set_type( const struct mwl_ArgsList *
                   , struct mwl_Workspace *
                   );


struct mwl_MapPairList {
    struct mwl_ASTNode * key
                     , * value
                     ;
    struct mwl_MapPairList * next;
};

struct mwl_Map {
    mwl_TypeCode_t dataType;
    struct mwl_MapPairList values;
};

mwl_TypeCode_t
mwl_induce_map_type( const struct mwl_MapPairList *
                   , struct mwl_Workspace * );

struct mwl_MapPairList *
mwl_shallow_copy_map_entries( struct mwl_MapPairList * );

#endif  /* H_MWL_SET_H */
