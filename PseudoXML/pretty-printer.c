/* pretty-printer con tracking dei commenti */

#include "parsup.h"
#include "comment_tracker.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Struttura per tracciare le linee già stampate con commenti */
typedef struct {
    int* lines;
    int count;
    int capacity;
} printed_lines;

static printed_lines plines = {NULL, 0, 0};

/* Inizializza la struttura delle linee stampate */
static void init_printed_lines(void) {
    plines.capacity = 100;
    plines.lines = (int*)malloc(plines.capacity * sizeof(int));
    plines.count = 0;
}

/* Registra una linea come stampata */
static void mark_line_printed(int line) {
    if (plines.count >= plines.capacity) {
        plines.capacity *= 2;
        plines.lines = (int*)realloc(plines.lines, plines.capacity * sizeof(int));
    }
    plines.lines[plines.count++] = line;
}

/* Verifica se una linea è già stata stampata */
static int is_line_printed(int line) {
    for (int i = 0; i < plines.count; i++) {
        if (plines.lines[i] == line) return 1;
    }
    return 0;
}

/* Libera la struttura delle linee */
static void free_printed_lines(void) {
    if (plines.lines) {
        free(plines.lines);
        plines.lines = NULL;
    }
    plines.count = 0;
    plines.capacity = 0;
}

/* Trova e stampa commenti per una specifica linea */
static void print_comments_for_line(FILE* out, int line, const char* indent) {
    if (is_line_printed(line)) return;
    
    comment_entry* curr = global_comments;
    while (curr != NULL) {
        if (curr->line_number == line) {
            fprintf(out, "%s#%s\n", indent ? indent : "", curr->text);
            mark_line_printed(line);
        }
        curr = curr->next;
    }
}

/* Stampa commenti standalone tra due linee */
static void print_standalone_comments(FILE* out, int from_line, int to_line, const char* indent) {
    for (int line = from_line; line < to_line; line++) {
        if (!is_line_printed(line)) {
            print_comments_for_line(out, line, indent);
        }
    }
}

/* Stampa un valore di campo */
static void print_field_value(FILE* out, field_entry* field) {
    switch (field->kind) {
        case is_Integer:
            fprintf(out, "%d", field->value_Integer);
            break;
            
        case is_Boolean:
            fprintf(out, "%s", field->value_Boolean ? "true" : "false");
            break;
            
        case is_String:
            fprintf(out, "\"%s\"", field->value_String);
            break;
            
        case is_Local:
            /* Riferimento locale: $var */
            if (field->copy_from != NULL) {
                fprintf(out, "$%s", field->copy_from->field_name);
            } else {
                fprintf(out, "$<undefined>");  /* Riferimento rotto */
            }
            break;
            
        case is_NonLocal:
            /* Riferimento non locale: $sezione.var */
            if (field->copy_from != NULL && field->copy_from->section != NULL) {
                fprintf(out, "$%s.%s", 
                        field->copy_from->section->name,
                        field->copy_from->field_name);
            } else {
                fprintf(out, "$<undefined>.<undefined>");  /* Riferimento rotto */
            }
            break;
            
        case is_Inherited:
            /* Campo ereditato: non stamparlo se è solo ereditato */
            /* Viene gestito nel chiamante */
            break;
            
        default:
            fprintf(out, "<unknown_value>");
            break;
    }
}

