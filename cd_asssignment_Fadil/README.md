yacc -d expl.y
lex expl.l
gcc -o tacgen y.tab.c lex.yy.c tac.c optimizer.c
./tacgen sample_input.txt