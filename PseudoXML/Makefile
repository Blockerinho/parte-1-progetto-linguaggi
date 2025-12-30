## Makefile per Progetto LC (Adattato da BNFC)

CC = gcc
# Flag aggiornati per supportare C11 e funzioni POSIX come strdup
CCFLAGS = -g -W -Wall -Wno-unused-parameter -Wno-unused-function -Wno-unneeded-internal-declaration -std=c11 -D_POSIX_C_SOURCE=200809L

FLEX = flex
FLEX_OPTS = -Ppseudo_xm_lgrammatica_

BISON = bison
BISON_OPTS = -t -ppseudo_xm_lgrammatica_

# Lista degli oggetti da compilare
# Nota: Abbiamo rimosso Printer.o e Test.o, e aggiunto i tuoi moduli custom
OBJS = Absyn.o Buffer.o Lexer.o Parser.o PseudoXMLParserSupport.o \
       comment_tracker.o pretty-printer.o main.o

# Nome dell'eseguibile finale
TARGET = progetto_lc

.PHONY : clean distclean demo all

all : $(TARGET)

clean :
	rm -f *.o $(TARGET) pseudoXMLgrammatica.aux pseudoXMLgrammatica.log pseudoXMLgrammatica.pdf pseudoXMLgrammatica.dvi pseudoXMLgrammatica.ps pseudoXMLgrammatica

distclean : clean
	rm -f Absyn.h Absyn.c Bison.h Buffer.h Buffer.c pseudoXMLgrammatica.l Lexer.c pseudoXMLgrammatica.y Parser.h Parser.c Printer.c Printer.h Skeleton.c Skeleton.h Test.c Makefile pseudoXMLgrammatica.tex

# Linker finale
$(TARGET) : ${OBJS}
	@echo "Linking $(TARGET)..."
	${CC} ${OBJS} -o $(TARGET)

# --- Regole di Compilazione File BNFC ---

Absyn.o : Absyn.c Absyn.h
	${CC} ${CCFLAGS} -c Absyn.c

Buffer.o : Buffer.c Buffer.h
	${CC} ${CCFLAGS} -c Buffer.c

# Generazione Lexer
Lexer.c : pseudoXMLgrammatica.l
	${FLEX} ${FLEX_OPTS} -oLexer.c pseudoXMLgrammatica.l

# Compilazione Lexer (Dipende ora da comment_tracker.h)
Lexer.o : Lexer.c Bison.h comment_tracker.h
	${CC} ${CCFLAGS} -Wno-sign-conversion -c Lexer.c

# Generazione Parser
Parser.c Bison.h : pseudoXMLgrammatica.y
	${BISON} ${BISON_OPTS} pseudoXMLgrammatica.y -o Parser.c

# Compilazione Parser
Parser.o : Parser.c Absyn.h Bison.h PseudoXMLParserSupport.h
	${CC} ${CCFLAGS} -c Parser.c

# --- Regole di Compilazione File Custom ---

# Supporto strutture dati e logica SDT
PseudoXMLParserSupport.o : PseudoXMLParserSupport.c PseudoXMLParserSupport.h Absyn.h
	${CC} ${CCFLAGS} -c PseudoXMLParserSupport.c

# Gestione commenti
comment_tracker.o : comment_tracker.c comment_tracker.h
	${CC} ${CCFLAGS} -c comment_tracker.c

# Pretty Printer personalizzato
pretty-printer.o : pretty-printer.c PseudoXMLParserSupport.h comment_tracker.h
	${CC} ${CCFLAGS} -c pretty-printer.c

# Main entry point
main.o : main.c Parser.h PseudoXMLParserSupport.h
	${CC} ${CCFLAGS} -c main.c

# --- Target Demo ---
demo : $(TARGET)
	@echo "--- Esecuzione Demo ---"
	./$(TARGET) test.cnf