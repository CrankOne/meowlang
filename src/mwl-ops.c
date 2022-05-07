#include "mwl-ops.h"
#include "mwl-ws.h"

#ifndef NO_PLAIN_EVAL
#include "mwl-defs.h"
#include <math.h>
#endif

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
    char buf1[64], buf2[64];
    if( (opCode & (0x1 << 16)) /* if op code implies map/set/string type */
     || (tcA & (mwl_kFIsCollection | mwl_kFIsForeign))
     || (tcB & (mwl_kFIsCollection | mwl_kFIsForeign))  /* or at least one of the operands */
     ) {
        #warning "TODO: type for operations on collections"
        snprintf( errBuf, errBufSize
                , "An operation on collections of types `%s' and `%s' with operator %s"
                  " is not yet implemented (TODO stub)"
                , mwl_to_str_type(buf1, sizeof(buf1), tcA)
                , mwl_to_str_type(buf2, sizeof(buf2), tcB)
                , mwl_to_str_op(opCode)
                );  // TODO
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
                        , mwl_to_str_type(buf1, sizeof(buf1), tcA)
                        , mwl_to_str_type(buf2, sizeof(buf2), tcB)
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
                        , mwl_to_str_type(buf1, sizeof(buf1), tcA)
                        , mwl_to_str_type(buf2, sizeof(buf2), tcB)
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
                            , mwl_to_str_type(buf1, sizeof(buf1), tcA)
                            );
                    return 0x0;
                } else if( !(mwl_kFIsNumeric & tcA) ) {
                    snprintf( errBuf, errBufSize
                            , "right argument of %s arithmetic operation is not a"
                              " number (it is of type `%s')"
                            , mwl_to_str_op(opCode)
                            , mwl_to_str_type(buf1, sizeof(buf1), tcB)
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

#ifndef NO_PLAIN_EVAL
/*                                                 ____________________________
 * ______________________________________________/ Plain Evaluation routines */

static int
_mwl_plainev_op_logic_unary_negate (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    if(b) return 2;
    r->dataType = mwl_kTpLogic;
    if( a->dataType == mwl_kTpLogic || a->dataType == mwl_kTpInteger )
        r->pl.asInteger = a->pl.asInteger ? 0 : 1;
    if( a->dataType == mwl_kTpFloat )
        r->pl.asInteger = a->pl.asFloat ? 0 : 1;
    return 0;
}


#define M_IMPLEM_PLAIN_EV( name, sign )                                     \
static int                                                                  \
_mwl_plainev_op_ ## name (                                                  \
                 const struct mwl_ConstVal * a                              \
               , const struct mwl_ConstVal * b                              \
               , struct mwl_ConstVal * r                                    \
               ) {                                                          \
    assert(a);                                                              \
    assert(b);                                                              \
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpInteger ) {  \
        r->dataType = mwl_kTpInteger;                                       \
        r->pl.asInteger = a->pl.asInteger sign b->pl.asInteger;             \
        return 0;                                                           \
    }                                                                       \
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpInteger ) {    \
        r->dataType = mwl_kTpFloat;                                         \
        r->pl.asFloat = a->pl.asFloat sign b->pl.asInteger;                 \
        return 0;                                                           \
    }                                                                       \
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpFloat ) {    \
        r->dataType = mwl_kTpFloat;                                         \
        r->pl.asFloat = a->pl.asInteger sign b->pl.asFloat;                 \
        return 0;                                                           \
    }                                                                       \
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpFloat ) {      \
        r->dataType = mwl_kTpFloat;                                         \
        r->pl.asFloat = a->pl.asFloat sign b->pl.asFloat;                   \
        return 0;                                                           \
    }                                                                       \
    r->dataType = 0;                                                        \
    return 1;                                                               \
}

M_IMPLEM_PLAIN_EV( plus,   + )
M_IMPLEM_PLAIN_EV( minus,  - )
M_IMPLEM_PLAIN_EV( divide, / )
M_IMPLEM_PLAIN_EV( mult,   * )
M_IMPLEM_PLAIN_EV( and,   && )
M_IMPLEM_PLAIN_EV( or,    || )
#undef M_IMPLEM_PLAIN_EV

