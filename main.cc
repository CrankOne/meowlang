#include <cmath>

#include "mwl.tab.h"
#include "mwl-ast-print.h"

#include <map>

static int
_print_node( struct mwl_ASTNode * node
           , int depth
           , void * data
           ) {
    char bf[128], bf1[64];
    if(mwl_kConstValue == node->nodeType) {
        mwl_to_str_constval(bf1, sizeof(bf1), &(node->pl.asConstVal));
        snprintf( bf, sizeof(bf), "%s %p"
                , bf1
                , node );
    } else if(mwl_kOperation == node->nodeType) {
        char tpBf[64];
        snprintf( bf, sizeof(bf), "%s %s %p"
                , mwl_to_str_op(node->pl.asOp.code)
                , mwl_to_str_type(tpBf, sizeof(tpBf), node->dataType)
                , node );
    } else {
        snprintf( bf, sizeof(bf), "%p", node );
    }
    puts(bf);
    return 0;
}

struct Event {
    std::map<int, int> foo;
    // ...
};

struct HitA {
    int n;
};

typedef std::map<int, HitA> HitsA;

int
main(int argc, char * argv[]) {
    mwl::ForeignTypes foreignTypes;
    mwl_Definitions defs(foreignTypes);

    mwl_Definitions & mathDefs = *defs.define_namespace("m")->second.pl.asNamespacePtr;
    mathDefs.define_constval("pi", M_PI);
    // ... other constants
    // ... functions: cos, sin, log, exp, etc.

    # if 0
    // Makign event to be a "foreign type" is an interesting case to make
    // MWL to process the bunch of events. It is not the case with NA64 where
    // we rather interested in analyzing subsets _within_ an event.
    // Define the "event" foreign type
    auto evIt = defs.define_foreign_type<Event>("event");
    // Define some fields within an event
    auto hitT1It = evIt->second.pl.asNamespacePtr->asForeignTypeTable
    #else
    auto hitsAIt = defs.define_foreign_type<HitsA>("hitsA"); {
        assert( hitsAIt->second.type == mwl_Definition::mwl_kDefForeignType );
        // define a table of operations
        mwl_ForeignTypeTable & table = *hitsAIt->second.pl.asForeignTypeTable;
        //table.
    }
    //hitsAIt->second.pl.asForeignTypeTable->selector_context
    //    = _hits_a_ctx;
    #endif

    mwl_ASTNode * ast = mwl_mk_AST( argv[1]
                                  , &defs  /* definitions */
                                  , stdout  /* debug stream */
                                  );
    if( ast ) {
        fputs("AST tree dump:\n", stdout);
        mwl_dump_AST(stdout, ast, 1);

        fputs("\nTopological sort result:\n", stdout);
        mwl_AST_for_all_tsorted( ast, _print_node, NULL );

        fputs("\nEvaluation result:\n", stdout);
        struct mwl_ConstVal evalResult;
        int rc = mwl_AST_eval(ast, &evalResult);
        if( 0 == rc ) {
            char bf[64];
            mwl_to_str_constval(bf, sizeof(bf), &evalResult);
            fputs(" => ", stdout);
            fputs(bf, stdout);
            fputc('\n', stdout);
        }
    }
    return 0;
}

#if 0
struct ExternDict {
    int foo, bar;
};

int
extern_getter( const char * tok
                   , void * resolverRef
                   , na64dp_dsul_CodeVal_t * ptr ) {
    assert(resolverRef);
    struct ExternDict * dict = (struct ExternDict*) resolverRef;
    if( !strcmp("foo", tok) ) {
        *ptr = dict->foo;
        return 1;
    } else if(!strcmp("bar", tok)) {
        *ptr = dict->bar;
        return 1;
    }
    return 0;
}
na64dp_dsul_CodeVal_t static_getter_one(na64dp_dsul_PrincipalValue_t did) {
    return ((uintptr_t) did) & 0x1;
}
na64dp_dsul_CodeVal_t static_getter_two(na64dp_dsul_PrincipalValue_t did) {
    return ((uintptr_t) did) & 0x2;
}
na64dp_dsul_CodeVal_t dyn_getter_three(na64dp_dsul_PrincipalValue_t did) {
    return ((uintptr_t) did) & 0x4;
}


