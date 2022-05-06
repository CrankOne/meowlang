#include "mwl-ops.h"
#include "mwl-ws.h"

#include <assert.h>

#define M_DEFINE_OP_CODE( name, code, ... ) \
    const mwl_OpCode_t kOp_ ## name = code;
M_for_all_opcodes(M_DEFINE_OP_CODE)
#undef M_DEFINE_OP_CODE

mwl_OpCode_t
mwl_to_opcode( const char * expr
             , struct mwl_Workspace * ws
             ) {  // TODO: check this
    assert(expr);
    size_t n = 0;
    mwl_OpCode_t opCode = 0x0;
    /* Check left complement */
    if(      '.' == expr[0] ) { opCode |= kOp_FS_LvLCompl; ++n; }
    else if( '!' == expr[0] && expr[1] != '=' ) { opCode |= kOp_FS_NoLCompl; ++n; }
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
        case '!':
            assert('=' == expr[n+1]);
            opCode |= kOp_CmpNE;
            ++n;
            break;
        case '=':
            assert('=' == expr[n+1]);
            opCode |= kOp_CmpEq;
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
    assert('\0' == expr[n]);
    assert(opCode);
    return opCode;
}

static int
mwl_to_str_op_princ(char * dst, mwl_OpCode_t c) {
    c &= ~(kOp_FS_NoLCompl | kOp_FS_LvLCompl | kOp_FS_NoRCompl | kOp_FS_LvRCompl);

    if(c == kOp_LogicNegate)    {dst[0] = '!'; return 1;}
    if(c == kOp_BtwsNegate)     {dst[0] = '~'; return 1;}
    if(c == kOp_ArithNegate)    {dst[0] = '-'; return 1;}
    if(c == (kOp_ArithAdd | kOp_FUnary)) {dst[0] = '+'; return 1;}

    if(c == kOp_ArithAdd)       {dst[0] = '+'; return 1;}
    if(c == kOp_ArithSub)       {dst[0] = '-'; return 1;}
    if(c == kOp_ArithMult)      {dst[0] = '*'; return 1;}
    if(c == kOp_ArithDiv)       {dst[0] = '/'; return 1;}
    if(c == kOp_ArithModul)     {dst[0] = '%'; return 1;}
    if(c == kOp_CmpLT)          {dst[0] = '<'; return 1;}
    if(c == kOp_CmpGT)          {dst[0] = '>'; return 1;}

    if(c == kOp_BtwsAnd)        {dst[0] = '&'; return 1;}
    if(c == kOp_BtwsOr)         {dst[0] = '|'; return 1;}
    if(c == kOp_BtwsXor)        {dst[0] = '^'; return 1;}

    if(c == kOp_LogicAnd)       {dst[0] = '&'; dst[1] = '&'; return 2;}
    if(c == kOp_LogicOr)        {dst[0] = '|'; dst[1] = '|'; return 2;}
    if(c == kOp_ArithPow)       {dst[0] = '*'; dst[1] = '*'; return 2;}

    if(c == kOp_CmpEq)          {dst[0] = '='; dst[1] = '='; return 2;}
    if(c == kOp_CmpNE)          {dst[0] = '!'; dst[1] = '='; return 2;}
    if(c == kOp_CmpLE)          {dst[0] = '<'; dst[1] = '='; return 2;}
    if(c == kOp_CmpGE)          {dst[0] = '>'; dst[1] = '='; return 2;}

    if(c == kOp_BtwsLShift)     {dst[0] = '<'; dst[1] = '<'; return 2;}
    if(c == kOp_BtwsRShift)     {dst[0] = '>'; dst[1] = '>'; return 2;}

    return 0;
}

const char *
mwl_to_str_op( mwl_OpCode_t opCode ) {
    static char buf[32];
    char sgn[5] = "\0\0\0\0\0";

    if( kOp_FUnary & opCode ) {
        mwl_to_str_op_princ(sgn, opCode);
    } else {
        int n = 0;
        if( (kOp_FS_NoLCompl | kOp_FS_LvLCompl) & opCode )
            sgn[n++] = (kOp_FS_NoLCompl & opCode) ? '!': '.';
        n += mwl_to_str_op_princ(sgn + n, opCode);
        if( (kOp_FS_NoRCompl | kOp_FS_LvRCompl) & opCode )
            sgn[n++] = (kOp_FS_NoRCompl & opCode) ? '!': '.';
    }

    snprintf(buf, sizeof(buf), "`%s' (%#x)", sgn, opCode);
    return buf;
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
            if( tcB ) return 0x0;  /* no second operand allowed for unary ops */
            /* for logic negate, any type is allowed yielding logic type */
            if( opCode == kOp_LogicNegate ) return mwl_kTpLogic;
            /* for bitwise negate we only accept integer type */
            if( opCode == kOp_BtwsNegate ) {
                if( tcA == mwl_kTpInteger ) return mwl_kTpInteger;
                return 0x0;
            }
            /* for unary arithmetic operators, consider only numerical types */
            if( opCode == kOp_ArithNegate
             || opCode == (kOp_ArithAdd | kOp_FUnary) ) {
                if( tcA == mwl_kTpInteger || tcA == mwl_kTpFloat ) return tcA;
                return 0x0;
            }
            return 0x0;
        } else {
            if( kOp_FComparison & opCode ) {
                // for comparison operators the result is always logic, but
                // one have to assure that we deal with numeric data types
                // on both of the sides
                if( (mwl_kFIsNumeric & tcA) && (mwl_kFIsNumeric & tcB) )
                    return mwl_kTpLogic;
                // For strings only "equals to" and "does not equal to" are
                // defined
                if( (mwl_kTpString == tcA)
                 && (mwl_kTpString & tcB)
                 && (opCode == kOp_CmpEq || opCode == kOp_CmpNE) )
                    return mwl_kTpLogic;
                // error of comparsion types
                snprintf( errBuf, errBufSize
                        , "can not compare `%s' with `%s' types with operator %s"
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
                if( tcA == mwl_kTpInteger && tcB == mwl_kTpInteger )
                    return mwl_kTpInteger;
                snprintf( errBuf, errBufSize
                        , "can not apply %s bitwise operation on `%s' and `%s'"
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
                            , "left argument of %s arithmetic operation is not a"
                              " number (it is of type `%s')"
                            , mwl_to_str_op(opCode)
                            , mwl_to_str_type(tcA)
                            );
                    return 0x0;
                } else if( !(mwl_kFIsNumeric & tcA) ) {
                    snprintf( errBuf, errBufSize
                            , "right argument of %s arithmetic operation is not a"
                              " number (it is of type `%s')"
                            , mwl_to_str_op(opCode)
                            , mwl_to_str_type(tcB)
                            );
                    return 0x0;
                }
                if( tcA == mwl_kTpFloat ) return mwl_kTpFloat;
                if( tcB == mwl_kTpFloat ) return mwl_kTpFloat;
                return mwl_kTpInteger;
            }  // arithmetic operation
        }  // binary operation
    }  // scalar data type
    return 0x0;
}

