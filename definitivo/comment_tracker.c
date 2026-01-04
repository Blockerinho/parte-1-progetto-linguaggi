#define _POSIX_C_SOURCE 200809L

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "comment_tracker.h"

/* Inizializzazione della lista globale */
comment_entry *glob_commentListHead = NULL;
comment_entry *glob_commentListTail = NULL;

void store_comment(int line, const char* text) {
    comment_entry* new_node = (comment_entry*)malloc(sizeof(comment_entry));
    if (!new_node) {
        fprintf(stderr, "Fatal Error: Out of memory in store_comment\n");
        return;
    }

    // posizione del commento
    new_node->line_number = line;
    new_node->already_printed = 0; 

    new_node->text = (char*)malloc(strlen(text)+1);
    if(new_node->text){
        strcpy(new_node->text, text); 
    }else{
        free(new_node); 
        return; 
    }

    new_node->next = NULL;
    new_node->already_printed = 0;

    /* Inserimento in coda alla lista */
    if (glob_commentListTail == NULL) {
        //lista vuota
        glob_commentListHead = new_node;
        glob_commentListTail = new_node;
    } else {
        // aggiungo in fondo
        glob_commentListTail->next = new_node;
        glob_commentListTail = new_node; 
    }
}

// libera la struttura dati per i commenti 
void free_comments(void) {
    comment_entry* curr = glob_commentListHead;
    while (curr!=NULL) {
        comment_entry* next = curr->next;
        if (curr->text) free(curr->text);
        free(curr);
        curr = next;
    }
    glob_commentListHead = NULL;
    glob_commentListTail = NULL;
}

comment_entry* find_comment_at_line(int line){
    comment_entry *curr = glob_commentListHead; 
    while(curr){ 
        if(curr->line_number == line){
            return curr;
        }
        curr = curr->next; 
    }
    return NULL; 
}