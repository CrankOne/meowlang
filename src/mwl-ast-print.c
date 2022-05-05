#include "mwl-ast-print.h"
#include "mwl-ast.h"
#include "mwl-types.h"
#include "mwl-ops.h"

size_t
mwl_to_str_constval( char * buf
                   , size_t n
                   , const struct mwl_ConstVal * obj
                   ) {
    if( obj->dataType == mwl_kTpLogic )
        return snprintf(buf, n, "%s", obj->pl.asInteger ? "true" : "false");
    if( obj->dataType == mwl_kTpInteger )
        return snprintf(buf, n, "%ld", (long int) obj->pl.asInteger );
    if( obj->dataType == mwl_kTpFloat )
        return snprintf(buf, n, "%f", (double) obj->pl.asFloat );
    if( obj->dataType == mwl_kTpString )
        return snprintf(buf, n, "\"%s\"", obj->pl.asString );
    return snprintf(buf, n, "???" );  // TODO: hex dump of data
}

void
mwl_dump_AST( FILE * stream
            , const struct mwl_ASTNode * node
            , int indent
            ) {
    fprintf(stream, "%*c", indent, ' ');
    if(!node) {
        fprintf(stream, "(null node)\n");
        return;
    }
    char nodeDumpBuf[128];
    switch( node->nodeType ) {
        case mwl_kConstValue: {
            mwl_to_str_constval(nodeDumpBuf, sizeof(nodeDumpBuf), &(node->pl.asConstVal));
            fprintf(stream, "constval:%s", nodeDumpBuf);
        break; };
        case mwl_kOperation: {
            fprintf( stream, "op:`%s' <%s>\n"
                   , mwl_to_str_op(node->pl.asOp.code)
                   , mwl_to_str_type(node->dataType)
                   );
            mwl_dump_AST(stream, node->pl.asOp.a, indent + 2);
            if(node->pl.asOp.b)
                mwl_dump_AST(stream, node->pl.asOp.b, indent + 2);
            return;  // goto end?
        break; };
        default:
            fprintf(stream, "%p\n", node);
        // ...
    };
    fprintf(stream, " <%s>\n", mwl_to_str_type(node->dataType));
}

