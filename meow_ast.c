#include "meowlang_ast.h"

#include <string.h>
#include <stdlib.h>
#include <assert.h>

const mwl_TypeCode_t mwl_kFIsNumeric= 0x1;
const mwl_TypeCode_t mwl_kLogic     = (1 << 1);
const mwl_TypeCode_t mwl_kInteger   = (2 << 1) | mwl_kFIsNumeric;
const mwl_TypeCode_t mwl_kFloat     = (3 << 1) | mwl_kFIsNumeric;
const mwl_TypeCode_t mwl_kString    = 0x10 | 0x1;
/* ... other types */

#define M_DEFINE_OP_CODE( name, code, ... ) \
    const mwl_OpCode_t kOp_ ## name = code;
M_for_all_opcodes(M_DEFINE_OP_CODE)
#undef M_DEFINE_OP_CODE

const char * mwl_to_str_type( mwl_TypeCode_t code ) {
    if( mwl_kLogic == code )    return "logic";
    if( mwl_kInteger == code )  return "integer";
    if( mwl_kFloat == code )    return "float";
    if( mwl_kString == code )   return "string";
    static char bf[32];
    snprintf(bf, sizeof(bf), "?%#x?", code);
    return bf;
}

mwl_OpCode_t
mwl_to_opcode( const char * expr
             , mwl_Workspace_t * ws
             ) {  // TODO: check this
    assert(expr);
    size_t n = 0;
    mwl_OpCode_t opCode = 0x0;
    /* Check left complement */
    if(      '.' == expr[0] ) { opCode |= kOp_FS_LvLCompl; ++n; }
    else if( '!' == expr[0] ) { opCode |= kOp_FS_NoLCompl; ++n; }
    /* Check the core symbol */
    switch( expr[n] ) {
        /* additive arithmetic */
        case '+': opCode |= kOp_ArithAdd; break;
        case '-': opCode |= kOp_ArithSub; break;
        /* multiplicative arithmetic */
        case '/': opCode |= kOp_ArithDiv; break;
        case '*':
            if( '*' == expr[n+1] ) {
                opCode |= kOp_ArithPow;
                ++n;
            }
            opCode |= kOp_ArithMult;
            break;
        case '%': opCode |= kOp_ArithModul; break;
        /* comparison and bit shifts */
        case '<':
            if('=' == expr[n+1]) { opCode |= kOp_CmpLE;         ++n; break; }
            if('<' == expr[n+1]) { opCode |= kOp_BtwsLShift;    ++n; break; }
            opCode |= kOp_CmpLT;
            break;
        case '>':
            if('=' == expr[n+1]) { opCode |= kOp_CmpGE;         ++n; break; }
            if('>' == expr[n+1]) { opCode |= kOp_BtwsRShift;    ++n; break; }
            opCode |= kOp_CmpGT;
            break;
        /* eq/ne comparison */
        case '=':
            assert('=' == expr[n+1]);
            opCode |= kOp_CmpEq;
            ++n;
            break;
        case '!':
            assert('=' == expr[n+1]);
            opCode |= kOp_CmpNE;
            ++n;
            break;
        /* AND/OR/XOR -- bitwise and logic */
        case '&':
            if('&' == expr[n+1]) {
                opCode |= kOp_LogicAnd;
                ++n;
                break;
            }
            opCode |= kOp_BtwsAnd;
            break;
        case '|':
            if('|' == expr[n+1]) {
                opCode |= kOp_LogicOr;
                ++n;
                break;
            }
            opCode |= kOp_BtwsOr;
            break;
        case '^':   opCode |= kOp_BtwsXor;  break;
        /* nothing matches */
        default:
            /* This should never happen unless this function does not match
             * a lexer... */
            snprintf( ws->errMsg, ws->errMsgSize
                    , "failed to parse operator \"%s\" (yet it passed the"
                      " lexer)", expr );
            return 0x0;
    };
    ++n;
    /* Check right complement */
    if(      '.' == expr[n] ) { opCode |= kOp_FS_LvRCompl; ++n; }
    else if( '!' == expr[n] ) { opCode |= kOp_FS_NoRCompl; ++n; }
    /* We must be done now */
    printf("xxx \"%s\", %#x\n", expr, opCode);  // XXX
    assert('\0' == expr[n]);
    assert(opCode);
    return opCode;
}

const char * mwl_to_str_op( mwl_OpCode_t opCode ) {
    static char buf[32];
    snprintf(buf, sizeof(buf), "%#x", opCode);
    return buf;
}

/*                          * * *   * * *   * * *                           */

