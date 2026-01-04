#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "Absyn.h"
#include "ParserSupport.h"

section_entry* create_section_entry(char* name, section_entry* next, int line) {
  section_entry* myself = (section_entry*) malloc(sizeof(*myself));
  myself->name = name;
  myself->prev = NULL;
  myself->next = next;
  myself->fields = NULL;
  myself->line = line;

  if (myself->next) {
    myself->next->prev = myself;
  }

  return myself;
}

field_entry* create_field_entry(char* name, Value value, section_entry* section, field_entry* next, section_entry* bindings, int line) {
  field_entry* myself = (field_entry*) malloc(sizeof(*myself));

  myself->name = name;
  myself->section = section;
  myself->prev = NULL;
  myself->next = next;
  myself->references = NULL;
  myself->backlinks = NULL;
  myself->line = line;

  if (myself->next) {
    myself->next->prev = myself;
  }

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
        fprintf(stderr, "Error: section %s: field %s references itself.\n", section->name, name);
        exit(1);
      }
      ref = search_bindings_local(ref_name, next);
      if (!ref) {
        fprintf(stderr, "Error: section %s: field %s references unknown local field %s.\n", section->name, name, ref_name);
        exit(1);
      }
      myself->references = ref;
      myself->references->backlinks = create_backlink(myself, myself->references->backlinks);
      break;
    case is_ValueNonLocal:
      myself->kind = is_NonLocal;
      ref_sec_name = value->value_nonlocal.section_name;
      ref_name = value->value_nonlocal.field_name;
      ref = search_bindings_nonlocal(bindings, ref_sec_name, ref_name);
      if (!ref) {
        fprintf(stderr, "Error: section %s: field %s references an unknown nonlocal field %s in section %s.\n", section->name, name, ref_name, ref_sec_name);
        exit(1);
      }
      myself->references = ref;
      myself->references->backlinks = create_backlink(myself, myself->references->backlinks);
      break;
    default:
      fprintf(stderr, "Error: section %s: invalid field kind for field %s.\n", section->name, name);
      exit(1);
  }
  return myself;
}

field_entry* create_field_entry_inherited(char* name, section_entry* section, field_entry* inherited_from, int line) {
  field_entry* current_field = section->fields;
  while (current_field) {
    if (strcmp(current_field->name, name) == 0) {
      if (current_field->kind == is_Inherited) {
        fprintf(stderr, "Error: section %s: field %s inherited from section %s conflicts with field %s inherited from section %s.\n", section->name, name, inherited_from->section->name, current_field->name, current_field->references->section->name);
        return section->fields; // non aggiungere il nuovo campo in caso di conflitto
      } else {
        // non dovrebbe accadere perché c'è già un controllo nel parser
        fprintf(stderr, "Error: inherited field %s comes after normal field %s in section %s.\n", name, current_field->name, section->name);
        exit(1);
      }
    }
    current_field = current_field->next;
  }
  
  field_entry* myself = (field_entry*) malloc(sizeof(*myself));

  myself->name = name;
  myself->section = section;
  myself->kind = is_Inherited;
  myself->prev = NULL;
  myself->next = section->fields;
  myself->line = line; 

  if (myself->next) {
    myself->next->prev = myself;
  }
  
  myself->references = inherited_from;
  myself->backlinks = NULL;

  myself->references->backlinks = create_backlink(myself, myself->references->backlinks);

  return myself; 
}

field_entry* inherit_fields(char* section_name, section_entry* section, section_entry* bindings) {
  section_entry* ancestor_section = bindings;
  while (ancestor_section) {
    if (strcmp(ancestor_section->name, section_name) == 0) {
      break;
    }
    ancestor_section = ancestor_section->next;
  }
  if (!ancestor_section) {
    fprintf(stderr, "Error: section %s: inherit action references unknown section %s.\n", section->name, section_name);
    exit(1);
  }

  field_entry* ancestor_field = ancestor_section->fields;
  while (ancestor_field) {
    // Passa la linea della sezione corrente ai campi ereditati
    section->fields = create_field_entry_inherited(ancestor_field->name, section, ancestor_field, section->line);
    ancestor_field = ancestor_field->next;
  }
  return section->fields;
}

backlink* create_backlink(field_entry* ptr, backlink* next) {
  backlink* myself = (backlink*) malloc(sizeof(*myself));
  myself->ptr = ptr;
  myself->prev = NULL;
  myself->next = next;

  if (myself->next) {
    myself->next->prev = myself;
  }
  return myself;
}

#ifdef DETECT_IMPORT_CYCLES
imp_file* create_imp_file(ino_t file_ino, imp_file* next) {
  imp_file* myself = (imp_file*) malloc(sizeof(*myself));
  myself->file_ino = file_ino;
  myself->next = next;
  return myself;
}

imp_file* search_imp_file(ino_t file_ino, imp_file* list) {
  imp_file* current_file = list;
  while (current_file) {
    if (current_file->file_ino == file_ino) {
      return current_file;
    }
    current_file = current_file->next;
  } 
  return NULL;
}
#endif

