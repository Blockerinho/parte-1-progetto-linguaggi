#ifndef PARSUP_H
#define PARSUP_H

#include "Absyn.h"

/* strutture dati per mantenere i binding */
typedef struct section_entry section_entry;
typedef struct field_entry field_entry;
typedef struct backlink backlink;

/* lista concatenata di sezioni */
struct section_entry {
  char* name;
  section_entry* prev;
  section_entry* next;
  field_entry* fields;
};

section_entry* create_section_entry(char* name, section_entry* next);

/* lista concatenata di campi */
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

field_entry* create_field_entry(char* name, Value value, section_entry* section, field_entry* next, section_entry* bindings);

field_entry* create_field_inherited(char* name, section_entry* section, field_entry* inherited_from, field_entry* next);

/* risoluzione dei valori */ 
field_entry* resolve_value(field_entry* field);
field_entry* get_resolved_value(section_entry* sections,const char* section_name,const char* field_name);
void print_resolved_value(field_entry* resolved);

/* lista concatenata di riferimenti a campi */
struct backlink {
 field_entry* ptr;
 backlink* prev;
 backlink* next;
};

backlink* create_backlink(field_entry* ptr, backlink* next);

void delete_field_entry(field_entry* field);

void delete_backlink(backlink* backlink, field_entry* field);


field_entry* inherit_fields(char* section_name, section_entry* section, section_entry* bindings);

/* cancellazioni */
void delete_field(field_entry* field);
int delete_field_by_name(section_entry* section, const char* field_name);
void delete_section(section_entry* section);
section_entry* delete_section_by_name(section_entry* sections, const char* section_name);
void free_all_sections(section_entry* sections);

/* dati i fields della sezione corrente e un nome di field, cerca il field con quel nome tra i field dati */
field_entry* search_bindings_local(char* field_name, field_entry* fields);

/* dato un nome di sezione e un nome di field, cerca la sezione con quel nome e quindi il field con quel nome */
field_entry* search_bindings_nonlocal(section_entry* bindings, char* section_name, char* field_name);

void print_bindings(section_entry* bindings);
#endif
