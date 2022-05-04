#ifndef H_MEOWLANG_AST_H
#define H_MEOWLANG_AST_H

#include <stdint.h>
#include <stdio.h>

#define MWL_ERROR_INCOMPAT_TYPE     -100   /* incompatible types */

/** Integer type used to identify operations */
typedef uint32_t mwl_OpCode_t;
/** Integer type used to identify node types */
typedef uint32_t mwl_TypeCode_t;

/** C-type of "integer" */
typedef int32_t mwl_Integer_t;
#define MWL_INTEGER_MAX INT32_MAX
#define MWL_INTEGER_MIN INT32_MIN
/** C-type of "float" */
typedef float mwl_Float_t;

/** Temporary workspace structure
 *
 * This structure contains the entire parsing and lexical analysis context of
 * the expression.
 * */
typedef struct mwl_Workspace {
    /** Error message buffer */
    char * errMsg;
    unsigned int errMsgSize;
    /** Problematic fragment: first line, column, last line, column */
    uint32_t errPos[2][2];
    /* Expression root node's */
    struct mwl_ASTNode * root;
    /* Parser debug messages stream (may be nullptr) */
    FILE * dbgStream;
} mwl_Workspace_t;


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

mwl_OpCode_t mwl_to_opcode( const char * expr, mwl_Workspace_t * ws );

enum mwl_NodeType {
    mwl_kConstValue,    /* Constant, literal or any other immediately evaluated */
    mwl_kOperation,     /* Operation (binary, arithmetic, logic, comparison) */
    mwl_kFunction,      /* External function call */
    mwl_kParameter,     /* External parameter resolved at a runtime */
    mwl_kVariable,      /* Variable set before the execution */
};

/* Type code features: */
extern const mwl_TypeCode_t mwl_kFIsNumeric;

extern const mwl_TypeCode_t mwl_kLogic;
extern const mwl_TypeCode_t mwl_kInteger;
extern const mwl_TypeCode_t mwl_kFloat;
extern const mwl_TypeCode_t mwl_kString;
/* ... other types? map, set, etc */
const char * mwl_type_to_str( mwl_TypeCode_t );

/*                          * * *   * * *   * * *                           */

/* Forward definitions */
struct mwl_ASTNode;
struct mwl_Set;
struct mwl_Map;

struct mwl_FuncDef {
    /* ... TODO */
};

struct mwl_ConstVal {
    mwl_TypeCode_t dataType;
    union {
        mwl_Integer_t asInteger;
        mwl_Float_t asFloat;
        char * asString;
    } pl;
};

/** Represents unary or binary operation
 * NOTE: `b` can be null for unary operations */
struct mwl_Op {
    mwl_OpCode_t code;
    struct mwl_ASTNode * a
                     , * b;
};

struct mwl_Func {
    /** Function definition */
    const struct mwl_FuncDef * funcdef;
    /* ... args list structures */
};

struct mwl_Parameter {
    // ...
};

struct mwl_Variable {
    // ...
};

/** Common AST node structure */
struct mwl_ASTNode {
    enum mwl_NodeType nodeType;
    mwl_TypeCode_t dataType;
    union {
        struct mwl_ConstVal   asConstVal;   /* for kConstValue */
        struct mwl_Op         asOp;         /* for kOperation */
        struct mwl_Func       asFunction;   /* for kFunction */
        struct mwl_Parameter  asParameter;  /* for kParameter */
        struct mwl_Variable   asVariable;   /* for kVariable */
    } pl;
};

/** Places copies of the nodes, infers type, etc */
int mwl_init_op_node( struct mwl_ASTNode * dest
                    , struct mwl_ASTNode * left
                    , mwl_OpCode_t
                    , struct mwl_ASTNode * right
                    , mwl_Workspace_t *
                    );
/** Makes shallow copy of the given AST node */
struct mwl_ASTNode * mwl_shallow_copy_node(struct mwl_ASTNode *);
/** For given types and operation types, infers result data type */
mwl_TypeCode_t mwl_infer_type( mwl_TypeCode_t
                             , mwl_OpCode_t
                             , mwl_TypeCode_t
                             , char * errbuf
                             , size_t errbufLen
                             );

/** Dumps AST tree in console */
void mwl_dump_AST( FILE * stream
                 , const struct mwl_ASTNode * node
                 , int indent
                 );


/*                                                               ______________
 * ____________________________________________________________/ Definitions */

struct mwl_Definition {
    enum mwl_NodeType type;
    union Data {
        struct mwl_ConstVal  asConstVal;
        struct mwl_FuncDef   asFunction;
        struct mwl_Parameter asParameter;
        struct mwl_Variable  asVariable;
    } pl;
};

struct mwl_Definitions {
    void * cppPtr;
};

/** Resolves symbol by identifier name */
const struct mwl_Definition * mwl_definitions_find( const struct mwl_Definition *
                                                  , const char * );

#endif  /* H_MEOWLANG_AST_H */
