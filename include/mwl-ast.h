#ifndef H_MEOWLANG_AST_H
#define H_MEOWLANG_AST_H

#include "mwl-defs.h"
#include "mwl-func.h"
#include "mwl-ops.h"
#include "mwl-set.h"

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

struct mwl_Workspace;

enum mwl_NodeType {
    mwl_kConstValue,    /* Constant, literal or any other immediately evaluated */
    mwl_kOperation,     /* Operation (binary, arithmetic, logic, comparison) */
    mwl_kSet,           /* Set of values */
    mwl_kMap,           /* (Assotiative) array */
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
    /** Data type of this node. Note that highest bit is reserved for
     * "is visited" flag of algorithms like DFS */
    mwl_TypeCode_t dataType  : sizeof(mwl_TypeCode_t)*8 - 1;
    mwl_TypeCode_t isVisited : 1;
    union {
        struct mwl_ConstVal     asConstVal;     /* for kConstValue */
        struct mwl_Op           asOp;           /* for kOperation */
        struct mwl_Func         asFunction;     /* for kFunction */
        struct mwl_Set          asSet;          /* for kSet */
        struct mwl_Map          asMap;          /* for kMap */
        struct mwl_Parameter    asParameter;    /* for kParameter */
        struct mwl_Variable     asVariable;     /* for kVariable */
        struct mwl_Definitions* asNamespace;    /* for kDefinitions */
    } pl;
    void * userdata;
};

/**\brief Places copies of the nodes, infers type, etc */
int mwl_init_op_node( struct mwl_ASTNode * dest
                    , struct mwl_ASTNode * left
                    , mwl_OpCode_t
                    , struct mwl_ASTNode * right
                    , struct mwl_Workspace *
                    );

/**\brief Makes shallow copy of the given AST node */
struct mwl_ASTNode * mwl_shallow_copy_node(struct mwl_ASTNode *);

/**\brief Resolves symbol by identifier name
 * 
 * Takes pointer to a `mwl_Definitions` instance (a C++ dictionary of
 * definitions), the name of the identifier and, optionally, a previous
 * identifiers list. */
int mwl_resolve_identifier_to_ast( struct mwl_ASTNode * dest
                                 , const struct mwl_Definitions * dict
                                 , const char * strtok );

/*\brief Applies operation to every node recursively*/
int mwl_AST_for_all_recursively( struct mwl_ASTNode * root
                               , int (*callback)(struct mwl_ASTNode *, int, void *)
                               , void * data );

/*\brief Resets `isVisited` flag for all nodes in a subtree*/
void mwl_AST_reset_visited( struct mwl_ASTNode * root );

/**\brief DFS algorithm on AST, non-recursive
 *
 * Note, that pre-order traversal in case of trees is equivalent to topological
 * sort.
 * */
int mwl_AST_dfs( struct mwl_ASTNode * root
               , int (*callback)(struct mwl_ASTNode *, int, void *)
               , void * data );

/**\brief Call function on every node in topological order 
 *
 * This routine is useful for evaluation and code generation. It applies DFS
 * algorithm to get the descendant order (preorder traversal), and then
 * evluates the function in reversed order.
 * */
int mwl_AST_for_all_tsorted( struct mwl_ASTNode * root
                           , int (*callback)(struct mwl_ASTNode *, int, void *)
                           , void * data );

#ifndef NO_PLAIN_EVAL
/**\brief Evaluates the AST
 *
 * Tries to evaluate the AST to a value.
 * */
int mwl_AST_eval( struct mwl_ASTNode * root
                , struct mwl_ConstVal * result
                );
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  /* H_MEOWLANG_AST_H */
