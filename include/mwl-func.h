#ifndef H_MWL_FUNCTION_H
#define H_MWL_FUNCTION_H

#include "mwl-types.h"

struct mwl_ASTNode;
struct mwl_ArgsList;

/** Foreign function call (a C/C++ procedure) */
struct mwl_FuncDef {
    /* ... TODO: whatever we need here to use in CLANG IR? */
};

struct mwl_ArgsList {
    struct mwl_ASTNode * self;
    struct mwl_ArgsList * next;
};

/** Makes a shallow copy of arguments list node. */
struct mwl_ArgsList * mwl_shallow_copy_args( const struct mwl_ArgsList * );

/** Frees arguments list */
void mwl_free_args_list( struct mwl_ArgsList * );

/** Payload part of AST node representing foreign function call with certain
 * arguments */
struct mwl_Func {
    /** Function definition */
    const struct mwl_FuncDef * funcdef;
    /* ... args list structures */
    struct mwl_ArgsList argsList;
};

/** For given function definition and list of arguments, computes the return
 * type code */
mwl_TypeCode_t mwl_resolve_func_return_type( const struct mwl_FuncDef *
                                           , const struct mwl_ArgsList *
                                           );

#endif  /* H_MWL_FUNCTION_H */