static int
_mwl_plainev_op_pow (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpInteger ) {
        r->dataType = mwl_kTpInteger;
        r->pl.asInteger = (mwl_Integer_t) pow( a->pl.asInteger, b->pl.asInteger);
        return 0;
    }
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpInteger ) {
        r->dataType = mwl_kTpFloat;
        r->pl.asFloat = pow(a->pl.asFloat, b->pl.asInteger);
        return 0;
    }
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpFloat ) {
        r->dataType = mwl_kTpFloat;
        r->pl.asFloat = pow(a->pl.asInteger, b->pl.asFloat);
        return 0;
    }
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpFloat ) {
        r->dataType = mwl_kTpFloat;
        r->pl.asFloat = pow(a->pl.asFloat, b->pl.asFloat);
        return 0;
    }
    r->dataType = 0;
    return 1;
}

static int
_mwl_plainev_op_modul (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpInteger ) {
        r->dataType = mwl_kTpInteger;
        r->pl.asInteger = a->pl.asInteger % b->pl.asInteger;
        return 0;
    }
    return 1;
}

static int
_mwl_plainev_op_arith_negate (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    if(b) return 2;
    if( a->dataType == mwl_kTpInteger ) {
        r->dataType = mwl_kTpInteger;
        r->pl.asInteger = - a->pl.asInteger;
        return 0;
    }
    if( a->dataType == mwl_kTpFloat ) {
        r->dataType = mwl_kTpFloat;
        r->pl.asInteger = - a->pl.asFloat;
        return 0;
    }
    return 1;
}

static int
_mwl_plainev_op_arith_unary_plus (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    if(b) return 2;
    if( a->dataType == mwl_kTpInteger ) {
        r->dataType = mwl_kTpInteger;
        r->pl.asInteger = a->pl.asInteger;
        return 0;
    }
    if( a->dataType == mwl_kTpFloat ) {
        r->dataType = mwl_kTpFloat;
        r->pl.asInteger = a->pl.asFloat;
        return 0;
    }
    return 1;
}



static int
_mwl_plainev_op_ne (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    r->dataType = mwl_kTpLogic;
    if( (a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpInteger)
     || (a->dataType == mwl_kTpLogic   && b->dataType == mwl_kTpLogic ) ) {
        r->pl.asInteger = a->pl.asInteger != b->pl.asInteger;
        return 0;
    }
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpInteger ) {
        r->pl.asInteger = a->pl.asFloat != b->pl.asInteger;
        return 0;
    }
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpFloat ) {
        r->pl.asInteger = a->pl.asInteger != b->pl.asFloat;
        return 0;
    }
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpFloat ) {
        r->pl.asInteger = a->pl.asFloat != b->pl.asFloat;
        return 0;
    }
    r->dataType = 0;
    return 1;
}

static int
_mwl_plainev_op_eq (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    int rc = _mwl_plainev_op_ne(a, b, r);
    if(rc) return rc;
    assert(r->dataType == mwl_kTpLogic);
    r->pl.asInteger = r->pl.asInteger ? 0 : 1;
    return 0;
}

static int
_mwl_plainev_op_gt (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    r->dataType = mwl_kTpLogic;
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpInteger ) {
        r->dataType = mwl_kTpLogic;
        r->pl.asInteger = a->pl.asInteger > b->pl.asInteger;
        return 0;
    }
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpInteger ) {
        r->pl.asFloat = a->pl.asFloat > b->pl.asInteger;
        return 0;
    }
    if( a->dataType == mwl_kTpInteger && b->dataType == mwl_kTpFloat ) {
        r->pl.asFloat = a->pl.asInteger > b->pl.asFloat;
        return 0;
    }
    if( a->dataType == mwl_kTpFloat && b->dataType == mwl_kTpFloat ) {
        r->pl.asFloat = a->pl.asFloat > b->pl.asFloat;
        return 0;
    }
    r->dataType = 0;
    return 1;
}

static int
_mwl_plainev_op_ge (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    int rc = _mwl_plainev_op_gt(a, b, r);
    if(rc) return rc;
    assert( mwl_kTpLogic == r->dataType );
    if(r->pl.asInteger) return 0;
    rc = _mwl_plainev_op_eq(a, b, r);
    if(rc) return rc;
    assert( mwl_kTpLogic == r->dataType );
    return 0;
}