field_entry* search_bindings_local(char* field_name, field_entry* fields) {
  field_entry* current_field = fields;
  while (current_field) {
    if (strcmp(current_field->name, field_name) == 0) {
      return current_field;
    }
    current_field = current_field->next;
  }
  return NULL;  
}

field_entry* search_bindings_nonlocal(section_entry* bindings, char* section_name, char* field_name) {
  section_entry* current_section = bindings;
  field_entry* current_field;
  while (current_section) {
    current_field = current_section->fields;
    while (current_field) {
      if (strcmp(current_field->name, field_name) == 0) {
        return current_field;
      }
      current_field = current_field->next;
    }
    current_section = current_section->next;
  }
  return NULL;
}

section_entry* delete_section_entry(section_entry* section) {
  section_entry* new_first = section;
  while (new_first) {
    new_first = new_first->prev;
  }
  if (new_first == section) {
    new_first = section->next;
  }
  
  if (section->prev) {
    section->prev->next = section->next;
  }
  if (section->next) {
    section->next->prev = section->prev;
  }

  while (section->fields) {
    delete_field_entry(section->fields);
  }

  free(section);
  return new_first;
}

section_entry* delete_section_by_name(char* section_name, section_entry* bindings) {
  section_entry* section_to_delete = bindings;
  section_entry* new_first = NULL;
  while (section_to_delete) {
    if (strcmp(section_to_delete->name, section_name) == 0) {
      new_first = delete_section_entry(section_to_delete);
      break;
    }
    section_to_delete = section_to_delete->next;
  }
  return new_first;
}

void delete_field_entry(field_entry* field) {
  if (field->prev) {
    field->prev->next = field->next;
  } else {
    field->section->fields = field->next;
  }
  if (field->next) {
    field->next->prev = field->prev;
  }

  // Se ho un riferimento a un altro campo, cancella il backlink
  // corrispondente in tale campo.
  if (field->references) {
    backlink* remote_backlink = field->references->backlinks;
    while (remote_backlink) {
      if (remote_backlink->ptr == field) {
        delete_backlink(remote_backlink, field->references);
        break;
      }
      remote_backlink = remote_backlink->next;
    }
  }

  // Cancella ricorsivamente i campi che si riferiscono a me.
  backlink* current_backlink = field->backlinks;
  while (current_backlink) {
    if (current_backlink->ptr) {
      delete_field_entry(current_backlink->ptr);
    } else {
      fprintf(stderr, "Error: empty backlink in section %s, field %s", field->section->name, field->name); // non dovrebbe succedere
      exit(1);
    }
    current_backlink = current_backlink->next;
  }

  free(field);
}

void delete_field_by_name(char* field_name, char* section_name, section_entry* bindings) {
  section_entry* section_to_delete = bindings;
  field_entry* field_to_delete;
  while (section_to_delete) {
    if (strcmp(section_to_delete->name, section_name) == 0) {
      field_to_delete = section_to_delete->fields;
      while (field_to_delete) {
        if (strcmp(field_to_delete->name, field_name) == 0) {
          delete_field_entry(field_to_delete);
          return;
        }
        field_to_delete = field_to_delete->next;
      }
    }
    section_to_delete = section_to_delete->next;
  }
}

void delete_backlink(backlink* backlink, field_entry* field) {
  if (backlink->prev) {
    backlink->prev->next = backlink->next;
  } else {
    field->backlinks = backlink->next;
  }
  if (backlink->next) {
    backlink->next->prev = backlink->prev;
  }
  free(backlink);
}

void print_bindings(section_entry* bindings) {
  printf("Pretty printing data structure for bindings :\n\n");
  field_entry* f;
  while (bindings) {
    printf("Section %s\n", bindings->name);
    f = bindings->fields;
    while (f) {
      switch(f->kind) {
        case is_Integer:
          printf("\tField %s: %i {", f->name, f->value_Integer);
          break;
        case is_Boolean:
          if (f->value_Boolean == 0) {
              printf("\tField %s: false {", f->name);
          } else {
              printf("\tField %s: true {", f->name);
          }
          break;
        case is_String:
          printf("\tField %s: %s {", f->name, f->value_String);
          break;
        case is_Local:
          printf("\tField %s -> Field %s {", f->name, f->references->name);
          break;
        case is_NonLocal:
          printf("\tField %s -> Section %s Field %s {", f->name, f->references->section->name, f->references->name);
          break;
        case is_Inherited:
          printf("\tField %s => Section %s Field %s {", f->name, f->references->section->name, f->references->name);
        }

        backlink* current_backlink = f->backlinks;
        while(current_backlink) {
          printf("(Section %s, Field %s), ", current_backlink->ptr->section->name, current_backlink->ptr->name);
          current_backlink = current_backlink->next;
        }
        printf("}\n");
        f = f->next;
      }
      bindings = bindings->next;
    }
}
