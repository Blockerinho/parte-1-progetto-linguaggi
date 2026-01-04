CC = gcc -g
CCFLAGS = -std=c11 -W -Wall -Wsign-conversion -Wno-unused-parameter -Wno-unused-function -Wno-unneeded-internal-declaration ${CC_OPTS}

FLEX = flex
FLEX_OPTS = -Ppseudo_xm_lgrammatica_

BISON = bison
BISON_OPTS = -t -ppseudo_xm_lgrammatica_

OBJS = Absyn.o Buffer.o Lexer.o ParserSupport.o Parser.o ReferenceSolver.o CommentTracker.o Printer.o

all : TestProgram

TestProgram : ${OBJS} TestProgram.o
	@echo "Linking TestProgram..."
	${CC} ${OBJS} TestProgram.o -o TestProgram

Absyn.o : Absyn.c Absyn.h
	${CC} ${CCFLAGS} -c Absyn.c

Buffer.o : Buffer.c Buffer.h
	${CC} ${CCFLAGS} -c Buffer.c

CommentTracker.o : CommentTracker.c CommentTracker.h
	${CC} ${CCFLAGS} -c CommentTracker.c

Printer.o : Printer.c Printer.h Absyn.h ParserSupport.h
	${CC} ${CCFLAGS} -c Printer.c

Lexer.c : Lexer.l
	${FLEX} ${FLEX_OPTS} -oLexer.c Lexer.l

Parser.c Bison.h : Parser.y
	${BISON} ${BISON_OPTS} Parser.y -o Parser.c
# ${BISON} ${BISON_OPTS} --defines=Bison.h Parser.y -o Parser.c

Lexer.o : CCFLAGS+=-Wno-sign-conversion

Lexer.o : Lexer.c Bison.h CommentTracker.h
	${CC} ${CCFLAGS} -c Lexer.c

Parser.o : Parser.c Absyn.h Bison.h ParserSupport.h
	${CC} ${CCFLAGS} -c Parser.c

ReferenceSolver.o : ReferenceSolver.c ParserSupport.h
	${CC} ${CCFLAGS} -c ReferenceSolver.c

ParserSupport.o : ParserSupport.c ParserSupport.h Absyn.h
	${CC} ${CCFLAGS} -c ParserSupport.c

TestProgram.o : TestProgram.c Parser.h Absyn.h Printer.h CommentTracker.h
	${CC} ${CCFLAGS} -c TestProgram.c

zip: Absyn.c Absyn.h Bison.h Buffer.c Buffer.h Lexer.l Makefile Parser.h ParserSupport.c ParserSupport.h Parser.y PseudoXML.cf ReferenceSolver.c TestProgram.c CommentTracker.c CommentTracker.h Printer.c Printer.h tests
	zip ProgettoLC\ parte1\ Gruppo\ 25-3.zip Absyn.c Absyn.h Bison.h Buffer.c Buffer.h Lexer.l Makefile Parser.h ParserSupport.c ParserSupport.h Parser.y PseudoXML.cf ReferenceSolver.c TestProgram.c CommentTracker.c CommentTracker.h Printer.c Printer.h tests/*.pseudoxml

demo: TestProgram
	-./TestProgram tests/test01.pseudoxml
	-./TestProgram tests/test02.pseudoxml
	-./TestProgram tests/test03.pseudoxml
	-./TestProgram tests/test04.pseudoxml
	-./TestProgram tests/test05.pseudoxml
	-./TestProgram tests/test06.pseudoxml
	-./TestProgram tests/test07.pseudoxml
	-./TestProgram tests/test08.pseudoxml
	-./TestProgram tests/test09.pseudoxml