static int
_mwl_plainev_op_lt (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    int rc = _mwl_plainev_op_ge(a, b, r);
    if(rc) return rc;
    assert( mwl_kTpLogic == r->dataType );
    r->pl.asInteger = r->pl.asInteger ? 0 : 1;
    return 0;
}

static int
_mwl_plainev_op_le (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    int rc = _mwl_plainev_op_gt(a, b, r);
    if(rc) return rc;
    assert( mwl_kTpLogic == r->dataType );
    r->pl.asInteger = r->pl.asInteger ? 0 : 1;
    return 0;
}



static int
_mwl_plainev_op_btws_unary_negate (
                 const struct mwl_ConstVal * a
               , const struct mwl_ConstVal * b
               , struct mwl_ConstVal * r
               ) {
    if(b) return 2;
    if( a->dataType != mwl_kTpInteger ) return 1;
    r->dataType = mwl_kTpInteger;
    r->pl.asInteger = ~ (a->pl.asInteger);
    return 0;
}

#define M_IMPLEM_PLAIN_EV( name, sign )                                     \
static int                                                                  \
_mwl_plainev_op_btws_ ## name (                                             \
                 const struct mwl_ConstVal * a                              \
               , const struct mwl_ConstVal * b                              \
               , struct mwl_ConstVal * r                                    \
               ) {                                                          \
    if( a->dataType != mwl_kTpInteger || b->dataType != mwl_kTpInteger )    \
        return 1;                                                           \
    r->dataType = mwl_kTpInteger;                                           \
    r->pl.asInteger = (a->pl.asInteger) sign (b->pl.asInteger);             \
    return 0;                                                               \
}

M_IMPLEM_PLAIN_EV( and,    &  )
M_IMPLEM_PLAIN_EV( or,     |  )
M_IMPLEM_PLAIN_EV( xor,    ^  )
M_IMPLEM_PLAIN_EV( lshift, << )
M_IMPLEM_PLAIN_EV( rshift, >> )
#undef M_IMPLEM_PLAIN_EV

mwl_PlainNodeEvaluator
mwl_op_get_plain_eval( mwl_OpCode_t c ) {
    c &= ~(kOp_FS_NoLCompl | kOp_FS_LvLCompl | kOp_FS_NoRCompl | kOp_FS_LvRCompl);

    if( c == kOp_LogicNegate ) return _mwl_plainev_op_logic_unary_negate;
    if( c == kOp_LogicAnd )    return _mwl_plainev_op_and;
    if( c == kOp_LogicOr )     return _mwl_plainev_op_or;

    if( c == kOp_ArithAdd )  return _mwl_plainev_op_plus;
    if( c == kOp_ArithSub )  return _mwl_plainev_op_minus;
    if( c == kOp_ArithDiv )  return _mwl_plainev_op_divide;
    if( c == kOp_ArithMult ) return _mwl_plainev_op_mult;
    if( c == kOp_ArithNegate ) return _mwl_plainev_op_arith_negate;
    if( c == (kOp_ArithAdd | kOp_FUnary) ) return _mwl_plainev_op_arith_unary_plus;
    if( c == kOp_ArithPow )  return _mwl_plainev_op_pow;
    if( c == kOp_ArithModul )  return _mwl_plainev_op_modul;

    if( c == kOp_CmpEq ) return _mwl_plainev_op_eq;
    if( c == kOp_CmpNE ) return _mwl_plainev_op_ne;
    if( c == kOp_CmpGT ) return _mwl_plainev_op_gt;
    if( c == kOp_CmpGE ) return _mwl_plainev_op_ge;
    if( c == kOp_CmpLT ) return _mwl_plainev_op_lt;
    if( c == kOp_CmpLE ) return _mwl_plainev_op_le;

    if( c == kOp_BtwsNegate ) return _mwl_plainev_op_btws_unary_negate;
    if( c == kOp_BtwsAnd )    return _mwl_plainev_op_btws_and;
    if( c == kOp_BtwsOr )     return _mwl_plainev_op_btws_or;
    if( c == kOp_BtwsXor )    return _mwl_plainev_op_btws_xor;
    if( c == kOp_BtwsLShift ) return _mwl_plainev_op_btws_lshift;
    if( c == kOp_BtwsRShift ) return _mwl_plainev_op_btws_rshift;

    return NULL;
}
#endif  /* ! NO_PLAIN_EVAL */

