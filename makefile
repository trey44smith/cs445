BIN = parser
CC = g++
OUT = c-

SRCS = $(BIN).y  $(BIN).l
HDRS = scanType.h treeNodes.h treeUtil.h printTree.h symbolTable.h ourgetopt.h semantic.h yyerror.h
OBJS = lex.yy.o $(BIN).tab.o

$(BIN) : $(OBJS)
	$(CC) -g $(OBJS) treeUtil.cpp printTree.cpp ourgetopt.cpp semantic.cpp symbolTable.cpp yyerror.cpp -o $(OUT)

lex.yy.c : $(BIN).l $(BIN).tab.h $(HDR)
	flex $(BIN).l

$(BIN).tab.h $(BIN).tab.c : $(BIN).y
	bison -v -t -d $(BIN).y

clean :
	rm -f *~ $(OBJS) $(BIN) lex.yy.c $(BIN).tab.h $(BIN).tab.c $(BIN).output

tar : $(HDR) $(SRCS) makefile
	gtar -cvf $(BIN).tar $(HDRS) $(SRCS) $(DOCS) makefile treeUtil.cpp printTree.cpp symbolTable.cpp semantic.cpp ourgetopt.cpp yyerror.cpp