struct mwl_ASTNode *
mwl_shallow_copy_node(struct mwl_ASTNode * node) {
    struct mwl_ASTNode * result = malloc(sizeof(struct mwl_ASTNode));
    memcpy(result, node, sizeof(struct mwl_ASTNode));
    return result;
}

mwl_TypeCode_t
mwl_infer_type( mwl_TypeCode_t tcA
              , mwl_OpCode_t opCode
              , mwl_TypeCode_t tcB
              , char * errBuf
              , size_t errBufSize
              ) {
    if( (opCode & (0x1 << 16)) /* if op code implies map/set/string type */
     || (tcA & (0x1 << 10)) || (tcB & (0x1 << 10))  /* or at least one of the operands */
      ) {
        // TODO: a set or string type
        return 0x0;
    } else {
        if( kOp_FUnary & opCode ) {
            // TODO: unary operations
            return 0x0;
        } else {
            if( kOp_FComparison & opCode ) {
                // for comparison operators the result is always logic, but
                // one have to assure that we deal with numeric data types
                // on both of the sides
                if( (mwl_kFIsNumeric & tcA) && (mwl_kFIsNumeric & tcB) )
                    return mwl_kLogic;
                // For strings only "equals to" and "does not equal to" are
                // defined
                if( (mwl_kString == tcA)
                 && (mwl_kString & tcB)
                 && (opCode == kOp_CmpEq || opCode == kOp_CmpNE) )
                    return mwl_kLogic;
                // error of comparsion types
                snprintf( errBuf, errBufSize
                        , "can not compare %s with %s by operator `%s'"
                        , mwl_to_str_type(tcA)
                        , mwl_to_str_type(tcB)
                        , mwl_to_str_op(opCode)
                        );
                return 0x0;
            }  // comparison operation
            if( kOp_FBitwise & opCode ) {
                // for binary bitwise operations (AND, OR, XOR) the result is
                // always an integer; we have to just check that both operands
                // are of integer type
                if( tcA == mwl_kInteger && tcB == mwl_kInteger )
                    return mwl_kInteger;
                snprintf( errBuf, errBufSize
                        , "can not apply `%s' bitwise operation on %s and %s"
                        , mwl_to_str_op(opCode)
                        , mwl_to_str_type(tcA)
                        , mwl_to_str_type(tcB)
                        );
                return 0x0;
            }  // bitwise operation
            if( kOp_FArithmetic & opCode ) {
                // for binary arithmetic operations we convert int to float in
                // mixed case and preserve int for the int-only operation
                if( !(mwl_kFIsNumeric & tcA) ) {
                    snprintf( errBuf, errBufSize
                            , "left argument of `%s' arithmetic operation is not a"
                              " number (it is of type %s)"
                            , mwl_to_str_op(opCode)
                            , mwl_to_str_type(tcA)
                            );
                    return 0x0;
                } else if( !(mwl_kFIsNumeric & tcA) ) {
                    snprintf( errBuf, errBufSize
                            , "right argument of `%s' arithmetic operation is not a"
                              " number (it is of type %s)"
                            , mwl_to_str_op(opCode)
                            , mwl_to_str_type(tcB)
                            );
                    return 0x0;
                }
                if( tcA == mwl_kFloat ) return mwl_kFloat;
                if( tcB == mwl_kFloat ) return mwl_kFloat;
                return mwl_kInteger;
            }  // arithmetic operation
        }  // binary operation
    }  // scalar data type
    return 0x0;
}

int
mwl_init_op_node( struct mwl_ASTNode * dest
                , struct mwl_ASTNode * left
                , mwl_OpCode_t opCode
                , struct mwl_ASTNode * right
                , mwl_Workspace_t * ws
                ) {
    mwl_OpCode_t tp = mwl_infer_type( left->dataType, opCode, right->dataType
                                    , ws->errMsg, ws->errMsgSize );
    if(!tp) {
        printf("xyx\n");  // XXX
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

/*                                                        _____________________
 * _____________________________________________________/ Printing functions */

size_t
mwl_to_str_constval( char * buf
                   , size_t n
                   , const struct mwl_ConstVal * obj
                   ) {
    if( obj->dataType == mwl_kLogic )
        return snprintf(buf, n, "%s", obj->pl.asInteger ? "true" : "false");
    if( obj->dataType == mwl_kInteger )
        return snprintf(buf, n, "%ld", (long int) obj->pl.asInteger );
    if( obj->dataType == mwl_kFloat )
        return snprintf(buf, n, "%f", (double) obj->pl.asFloat );
    if( obj->dataType == mwl_kString )
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

