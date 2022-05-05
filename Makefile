# Makefile generating parser for domain-specific language detector selection
# This will require you to have YACC/LEX installed.
# The resulting sources may be used as is, however (without those dependencies)

YACC=bison
LEX=flex

CC=gcc
CFLAGS=-x c -Wall -g -ggdb -Iinclude -Wfatal-errors -c

CXX=g++
CXXFLAGS=-Wall -g -ggdb -Iinclude -Wfatal-errors -c 

all: mwl

mwl.a: build/mwl-ast-print.o \
	 build/mwl-ast.o \
	 build/mwl-defs.o \
	 build/mwl-errors.o \
	 build/mwl-defs.o \
	 build/mwl-errors.o \
	 build/mwl-func.o \
	 build/mwl-ops.o \
	 build/mwl-types.o \
	 build/mwl-ws.o \
	 build/mwl.tab.o \
	 build/lex.yy.o
	ar rcs $@ $^

mwl: build/main.o \
	 mwl.a
	$(CXX) build/main.o mwl.a -o $@

grammar: mwl.tab.c lex.yy.c

lex.yy.h lex.yy.c: mwl.l mwl.tab.h
	$(LEX) --header-file=lex.yy.h $<

mwl.tab.h mwl.tab.c: mwl.y
	$(YACC) --report=all --report-file=build/grammar-report.txt -d -p mwl_ $<

build/mwl.tab.o: mwl.tab.c lex.yy.h
	$(CC) $(CFLAGS) $< -o $@

build/lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) $< -o $@

build/main.o: main.cc include/mwl-ws.h grammar
	$(CXX) $(CXXFLAGS) $< -o $@


build/%.o: src/%.c
	$(CC) $(CFLAGS) $< -o $@

build/%.o: src/%.cc
	$(CXX) $(CXXFLAGS) $< -o $@

clean:
	rm -fv mwl.tab.h y.tab.h \
		   mwl.tab.c y.tab.c \
		   lex.yy.h lex.yy.c \
		   build/* \
		   mwl.a mwl

.PHONY: all clean grammar
