#ifndef COMMENT_TRACKER_H
#define COMMENT_TRACKER_H

#include <stdio.h> 

/* Enum usata nel Lexer per distinguere commenti a inizio riga o dopo codice */
typedef enum { 
    COMMENT_STANDALONE, 
    COMMENT_INLINE 
} comment_position;

/* Struttura lista concatenata per memorizzare i commenti */
typedef struct comment_entry {
    int line_number;
    char* text;
    comment_position position;
    int already_printed; //fondamentale per non stampare 2 volte stesso commento
    struct comment_entry* next;
} comment_entry;

/* Variabile globale accessibile ovunque (Lexer, Pretty Printer, Main) */
extern comment_entry* glob_commentListHead;

/* Funzione chiamata dal Lexer per registrare un nuovo commento */
void store_comment(int line, const char* text);

/* Funzione per pulire la memoria a fine esecuzione */
void free_comments(void);

// per il pretty-printer: trova il commento nella relativa riga
comment_entry* find_comment_at_line(int line); 

#endif
