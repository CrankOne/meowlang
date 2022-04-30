# Makefile generating parser for domain-specific language detector selection
# This will require you to have YACC/LEX installed.
# The resulting sources may be used as is, however (without those dependencies)

YACC=bison
LEX=flex

grammar: meowlang.tab.c lex.yy.c

a.out: meowlang.tab.c lex.yy.c main.c
	gcc -g -ggdb -Wall -I../../include $^

lex.yy.c: meowlang.l meowlang.tab.h
	$(LEX) --header-file=lex.yy.h $<

meowlang.tab.h meowlang.tab.c: meowlang.y
	$(YACC) -d -p meowlang_ $<

clean:
	rm -fv meowlang.tab.h y.tab.h \
		   meowlang.tab.c y.tab.c \
		   lex.yy.h lex.yy.c a.out

.PHONY: all clean grammar