static struct na64dpsu_SymDefinition gGetters[] = {
    { "+", { .resolve_name = extern_getter }, "Common external resolver" },
    { "$first", { static_getter_one }, "Some static getter one" },
    { "$second", { static_getter_two }, "Some static getter two" },
    { "~third", { dyn_getter_three }, "Some dynamic getter" },
    { NULL }  /* sentinel */
};

static const struct TestCase {
    na64dp_dsul_PrincipalValue_t pv;
    const char * expr;
    int expectedResult;
} tests[] = {
    /*  0 */ { (void*) ((uintptr_t) 0x0), "first == 0 && second==0 && third==  0", 1 },
    /*  1 */ { (void*) ((uintptr_t) 0x0), "first == 1 && second==0 && third==  0", 0 },
    /*  2 */ { (void*) ((uintptr_t) 0x1), "first==1 && second==0 && third==0", 1 },
    /*  3 */ { (void*) ((uintptr_t) 0x1), "first==1 && second==1 && third==0", 0 },
    /*  4 */ { (void*) ((uintptr_t) 0x1), "first==0 || foo==42", 1 },
    /*  5 */ { (void*) ((uintptr_t) 0x3), "first==0 || foo==15", 0 },
    /*  6 */ { (void*) ((uintptr_t) 0x7), "first==0 || foo==15 || third==4", 1 },
    /*  7 */ { (void*) ((uintptr_t) 0x3), "first==1 && second == 2", 1 },
    /*  8 */ { (void*) ((uintptr_t) 0x7), "foo==41 || bar==10 && third==4", 0 },
    /*  9 */ { (void*) ((uintptr_t) 0x7), "(foo==41 || bar==10) && third==4", 0 },
    /* 10 */ { (void*) ((uintptr_t) 0x6), "(! foo == 10) ^^ (! bar == 15)", 1 },
    /* 11 */ { (void*) ((uintptr_t) 0x6), "(  foo == 10) ^^ (! bar == 15)", 0 },
    /* 12 */ { (void*) ((uintptr_t) 0x7), "foo == 42 && bar==15", 1 },
    /* 13 */ { (void*) ((uintptr_t) 0x1), "0 == third", 1 }
};

static int
run_test( const struct TestCase * tc, FILE * f ) {
    char wsBuffer[2048], errBf[128];
    struct ExternDict ed = {42, 15};
    int rc = na64dp_dsul_compile_detector_selection( (char*) tc->expr
                                                   , gGetters
                                                   , &ed
                                                   , wsBuffer, sizeof(wsBuffer)
                                                   , errBf, sizeof(errBf)
                                                   , f );
    if( rc ) {
        fprintf(stderr, "%s\n", errBf);
        return 1;
    }

    na64dp_dsul_update_extern_symbols( wsBuffer, &ed );
    na64dp_dsul_update_static_symbols( wsBuffer, tc->pv );

    if( f ) {
        na64dp_dsul_dump( wsBuffer, f );
    }

    rc = na64dp_dsul_eval( wsBuffer );
    if( f ) {
        printf( "Result: %d, expected %d\n",  rc, tc->expectedResult );
    }
    return rc == tc->expectedResult ? 0 : 2;
}

int
main(int argc, char * argv[]) {
    if( 2 == argc ) {
        run_test( tests + atoi(argv[1]), stdout );
        return EXIT_SUCCESS;
    }

    for( int i = 0; i < sizeof(tests)/sizeof(struct TestCase); ++i ) {
        int rc = run_test( tests + i, NULL );
        if( 1 == rc ) {
            fprintf( stderr, "Failed to compile #%d\n", i );
        } else if (2 == rc) {
            fprintf( stderr, "Wrong result for #%d\n", i );
        }
    }

    return EXIT_SUCCESS;
}
#endif
