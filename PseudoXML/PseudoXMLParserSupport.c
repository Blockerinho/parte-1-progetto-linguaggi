#include <stdlib.h>
#include <stdio.h>

#include "Absyn.h"
#include "PseudoXMLParserSupport.h"

section_entry* create_section_entry(char* name, section_entry* next) {
  section_entry* myself = (section_entry*) malloc(sizeof(*myself));
  myself->name = name;
  myself->next = next;
  myself->fields = NULL;
  return myself;
}

field_entry* create_field_entry(char* name, Value value, section_entry* section, field_entry* next, section_entry* bindings) {
  field_entry* myself = (field_entry*) malloc(sizeof(*myself));

  myself->field_name = name;
  myself->section = section;
  myself->next = next;
  myself->backlinks = NULL;
  myself->value = value;

  char* ref_name;
  char* ref_sec_name;
  field_entry* ref;

  switch(value->kind) {
    case is_ValueInt:
      myself->kind = is_Integer;
      myself->value_Integer = value->value_int;
      break;
    case is_ValueBool:
      myself->kind = is_Boolean;
      myself->value_Boolean = value->value_bool;
      break;
    case is_ValueString:
      myself->kind = is_String;
      myself->value_String = value->value_string;
      break;
    case is_ValueLocal:
      myself->kind = is_Local;
      ref_name = value->value_local;
      if (strcmp(ref_name, name) == 0) {
        fprintf(stderr, "Error: a field cannot reference itself.\n");
        exit(1);
      }
      ref = search_bindings_local(ref_name, next);
      if (!ref) {
        fprintf(stderr, "Error: field references an unknown field.\n");
        exit(1);
      }
      myself->copy_from = ref;
      myself->copy_from->backlinks = create_backlink(myself, myself->copy_from->backlinks);
      break;
    case is_ValueNonLocal:
      myself->kind = is_NonLocal;
      ref_sec_name = value->value_nonlocal.section_name;
      ref_name = value->value_nonlocal.field_name;
      ref = search_bindings_nonlocal(bindings, ref_sec_name, ref_name);
      if (!ref) {
        fprintf(stderr, "Error: field references an unknown field.\n");
        exit(1);
      }
      myself->copy_from = ref;
      myself->copy_from->backlinks = create_backlink(myself, myself->copy_from->backlinks);
      break;
  }
  return myself;
}

backlink* create_backlink(field_entry* ptr, backlink* next) {
  backlink* myself = (backlink*) malloc(sizeof(*myself));
  myself->ptr = ptr;
  myself->next = next;
  return myself;
}

field_entry* search_bindings_local(char* field_name, field_entry* fields) {
  field_entry* current_field = fields;
  while (current_field) {
    if (strcmp(current_field->field_name, field_name) == 0) {
      return current_field;
    }
    current_field = current_field->next;
  }
  return NULL;  
}

/* dato un nome di sezione e un nome di field, cerca la sezione con quel nome e quindi il field con quel nome */
field_entry* search_bindings_nonlocal(section_entry* bindings, char* section_name, char* field_name) {
  section_entry* current_section = bindings;
  field_entry* current_field;
  while (current_section) {
    current_field = current_section->fields;
    while (current_field) {
      if (strcmp(current_field->field_name, field_name) == 0) {
        return current_field;
      }
      current_field = current_field->next;
    }
    current_section = current_section->next;
  }
  return NULL;
}

/* per ogni field appena aggiunto, aggiungi il riferimento alla sezione a cui appartiene */
void fill_sec_name(section_entry* section) {
  field_entry* current_field = section->fields;
  while (current_field) {
    current_field->section = section;
    current_field = current_field->next;
  }
}

field_entry* inherit_fields(char* section_name, field_entry* next, section_entry* bindings) {
  section_entry* current_section = bindings;
  while (current_section) {
    if (strcmp(current_section->name, section_name) == 0) {
      break;
    }
    current_section = current_section->next;
  }
  if (!current_section) {
    fprintf(stderr, "Inherit action references unknown section.\n");
    exit(1);
  }

  field_entry* current_field = current_section->fields;
  field_entry* new_field = next;;
  while (current_field) {
    new_field = create_field_entry(current_field->field_name, current_field->value, NULL, new_field, bindings);
    current_field->backlinks = create_backlink(new_field, current_field->backlinks);
    current_field = current_field->next;
  }
  return new_field;
}


void print_bindings(section_entry* bindings) {
  printf("Pretty printing struttura dati dei bindings:\n\n");
  field_entry* f;
  while (bindings) {
    printf("Section %s\n", bindings->name);
    f = bindings->fields;
    while (f) {
      switch(f->kind) {
        case is_Integer:
          printf("\tField %s: %i {", f->field_name, f->value_Integer);
          break;
        case is_Boolean:
          if (f->value_Boolean == 0) {
              printf("\tField %s: false {", f->field_name);
          } else {
              printf("\tField %s: true {", f->field_name);
          }
          break;
        case is_String:
          printf("\tField %s: %s {", f->field_name, f->value_String);
          break;
        case is_Local:
          printf("\tField %s -> Field %s {", f->field_name, f->copy_from->field_name);
          break;
        case is_NonLocal:
          printf("\tField %s -> Section %s Field %s {", f->field_name, f->copy_from->section->name, f->copy_from->field_name);
          break;
        case is_Inherited:
          break;
        }
        backlink* current_backlink = f->backlinks;
        while(current_backlink) {
          printf("(Section %s, Field %s), ", current_backlink->ptr->section->name, current_backlink->ptr->field_name);
          current_backlink = current_backlink->next;
        }
        printf("}\n");
        f = f->next;
      }
      bindings = bindings->next;
    }
  }
