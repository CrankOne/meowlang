#ifndef H_MWL_AST_PRINT_H
#define H_MWL_AST_PRINT_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mwl_ASTNode;

/** Dumps AST tree in console */
void mwl_dump_AST( FILE * stream
                 , const struct mwl_ASTNode * node
                 , int indent
                 );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  /* H_MWL_AST_PRINT_H */