/* Stampa una sezione con tutti i suoi campi */
static void print_section(FILE* out, section_entry* section, int* current_line) {
    if (section == NULL) return;
    
    /* Commenti prima della sezione */
    print_comments_for_line(out, *current_line, "");
    
    /* Tag apertura sezione */
    fprintf(out, "<section name=%s>", section->name);
    
    /* Commento inline sulla stessa linea della section */
    comment_entry* curr = global_comments;
    while (curr != NULL) {
        if (curr->line_number == *current_line && 
            curr->position == COMMENT_INLINE) {
            fprintf(out, " #%s", curr->text);
            mark_line_printed(*current_line);
        }
        curr = curr->next;
    }
    fprintf(out, "\n");
    (*current_line)++;
    
    /* Prima stampa tutti gli <inherit> */
    field_entry* field = section->fields;
    while (field != NULL) {
        if (field->kind == is_Inherited && field->inherit_from != NULL) {
            print_comments_for_line(out, *current_line, "  ");
            
            fprintf(out, "  <inherit>%s</inherit>", 
                    field->inherit_from->section->name);
            
            /* Commento inline */
            curr = global_comments;
            while (curr != NULL) {
                if (curr->line_number == *current_line && 
                    curr->position == COMMENT_INLINE) {
                    fprintf(out, " #%s", curr->text);
                    mark_line_printed(*current_line);
                }
                curr = curr->next;
            }
            fprintf(out, "\n");
            (*current_line)++;
        }
        field = field->next;
    }
    
    /* Poi stampa tutti i campi normali */
    field = section->fields;
    while (field != NULL) {
        /* Non stampare campi solo ereditati (senza override) */
        if (field->kind != is_Inherited) {
            print_comments_for_line(out, *current_line, "  ");
            
            /* Tag apertura field */
            fprintf(out, "  <field name=%s>", field->field_name);
            
            /* Valore */
            print_field_value(out, field);
            
            /* Tag chiusura field */
            fprintf(out, "</field>");
            
            /* Commento inline */
            curr = global_comments;
            while (curr != NULL) {
                if (curr->line_number == *current_line && 
                    curr->position == COMMENT_INLINE) {
                    fprintf(out, " #%s", curr->text);
                    mark_line_printed(*current_line);
                }
                curr = curr->next;
            }
            fprintf(out, "\n");
            (*current_line)++;
        }
        field = field->next;
    }
    
    /* Commenti prima del tag di chiusura */
    print_comments_for_line(out, *current_line, "");
    
    /* Tag chiusura sezione */
    fprintf(out, "</section>");
    
    /* Commento inline sulla chiusura */
    curr = global_comments;
    while (curr != NULL) {
        if (curr->line_number == *current_line && 
            curr->position == COMMENT_INLINE) {
            fprintf(out, " #%s", curr->text);
            mark_line_printed(*current_line);
        }
        curr = curr->next;
    }
    fprintf(out, "\n");
    (*current_line)++;
}

/* Pretty-printer principale */
void pretty_print(FILE* out, section_entry* sections) {
    if (sections == NULL || out == NULL) {
        return;
    }
    
    init_printed_lines();
    int current_line = 1;
    
    /* gli import non vengono stampati
    
    /* Itera su tutte le sezioni */
    section_entry* sec = sections;
    while (sec != NULL) {
        print_section(out, sec, &current_line);
        sec = sec->next;
    }
    
    /* Stampa eventuali commenti finali rimasti */
    comment_entry* curr = global_comments;
    while (curr != NULL) {
        if (!is_line_printed(curr->line_number)) {
            fprintf(out, "#%s\n", curr->text);
        }
        curr = curr->next;
    }
    
    free_printed_lines();
}


/*
void pretty_print_to_file(const char* filename, section_entry* sections) {
    if (filename == NULL) {
        fprintf(stderr, "ERRORE: Nome file NULL in pretty_print_to_file\n");
        return;
    }
    
    FILE* out = fopen(filename, "w");
    if (out == NULL) {
        fprintf(stderr, "ERRORE: Impossibile aprire file '%s' per scrittura\n", filename);
        return;
    }
    
    pretty_print(out, sections);
    fclose(out);
    
    printf("Output serializzato in '%s'\n", filename);
} Wrapper per stampare su file */ 

/* Versione semplificata senza commenti (per debug) */
void pretty_print_simple(FILE* out, section_entry* sections) {
    if (sections == NULL || out == NULL) return;
    
    section_entry* sec = sections;
    while (sec != NULL) {
        fprintf(out, "<section name=%s>\n", sec->name);
        
        field_entry* field = sec->fields;
        while (field != NULL) {
            if (field->kind != is_Inherited) {
                fprintf(out, "  <field name=%s>", field->field_name);
                print_field_value(out, field);
                fprintf(out, "</field>\n");
            }
            field = field->next;
        }
        
        fprintf(out, "</section>\n");
        sec = sec->next;
    }
}