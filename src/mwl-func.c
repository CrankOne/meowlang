#include "mwl-func.h"

void
mwl_free_args_list( struct mwl_ArgsList * head ) {
    struct mwl_ArgsList * c = head;
    do {
        struct mwl_ArgsList * cc = c;
        c = c->next;
        free(cc);
    } while( c );
}

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
    #warning "TODO: resolve function return type"
    return 0x0;
}

