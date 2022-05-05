#include "mwl-ast.h"
#include "mwl-ws.h"
#include "mwl-errors.h"
#include "mwl-defs.h"
#include "mwl-ops.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

struct mwl_ASTNode *
mwl_shallow_copy_node(struct mwl_ASTNode * node) {
    struct mwl_ASTNode * result = malloc(sizeof(struct mwl_ASTNode));
    memcpy(result, node, sizeof(struct mwl_ASTNode));
    return result;
}

int
mwl_init_op_node( struct mwl_ASTNode * dest
                , struct mwl_ASTNode * left
                , mwl_OpCode_t opCode
                , struct mwl_ASTNode * right
                , struct mwl_Workspace * ws
                ) {
    mwl_OpCode_t tp = mwl_infer_type( left->dataType, opCode, right->dataType
                                    , ws->errMsg, ws->errMsgSize );
    if(!tp) {
        if('\0' == ws->errMsg[0]) {
            snprintf( ws->errMsg, ws->errMsgSize
                    , "failed to infer resulting type of the operation" );
        }
        return MWL_ERROR_INCOMPAT_TYPE;
    }
    dest->nodeType = mwl_kOperation;
    dest->dataType = tp;
    dest->pl.asOp.code = opCode;
    dest->pl.asOp.a = mwl_shallow_copy_node(left);
    if(right)
        dest->pl.asOp.b = mwl_shallow_copy_node(right);
    return 0;
}

/**\brief Resolves a string identifier into constval, namespace, function, etc
 *
 * This function is called during parsing and shall initialize new AST node
 * (provided by `dest` argument) with respect to previously known context
 * (like namespace resolution chain).
 *
 * \returns -1 if no definition found
 * */
int
mwl_resolve_identifier_to_ast( struct mwl_ASTNode * dest
                             , const struct mwl_Definitions * dict
                             , const char * strtok
                             ) {
    assert(dest && dict && strtok);
    const struct mwl_Definition * cDef = mwl_find_definition(dict, strtok);
    if( !cDef ) return -1;  /* definition not found */
    switch(cDef->type) {
        case mwl_kDefConstval:
            /* definition aliases ordinary value (math or physical constant) */
            dest->nodeType = mwl_kConstValue;
            dest->dataType = cDef->pl.asConstVal.dataType;
            dest->pl.asConstVal = cDef->pl.asConstVal;
            return 0;
        case mwl_kDefNamespace:
            /* definition refers to a namespace, perhaps for further resolution */
            dest->nodeType = mwl_kNamespace;
            dest->dataType = cDef->dataType;
            assert(0x0 == dest->dataType);  /* NOTE: 0x0 or shall we have typed namespaces? */
            dest->pl.asNamespace = cDef->pl.asNamespacePtr;
            return 0;
        case mwl_kDefForeignCall:
            /* definition refers to a foreign function call */
            dest->nodeType = mwl_kFunction;
            dest->dataType = 0x0;
            /* ^^^ NOTE: function returned data type is not resolved until
             * argslist is provided */
            dest->pl.asFunction.funcdef = &(cDef->pl.asFuncdef);
            return 0;
        default:
            return -2;
    };
}

