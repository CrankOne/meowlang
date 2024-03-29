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
    node->isVisited = 0;
    node->userdata = NULL;
    return result;
}

int
mwl_init_op_node( struct mwl_ASTNode * dest
                , struct mwl_ASTNode * left
                , mwl_OpCode_t opCode
                , struct mwl_ASTNode * right
                , struct mwl_Workspace * ws
                ) {
    char buf1[64], buf2[64];
    mwl_OpCode_t tp = mwl_infer_type( left->dataType, opCode
                                    , right ? right->dataType : 0x0
                                    , ws->errMsg, ws->errMsgSize );
    if( (left->dataType != 0x0) && ( right ? 0x0 != right->dataType : 0x1 )
     && 0x0 == tp ) {
        if('\0' == ws->errMsg[0]) {
            if( opCode & kOp_FUnary ) {
                assert(!right);
                snprintf( ws->errMsg, ws->errMsgSize
                        , "failed to infer resulting type of the unary"
                          " operation %s with operand type `%s'"
                        , mwl_to_str_op(opCode)
                        , mwl_to_str_type(buf1, sizeof(buf1), left->dataType)
                        );
            } else {
                assert(right);
                snprintf( ws->errMsg, ws->errMsgSize
                        , "failed to infer resulting type of the binary"
                          " operation %s with operands types `%s' and `%s'"
                        , mwl_to_str_op(opCode)
                        , mwl_to_str_type(buf1, sizeof(buf1), left->dataType)
                        , mwl_to_str_type(buf2, sizeof(buf2), right->dataType)
                        );
            }
        }
        return MWL_ERROR_INCOMPAT_TYPE;
    }

    dest->nodeType = mwl_kOperation;
    dest->dataType = tp;
    dest->pl.asOp.code = opCode;
    dest->pl.asOp.a = mwl_shallow_copy_node(left);
    if(right)
        dest->pl.asOp.b = mwl_shallow_copy_node(right);

    dest->userdata = NULL;
    dest->isVisited = 0;
    dest->selector = NULL;

    return 0;
}

int
mwl_resolve_identifier_to_ast( struct mwl_ASTNode * dest
                             , const struct mwl_Definitions * dict
                             , char * strtok
                             , int fl, int fc
                             , int ll, int lc
                             ) {
    assert(dest && dict && strtok);
    const struct mwl_Definition * cDef = mwl_find_definition(dict, strtok);
    if( !cDef ) { /* definition not found */
        dest->nodeType = mwl_kUnresolvedIdentifier;
        dest->dataType = 0x0;
        dest->pl.asUnresolved.name = strtok;
        dest->pl.asUnresolved.pos[0][0] = fl;
        dest->pl.asUnresolved.pos[0][1] = fc;
        dest->pl.asUnresolved.pos[0][0] = ll;
        dest->pl.asUnresolved.pos[0][1] = lc;
        return 1;
    }
    dest->isVisited = 0;
    dest->userdata = NULL;
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
            #warning "FIXME: function returned data type resolution"
            dest->dataType = 0x0;
            /* ^^^ NOTE: function returned data type is not resolved until
             * argslist is provided */
            dest->pl.asFunction.funcdef = &(cDef->pl.asFuncdef);
            return 0;
        default:
            /* This shall never happen unless the unforeseen definitions type. */
            return -2;
    };
}

