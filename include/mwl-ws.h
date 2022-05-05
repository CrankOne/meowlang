#ifndef H_MWL_WORKSPACE_H
#define H_MWL_WORKSPACE_H

#include <stdio.h>

struct mwl_Definitions;
struct mwl_ASTNode;

typedef struct mwl_Workspace {
    /** Reference to foreign symbols table */
    const struct mwl_Definitions * defs;
    /** Error message buffer */
    char * errMsg;
    unsigned int errMsgSize;
    /** Problematic fragment: first line, column, last line, column */
    uint32_t errPos[2][2];
    /* Expression root node's */
    struct mwl_ASTNode * root;
    /* Parser debug messages stream (may be nullptr) */
    FILE * dbgStream;
} mwl_Workspace_t;

#endif  /* H_MWL_WORKSPACE_H */

