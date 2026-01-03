#ifndef PARSUP_H
#define PARSUP_H


#ifdef DETECT_IMPORT_CYCLES
#include <sys/types.h>
#endif

#include "Absyn.h"

typedef struct section_entry section_entry;
typedef struct field_entry field_entry;
typedef struct backlink backlink;

/* Rappresenta una sezione. I puntatori prev e next permettono di formare una
   lista concatenata. */
struct section_entry {
  char* name;
  section_entry* prev;
  section_entry* next;
  field_entry* fields;
};

/* Crea una sezione vuota dato il nome e il prossimo elemento della lista. */
section_entry* create_section_entry(char* name, section_entry* next);

/* Rappresenta un campo. I puntatori prev e next permettono di formare una
   lista concatenata. Il puntatore references punta al campo a cui questo
   campo fa riferimento (con la notazione $ oppure perché ereditato), e ha
   senso solo se kind è is_Local, is_NonLocal o is_Inherited. Il puntatore
   backlinks contiene i campi che si riferiscono a questo campo. */
struct field_entry {
  char* name;
  section_entry* section;
  enum { is_Integer, is_Boolean, is_String, is_Local, is_NonLocal, is_Inherited } kind;
  union {
    int value_Integer;
    int value_Boolean;
    char* value_String;
  };
  field_entry* prev;
  field_entry* next;
  field_entry* references;
  backlink* backlinks;
};

/* Crea un campo dato nome, valore, sezione di appartenenza, lista degli altri
   campi, e bindings creati fino a quel momento (da usare nel caso di campi
   che si riferiscono ad altri campi in altre sezioni). */
field_entry* create_field_entry(char* name, Value value, section_entry* section, field_entry* next, section_entry* bindings);

/* Crea un campo ereditato dato nome, sezione di appartenenza, campo da cui
   eredita. */
field_entry* create_field_entry_inherited(char* name, section_entry* section, field_entry* inherited_from);

/* Dato il nome di una sezione, la sezione corrente e i bindings, eredita
   tutti i campi della sezione con quel nome nella sezione corrente. */
field_entry* inherit_fields(char* section_name, section_entry* section, section_entry* bindings);

/* Riferimento a un campo. I puntatori prev e next permettono di formare una
   lista concatenata. */
struct backlink {
 field_entry* ptr;
 backlink* prev;
 backlink* next;
};

backlink* create_backlink(field_entry* ptr, backlink* next);

#ifdef DETECT_IMPORT_CYCLES
/* Rappresenta un file importato, con l'inode. */
typedef struct imp_file imp_file;
struct imp_file {
  ino_t file_ino;
  imp_file* next;
};

imp_file* create_imp_file(ino_t file_ino, imp_file* next);

imp_file* search_imp_file(ino_t file_ino, imp_file* list);
#endif


/* Dato il nome di un campo e una lista di campi, cerca il campo con quel nome
   nella lista. */
field_entry* search_bindings_local(char* field_name, field_entry* fields);

/* Dato il nome di una sezione, il nome di un campo e i bindings, cerca il
   campo con quel nome. */
field_entry* search_bindings_nonlocal(section_entry* bindings, char* section_name, char* field_name);




/* risoluzione dei valori */ 
field_entry* resolve_field(field_entry* field);
Value resolve_value(field_entry* field);
Value get_resolved_value(section_entry* sections,const char* section_name,const char* field_name);
void print_resolved_value(field_entry* resolved);

/* Cancella una sezione, ritorna la nuova sezione all'inizio della lista. */
section_entry* delete_section_entry(section_entry* section);
section_entry* delete_section_by_name(char* section_name, section_entry* bindings);

/* Cancella un campo */
void delete_field_entry(field_entry* field);
void delete_field_by_name(char* field_name, char* section_name, section_entry* bindings);

/* Cancella un backlink */
void delete_backlink(backlink* backlink, field_entry* field);

void free_all_sections(section_entry* sections);


/* Printing dei bindings per debugging.
   La notazione -> significa che il campo si riferisce a un altro campo locale
   o non locale.
   La notazione => significa che il campo eredita da un'altra sezione.
   Tra { } c'è la lista dei backlink.
 */
void print_bindings(section_entry* bindings);
#endif