static int
_impl_mwl_AST_for_all_recursively( int depth
                 , struct mwl_ASTNode * root
                 , int (*callback)(struct mwl_ASTNode *, int, void *)
                 , void * data ) {
    assert(root);
    assert(callback);
    int rc;
    rc = callback(root, depth, data);
    if(rc) return rc;
    switch( root->nodeType ) {
        case mwl_kOperation:
            if((rc = _impl_mwl_AST_for_all_recursively(depth+1, root->pl.asOp.a, callback, data)))
                return rc;
            if( root->pl.asOp.b
             && (rc = _impl_mwl_AST_for_all_recursively(depth+1, root->pl.asOp.b, callback, data)))
                    return rc;
            return 0;
        case mwl_kFunction:
            /* Iterate over arguments */
            for( struct mwl_ArgsList * c = &(root->pl.asFunction.argsList)
               ; c && c->self
               ; c = c->next ) {
                if((rc = _impl_mwl_AST_for_all_recursively(depth+1, c->self, callback, data)))
                    return rc;
            }
            return 0;
        case mwl_kSet:
            /* Iterate over elements of the set */
            for( struct mwl_ArgsList * c = &(root->pl.asSet.values)
               ; c && c->self
               ; c = c->next ) {
                if((rc = _impl_mwl_AST_for_all_recursively(depth+1, c->self, callback, data)))
                    return rc;
            }
            return 0;
        case mwl_kMap:
            /* Iterate over elements of the map */
            for( struct mwl_MapPairList * c = &(root->pl.asMap.values)
               ; c
               ; c = c->next ) {
                if((rc = _impl_mwl_AST_for_all_recursively(depth+1, c->key, callback, data)))
                    return rc;
                if((rc = _impl_mwl_AST_for_all_recursively(depth+1, c->value, callback, data)))
                    return rc;
            }
            return 0;
        /* ... add other composite nodes here  */
        case mwl_kUnresolvedIdentifier:
        case mwl_kConstValue:
        case mwl_kVariable:
        case mwl_kNamespace:
            return 0;
    };
    assert(0);
}

int
mwl_AST_for_all_recursively( struct mwl_ASTNode * root
                           , int (*callback)(struct mwl_ASTNode *, int, void *)
                           , void * data ) {
    return _impl_mwl_AST_for_all_recursively(0, root, callback, data);
}


static int
_reset_visited( struct mwl_ASTNode * node, int depth, void * data ) {
    node->isVisited = 0;
    return 0;
}

void
mwl_AST_reset_visited( struct mwl_ASTNode * root ) {
    mwl_AST_for_all_recursively(root, _reset_visited, NULL);
}

#if 0
procedure DFS_iterative(G, v) is
    let S be a stack
    S.push(v)
    while S is not empty do
        v = S.pop()
        if v is not labeled as discovered then
            label v as discovered
            for all edges from v to w in G.adjacentEdges(v) do 
                S.push(w)
#endif
int
mwl_AST_dfs( struct mwl_ASTNode * root
           , int (*callback)(struct mwl_ASTNode *, int, void *)
           , void * data
           ) {
    struct mwl_ASTNode * v;
    struct NodesList {
        struct mwl_ASTNode * node;
    } stack[512] = {{root}}     /* TODO: configurable limit? */
    , * top = stack
    , * stackEnd = stack + sizeof(stack)/sizeof(struct NodesList)
    ;
    top->node = root; ++top;  // push
    while( top != stack ) {
        --top; v = top->node; assert(v);  // pop
        if( ! v->isVisited ) {
            v->isVisited = 1;
            if( callback(v, top - stack, data) ) return 1;
            /* push descendants */
            switch( v->nodeType ) {
                case mwl_kOperation:
                    // push b if has
                    if(v->pl.asOp.b) {
                        top->node = v->pl.asOp.b;
                        if(++top == stackEnd) return -1;
                    }
                    // push a
                    top->node = v->pl.asOp.a;
                    assert(top->node);
                    if(++top == stackEnd) return -1;
                    continue;
                case mwl_kConstValue:
                case mwl_kVariable:
                    continue;
                    //callback(top->node, data);
                default:
                    return -2;
            };
        }
    }
    return 0;
}

/*                                                          ___________________
 * _______________________________________________________/ Topological sort */

struct ListEntry {
    struct mwl_ASTNode * node;
    int depth;
    struct ListEntry * next;
};

struct ReentrantTSortObject {
    struct ListEntry * head
                   , * tail
                   ;
};

/* An internal helper function used to build an order of nodes */
static int
_tsort_accumulate( struct mwl_ASTNode * node
                 , int depth
                 , void * rtso_ ) {
    struct ReentrantTSortObject * rtso = (struct ReentrantTSortObject *) rtso_;
    struct ListEntry * le = malloc(sizeof(struct ListEntry));
    le->node = node;
    le->depth = depth;
    le->next = rtso->tail;
    rtso->tail = le;
    return 0;
}

