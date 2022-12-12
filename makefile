BIN  = parser
CC   = g++
CFLAGS = -DCPLUSPLUS -g  
TAR = parser.tar

SRCS = parser.y parser.l
HDRS = scantype.h treeNode.h
OBJS = lex.yy.o parser.tab.o
LIBS = -lm 

parser: lex.yy.o parser.tab.o treeUtils.o printtree.o ourGetopt.o symbolTable.o semantic.o yyerror.o emitcode.o codeGen.o
	g++ -DCPLUSPLUS -g lex.yy.o parser.tab.o treeUtils.o printtree.o ourGetopt.o symbolTable.o semantic.o yyerror.o emitcode.o codeGen.o -lm -o c-

parser.tab.h parser.tab.c: parser.y treeUtils.h ourGetopt.h semantic.h yyerror.h codeGen.h
	bison -v -t -d parser.y  

lex.yy.c: parser.l parser.tab.h
	flex parser.l

clean:
	rm -f lex.yy.o parser.tab.o c- lex.yy.c parser.tab.h parser.tab.c parser.tar parser.output treeUtils.o ourGetopt.o semantic.o printtree.o symbolTable.o yyerror.o codeGen.o emitcode.o test.tm parser.tar *~

tar:
	tar -cvf parser.tar parser.y parser.l scantype.h treeNodes.h treeUtils.h treeUtils.c ourGetopt.h ourGetopt.c symbolTable.h symbolTable.cpp semantic.h semantic.cpp printtree.h printtree.cpp yyerror.h yyerror.cpp emitcode.h emitcode.cpp codeGen.h codeGen.cpp makefile 
	ls -l parser.tar