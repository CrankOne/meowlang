#ifndef H_MWL_OPERATIONS_H
#define H_MWL_OPERATIONS_H

#include "mwl-types.h"

struct mwl_Workspace;

#define M_for_all_opcodes(m, ...) \
    /* Features flags */ \
    m( FUnary,       ( 0x1 <<  8), __VA_ARGS__ ) \
    m( FArithmetic,  ( 0x2 <<  8), __VA_ARGS__ ) \
    m( FBitwise,     ( 0x4 <<  8), __VA_ARGS__ ) \
    m( FComparison,  ( 0x8 <<  8), __VA_ARGS__ ) \
    /* Options controlling complement consideration */      \
    m( FS_NoLCompl,  ( 0x3 << 16), __VA_ARGS__ ) /* require left complement */   \
    m( FS_LvLCompl,  ( 0x5 << 16), __VA_ARGS__ ) /* leave left complement */     \
    m( FS_NoRCompl,  ( 0x9 << 16), __VA_ARGS__ ) /* require right complement */  \
    m( FS_LvRCompl,  (0x11 << 16), __VA_ARGS__ ) /* leave right complement */    \
    /* Logic operator codes */ \
    m( LogicNegate, 1 | kOp_FUnary, __VA_ARGS__ ) \
    m( LogicAnd,    1, __VA_ARGS__ ) \
    m( LogicOr,     2, __VA_ARGS__ ) \
    /* Arithmetic operator codes */ \
    m( ArithNegate, 1 | kOp_FArithmetic | kOp_FUnary, __VA_ARGS__ ) \
    m( ArithAdd,    1 | kOp_FArithmetic, __VA_ARGS__ ) \
    m( ArithSub,    2 | kOp_FArithmetic, __VA_ARGS__ ) \
    m( ArithMult,   3 | kOp_FArithmetic, __VA_ARGS__ ) \
    m( ArithDiv,    4 | kOp_FArithmetic, __VA_ARGS__ ) \
    m( ArithPow,    5 | kOp_FArithmetic, __VA_ARGS__ ) \
    m( ArithModul,  6 | kOp_FArithmetic, __VA_ARGS__ ) \
    /* Comparison operator codes */ \
    m( CmpEq,       1 | kOp_FComparison, __VA_ARGS__ ) \
    m( CmpNE,       2 | kOp_FComparison, __VA_ARGS__ ) \
    m( CmpGT,       3 | kOp_FComparison, __VA_ARGS__ ) \
    m( CmpGE,       4 | kOp_FComparison, __VA_ARGS__ ) \
    m( CmpLT,       5 | kOp_FComparison, __VA_ARGS__ ) \
    m( CmpLE,       6 | kOp_FComparison, __VA_ARGS__ ) \
    /* Bitwise arithmetic codes */ \
    m( BtwsNegate,  1 | kOp_FBitwise | kOp_FUnary, __VA_ARGS__ ) \
    m( BtwsAnd,     1 | kOp_FBitwise, __VA_ARGS__ ) \
    m( BtwsOr,      2 | kOp_FBitwise, __VA_ARGS__ ) \
    m( BtwsXor,     3 | kOp_FBitwise, __VA_ARGS__ ) \
    m( BtwsLShift,  5 | kOp_FBitwise, __VA_ARGS__ ) \
    m( BtwsRShift,  6 | kOp_FBitwise, __VA_ARGS__ ) \
    /* ... */

#define M_DECLARE_OP_CODE( name, code, ... ) \
    extern const mwl_OpCode_t kOp_ ## name;
M_for_all_opcodes(M_DECLARE_OP_CODE)
#undef M_DECLARE_OP_CODE

/** Converts operator to opcode */
mwl_OpCode_t mwl_to_opcode( const char * expr, struct mwl_Workspace * ws );

/** Returns string representing an operator */
const char * mwl_to_str_op(mwl_OpCode_t);

/** Deduces data type based on operation and operand types */
mwl_TypeCode_t
mwl_infer_type( mwl_TypeCode_t tcA
              , mwl_OpCode_t opCode
              , mwl_TypeCode_t tcB
              , char * errBuf
              , size_t errBufSize
              );

#endif  /* H_MWL_OPERATIONS_H */

