#ifndef H_MWL_AST_PRINT_H
#define H_MWL_AST_PRINT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mwl_ASTNode;
struct mwl_ConstVal;

size_t mwl_to_str_constval( char * buf
                          , size_t n
                          , const struct mwl_ConstVal * obj );

/** Dumps AST tree in console */
void mwl_dump_AST( FILE * stream
                 , const struct mwl_ASTNode * node
                 , int depth );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  /* H_MWL_AST_PRINT_H */

