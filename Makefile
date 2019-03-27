myshell: myshell.c

	flex shell.l
	
	cc -c myshell.c lex.yy.c

	cc -o myshell myshell.o lex.yy.o -lfl
clean: 
	rm -f myshell myshell.o lex.yy.o lex.yy.c