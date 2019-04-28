CC = gcc

CFLAGS = -Wall -ggdb

# Use when tree.c and pretty.c implemented
# main: parser.tab.o lex.yy.o main.o tree.h tree.o pretty.h pretty.o
# 	$(CC) tree.o lex.yy.o parser.tab.o pretty.o main.o -o golite -lfl

main: parser.tab.o lex.yy.o main.o comm.h comm.o tree.h tree.o pretty.o weed.h weed.o symbol.h symbol.o type.h type.o code.h code.o
	$(CC) lex.yy.o parser.tab.o comm.o tree.o pretty.o weed.o symbol.o type.o code.o main.o -o golite -lfl

parser.tab.c parser.tab.h: golite.y tree.h
	bison --yacc --defines=parser.tab.c --output=parser.tab.h golite.y

lex.yy.c: golite.l parser.tab.h
	flex golite.l

clean:
	rm parser.tab.* lex.yy.*; rm *.o; rm golite;

debug: golite.y
	rm golite.output; bison --verbose golite.y
