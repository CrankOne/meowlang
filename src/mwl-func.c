#include "mwl-func.h"

struct mwl_ArgsList *
mwl_shallow_copy_args( const struct mwl_ArgsList * src ) {
    struct mwl_ArgsList * r = malloc(sizeof(struct mwl_ArgsList));
    *r = *src;
    return r;
}

mwl_TypeCode_t
mwl_resolve_func_return_type( const struct mwl_FuncDef * funcDef
                            , const struct mwl_ArgsList * args
                            ) {
    // TODO
    return 0x0;
}