int
mwl_AST_for_all_tsorted( struct mwl_ASTNode * root
                       , int (*callback)(struct mwl_ASTNode *, int, void *)
                       , void * data ) {
    /* Build list of object to establish ordering */
    struct ReentrantTSortObject rtso = {NULL, NULL};
    mwl_AST_dfs( root, _tsort_accumulate, &rtso );
    /* Iterate over list backwards */
    int rc = 0;
    for( struct ListEntry * c = rtso.tail
       ; c ; ) {
        c->node->isVisited = 0;
        if( 0 == rc ) {
            rc = callback(c->node, c->depth, data);
        }
        struct ListEntry * toDelete = c;
        c = c->next;
        free(toDelete);
    }
    return rc;
}

/*                                                       ______________________
 * ____________________________________________________/ Selector resolution */

int
mwl_resolve_selector_context( struct mwl_ASTNode * selectorNode
                            , struct mwl_ASTNode * expression
                            ) {
    /* Selectors are valid for:
     *  - variables of foreign types (in this case foreign type
     *    shall define additional selector context)
     *  - (todo) collections indexed with ordinary types, in which case
     *    selector result has its data type set and it must match
     *    to the collection's key type
     *  - (todo) collections indexed with foreign types which may define
     *    a sub-context specifically to selector expressions.
     */
    // ...
    //#error "Last TODO stub"
    return 0x0;  // TODO
}

#ifndef NO_PLAIN_EVAL
/*                                                          ___________________
 * _______________________________________________________/ Plain evaluation */

struct ReentrantEvaluationObject {
    struct mwl_ConstVal * result;
};

static int
_plain_eval_op( struct mwl_ASTNode * node
              , void * reo_ ) {
    assert(node->nodeType == mwl_kOperation);
    struct mwl_Op * op = &(node->pl.asOp);
    struct ReentrantEvaluationObject * reo
            = (struct ReentrantEvaluationObject *) reo_;
    /* Find plain evaluator function for the operation. Note, that this
     * evaluator have to be applied to scalar nodes, even if provided op code
     * is on-set operation */
    mwl_PlainNodeEvaluator evaluator
        = mwl_op_get_plain_eval(op->code);
    
    /* For operands, take either constval node directly, or rely on `userdata'
     * for the result of previously computed expressions */
    assert( op->a );
    assert( op->a->nodeType == mwl_kConstValue || NULL != op->a->userdata );
    struct mwl_ConstVal * a = op->a->nodeType == mwl_kConstValue
                            ? &(op->a->pl.asConstVal)
                            : (struct mwl_ConstVal *) op->a->userdata
                            ;
    assert(a);
    struct mwl_ConstVal * b = NULL;
    if(op->b) {
        assert( op->b->nodeType == mwl_kConstValue || NULL != op->b->userdata );
        b = op->b->nodeType == mwl_kConstValue
          ? &(op->b->pl.asConstVal)
          : (struct mwl_ConstVal *) op->b->userdata
          ;
        assert(b);
    }
    struct mwl_ConstVal * r = malloc(sizeof(struct mwl_ConstVal));
    memset(r, 0x0, sizeof(struct mwl_ConstVal));
    int rc = evaluator(a, b, r);
    if(rc) {
        /* TODO: put details in reo */
        return rc;
    }
    if( r->dataType != node->dataType ) {
        /* TODO: put details in reo */
        return -100;
    }
    node->userdata = r;
    return 0;
}

static int
_plain_eval_ast( struct mwl_ASTNode * node
               , int depth
               , void * reo_
               ) {
    /* skip constval nodes */
    if(node->nodeType == mwl_kConstValue) return 0;
    if(node->nodeType == mwl_kOperation)
        return _plain_eval_op(node, reo_);
    return 1;  /* unimplemented node? */
}

int
mwl_AST_eval( struct mwl_ASTNode * root
            , struct mwl_ConstVal * result
            ) {
    struct ReentrantEvaluationObject reo = {result};
    int rc = mwl_AST_for_all_tsorted( root
                                    , _plain_eval_ast
                                    , &reo);
    if(0 == rc) {
        assert(root->userdata);
        *result = *((struct mwl_ConstVal *) root->userdata);
    }
    /* TODO free userdata fields recursively here */
    return rc;
}
#endif  /* NO_PLAIN_EVAL */
