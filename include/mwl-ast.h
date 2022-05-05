#ifndef H_MEOWLANG_AST_H
#define H_MEOWLANG_AST_H

#include "mwl-defs.h"
#include "mwl-func.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mwl_Workspace;

enum mwl_NodeType {
    mwl_kConstValue,    /* Constant, literal or any other immediately evaluated */
    mwl_kOperation,     /* Operation (binary, arithmetic, logic, comparison) */
    mwl_kFunction,      /* External function call */
    mwl_kParameter,     /* External parameter resolved at a runtime */
    mwl_kVariable,      /* Variable set before the execution */
    mwl_kNamespace,     /* Reference to namespace definitions */
};

/** Represents unary or binary operation
 * NOTE: `b` can be null for unary operations */
struct mwl_Op {
    mwl_OpCode_t code;
    struct mwl_ASTNode * a
                     , * b;
};

/** Common AST node structure */
struct mwl_ASTNode {
    enum mwl_NodeType nodeType;
    mwl_TypeCode_t dataType;
    union {
        struct mwl_ConstVal     asConstVal;     /* for kConstValue */
        struct mwl_Op           asOp;           /* for kOperation */
        struct mwl_Func         asFunction;     /* for kFunction */
        struct mwl_Parameter    asParameter;    /* for kParameter */
        struct mwl_Variable     asVariable;     /* for kVariable */
        struct mwl_Definitions* asNamespace;    /* for kDefinitions */
    } pl;
};

/** Places copies of the nodes, infers type, etc */
int mwl_init_op_node( struct mwl_ASTNode * dest
                    , struct mwl_ASTNode * left
                    , mwl_OpCode_t
                    , struct mwl_ASTNode * right
                    , struct mwl_Workspace *
                    );

/** Makes shallow copy of the given AST node */
struct mwl_ASTNode * mwl_shallow_copy_node(struct mwl_ASTNode *);

/** Resolves symbol by identifier name
 * 
 * Takes pointer to a `mwl_Definitions` instance (a C++ dictionary of
 * definitions), the name of the identifier and, optionally, a previous
 * identifiers list. */
int
mwl_resolve_identifier_to_ast( struct mwl_ASTNode * dest
          , const struct mwl_Definitions * dict
          , const char * strtok
          );

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  /* H_MEOWLANG_AST_H */
