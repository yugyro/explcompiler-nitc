CC    = gcc
CFLAGS = -Wall -O2
LEX   = flex
YACC  = bison -y

expl2tac: y.tab.c lex.yy.c ast.c tacgen.c opt.c main.c tac.h
	$(CC) $(CFLAGS) -o $@ y.tab.c lex.yy.c ast.c tacgen.c opt.c main.c

y.tab.c y.tab.h: expl.y tac.h
	$(YACC) -d expl.y

lex.yy.c: expl.l y.tab.h
	$(LEX) expl.l

clean:
	rm -f expl2tac y.tab.c y.tab.h lex.yy.c
