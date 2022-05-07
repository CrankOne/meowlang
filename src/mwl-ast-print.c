#include "mwl-ast-print.h"
#include "mwl-ast.h"
#include "mwl-types.h"
#include "mwl-ops.h"

#include <assert.h>
#include <string.h>

size_t
mwl_to_str_constval( char * buf
                   , size_t n
                   , const struct mwl_ConstVal * obj
                   ) {
    if( obj->dataType == mwl_kTpLogic )
        return snprintf(buf, n, "%s:bool", obj->pl.asInteger ? "true" : "false");
    if( obj->dataType == mwl_kTpInteger )
        return snprintf(buf, n, "%ld:int", (long int) obj->pl.asInteger );
    if( obj->dataType == mwl_kTpFloat )
        return snprintf(buf, n, "%e:float", (double) obj->pl.asFloat );
    if( obj->dataType == mwl_kTpString )
        return snprintf(buf, n, "\"%s\":str", obj->pl.asString );
    return snprintf(buf, n, "???:(type#%#x)", (int) obj->dataType );  // TODO: hex dump of data
}

#if 1
static void
_impl_mwl_dump_AST( FILE * stream
                  , const struct mwl_ASTNode * node
                  , int depth
                  , char * prefix
                  ) {
    fputs(prefix, stream);
    for( char * c = prefix; *c != '\0'; ++c ) if('`' == *c) *c = ' ';
    if(!node) {
        fputs("(null node)\n", stream);
        return;
    }
    //char * endPtr = prefix + strlen(prefix);
    char buf[128];
    switch( node->nodeType ) {
        case mwl_kConstValue:
            fputs("- ", stream);
            mwl_to_str_constval(buf, sizeof(buf), &(node->pl.asConstVal));
            fputs(buf, stream);
            fputc('\n', stream);
            return;

        case mwl_kOperation:
            fprintf( stream, "+- %s:%s\n"
                   , mwl_to_str_op(node->pl.asOp.code)
                   , mwl_to_str_type(buf, sizeof(buf), node->dataType)
                   );
            if(depth) {
                strcat(prefix, node->pl.asOp.b ? "|" : "`");
                _impl_mwl_dump_AST(stream, node->pl.asOp.a, depth + 1, prefix);
                prefix[strlen(prefix)-1] = '\0';
            }
            if(node->pl.asOp.b && depth) {
                strcat(prefix, "`");
                _impl_mwl_dump_AST(stream, node->pl.asOp.b, depth + 1, prefix);
                prefix[strlen(prefix)-1] = '\0';
            }
            return;  // goto end?

        case mwl_kSet:
            fprintf( stream, "+- %s\n"
                   , mwl_to_str_type(buf, sizeof(buf), node->dataType)
                   );
            if(depth) {
                strcat(prefix, node->pl.asSet.values.next ? "|" : "`");
                for( const struct mwl_ArgsList * c = &(node->pl.asSet.values)
                   ; c
                   ; c = c->next ) {
                    if( ! c->next )
                        prefix[strlen(prefix)-1] = '`';
                    _impl_mwl_dump_AST(stream, c->self, depth + 1, prefix);
                }
                prefix[strlen(prefix)-1] = '\0';
            }
            return;

        case mwl_kMap:
            fprintf( stream, "+- %s\n"
                   , mwl_to_str_type(buf, sizeof(buf), node->dataType)
                   );
            if(depth) {
                size_t n = 0;
                strcat(prefix, node->pl.asMap.values.next ? "|" : "`");
                for( const struct mwl_MapPairList * c = &(node->pl.asMap.values)
                   ; c
                   ; c = c->next, ++n ) {
                    if( ! c->next )
                        prefix[strlen(prefix)-1] = '`';
                    fprintf(stream, "%s+- (node #%zu)\n", prefix, n+1);
                    for( char * c = prefix; *c != '\0'; ++c ) if('`' == *c) *c = ' ';
                    strcat(prefix, "|" );
                    _impl_mwl_dump_AST(stream, c->key,   depth + 1, prefix);
                    prefix[strlen(prefix)-1] = '`';
                    _impl_mwl_dump_AST(stream, c->value, depth + 1, prefix);
                    prefix[strlen(prefix)-1] = '\0';
                }
                prefix[strlen(prefix)-1] = '\0';
            }
            return;

        //#ifdef NDEBUG
        //default:
        //    fprintf(stream, "%p", node);
        //#endif
        // ...
    };
    fprintf(stream, " <%s>\n", mwl_to_str_type(buf, sizeof(buf), node->dataType));
}

void
mwl_dump_AST( FILE * stream
            , const struct mwl_ASTNode * node
            , int depth
            ) {
    char prefix[128] = "";
    _impl_mwl_dump_AST(stream, node, depth, prefix);
}
#endif

#if 0
struct ReentrantPrintObject {
    char prefix[512];  /* TODO: configurable parameter */
    char * prefixEnd;
    FILE * dest;
};

static int
_print_node( struct mwl_ASTNode * node
            , int depth
            , void * data
            ) {
    struct ReentrantPrintObject * rpo = (struct ReentrantPrintObject *) data;
    fputs(rpo->prefix, rpo->dest);
    //fprintf(rpo->dest, "[%d] ", depth);
    switch( node->nodeType ) {
        case mwl_kOperation:
        case mwl_kFunction:
            fputc('+', rpo->dest);
            *(rpo->prefixEnd) = '|';
            *(++(rpo->prefixEnd)) = '\0';
            if( node->nodeType == mwl_kOperation ) {
                fprintf( rpo->dest, "op:`%s' <%s>"
                       , mwl_to_str_op(node->pl.asOp.code)
                       , mwl_to_str_type(node->dataType)
                       );
            } else {
                fprintf( rpo->dest, "<func>"  // TODO ...
                       );
            }
            break;
        /* ... add other composite nodes here  */
        case mwl_kConstValue:
        case mwl_kParameter:
        case mwl_kVariable:
        case mwl_kNamespace:
            fputc('-', rpo->dest);
    };
    //++(rpo->prefixEnd);
    //assert(rpo->prefixEnd >= rpo->prefix);
    //*(rpo->prefixEnd) = '\0';
    fputc('\n', rpo->dest);
    return 0;
}

void
mwl_dump_AST( FILE * stream
            , const struct mwl_ASTNode * node
            , int indent
            , int depth
            ) {
    struct ReentrantPrintObject rpo = { "", NULL, stream };
    rpo.prefixEnd = rpo.prefix;
    mwl_AST_for_all_recursively( (struct mwl_ASTNode *) node
                               , _print_node
                               , &rpo );
}
#endif

