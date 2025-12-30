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
  char* field_name;
  enum { is_Integer, is_Boolean, is_String, is_Local, is_NonLocal, is_Inherited } kind;
  union {
    Integer value_Integer;
    Boolean value_Boolean;
    String value_String;
  };
  section_entry* section;
  field_entry* prev;
  field_entry* next;
  field_entry* inherit_from;
  field_entry* copy_from;
  backlink* backlinks;
};

field_entry* create_field_entry(char* name, section_entry* section, field_entry* next);
field_entry* create_field_entry_Integer(char* name, section_entry* section, field_entry* next, Integer value);
field_entry* create_field_entry_Bool(char* name, section_entry* section, field_entry* next, Boolean value);
field_entry* create_field_entry_String(char* name, section_entry* section, field_entry* next, String value);
field_entry* create_field_entry_Local(char* name, section_entry* section, field_entry* next, field_entry* copy_from);
field_entry* create_field_entry_NonLocal(char* name, section_entry* section, field_entry* next, field_entry* copy_from);
field_entry* create_field_entry_Inherited(char* name, section_entry* section, field_entry* next, field_entry* inherit_from);

/* risoluzione dei valori */ 
field_entry* resolve_value(field_entry* field);
field_entry* get_resolved_value(section_entry* sections,const char* section_name,const char* field_name);
void print_resolved_value(field_entry* resolved);

/* lista concatenata di riferimenti a campi */
struct backlink {
 field_entry* ptr;
 backlink* next;
};

backlink* create_backlink(field_entry* ptr, backlink* next);

/* cancellazioni */
void delete_field(field_entry* field);
int delete_field_by_name(section_entry* section, const char* field_name);
void delete_section(section_entry* section);
section_entry* delete_section_by_name(section_entry* sections, const char* section_name);
void free_all_sections(section_entry* sections);

/* dato un albero, popola la struttura dati con i binding */
section_entry* create_bindings_from_tree(SourceFile tree);

/* data una sezione e un nome di field, cerca il field con quel nome in quella sezione */
field_entry* search_bindings_local(section_entry* section, char* field_name);

/* dato un nome di sezione e un nome di field, cerca la sezione con quel nome e quindi il field con quel nome */
field_entry* search_bindings_nonlocal(section_entry* bindings, char* section_name, char* field_name);

void print_bindings(section_entry* bindings);
#endif
