#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comment_tracker.h"

/* Inizializzazione della lista globale */
comment_entry* global_comments = NULL;

void add_comment(int line, const char* text, comment_position pos) {
    comment_entry* new_node = (comment_entry*)malloc(sizeof(comment_entry));
    if (!new_node) {
        fprintf(stderr, "Errore fatale: Out of memory in add_comment\n");
        return;
    }

    /* Il Lexer potrebbe passare il testo con o senza il '#' iniziale. 
       Lo rimuoviamo per uniformità, così il pretty printer può decidere come ristamparlo. */
    const char* content = (text[0] == '#') ? text + 1 : text;
    
    new_node->text = strdup(content);
    if (!new_node->text) {
        free(new_node);
        return;
    }

    new_node->line_number = line;
    new_node->position = pos;
    new_node->next = NULL;

    /* Inserimento in coda alla lista (preserva l'ordine di apparizione) */
    if (global_comments == NULL) {
        global_comments = new_node;
    } else {
        comment_entry* curr = global_comments;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
}

void free_comments(void) {
    comment_entry* curr = global_comments;
    while (curr) {
        comment_entry* next = curr->next;
        if (curr->text) free(curr->text);
        free(curr);
        curr = next;
    }
    global_comments = NULL;
}