#ifndef COMMENT_TRACKER_H
#define COMMENT_TRACKER_H

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
    struct comment_entry* next;
} comment_entry;

/* Variabile globale accessibile ovunque (Lexer, Pretty Printer, Main) */
extern comment_entry* global_comments;

/* Funzione chiamata dal Lexer per registrare un nuovo commento */
void add_comment(int line, const char* text, comment_position pos);

/* Funzione per pulire la memoria a fine esecuzione */
void free_comments(void);

#endif