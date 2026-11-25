yacc -d parser.y
lex lexer.l
gcc -g lex.yy.c y.tab.c exptree.c symbol.c codegen.c typetable.c -o exptree
