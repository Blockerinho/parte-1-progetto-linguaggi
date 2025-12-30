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

field_entry* create_field_entry(char* name, section_entry* section, field_entry* next) {
  field_entry* myself = (field_entry*) malloc(sizeof(*myself));

  myself->field_name = name;
  myself->section = section;
  myself->next = next;
  myself->backlinks = NULL;
  
  return myself;
}

field_entry* create_field_entry_Integer(char* name, section_entry* section, field_entry* next, Integer value) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_Integer;
  myself->value_Integer = value;
  return myself;
}

field_entry* create_field_entry_Bool(char* name, section_entry* section, field_entry* next, Boolean value) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_Boolean;
  myself->value_Boolean = value;
  return myself;
}

field_entry* create_field_entry_String(char* name, section_entry* section, field_entry* next, String value) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_String;
  myself->value_String = value;
  return myself;
}

field_entry* create_field_entry_Local(char* name, section_entry* section, field_entry* next, field_entry* copy_from) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_Local;
  myself->copy_from = copy_from;
  copy_from->backlinks = create_backlink(myself, copy_from->backlinks);
  return myself;
}

field_entry* create_field_entry_NonLocal(char* name, section_entry* section, field_entry* next, field_entry* copy_from) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_NonLocal;
  myself->copy_from = copy_from;
  copy_from->backlinks = create_backlink(myself, copy_from->backlinks);
  return myself;
}

field_entry* create_field_entry_Inherited(char* name, section_entry* section, field_entry* next, field_entry* inherit_from) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_Inherited;
  myself->inherit_from = inherit_from;
  inherit_from->backlinks = create_backlink(myself, inherit_from->backlinks);
  return myself;
}

backlink* create_backlink(field_entry* ptr, backlink* next) {
  backlink* myself = (backlink*) malloc(sizeof(*myself));
  myself->ptr = ptr;
  myself->next = next;
  return myself;
}

field_entry* search_bindings_local(section_entry* section, char* field_name) {
  field_entry* current_field = section->fields;
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

section_entry* create_bindings_from_tree(SourceFile tree) {
  section_entry* bindings = NULL;  
  ListTopLevelTag tl = tree->u.mainFile_.listtopleveltag_; 
  TopLevelTag t;
  field_entry* f;
  char* section_name;
  ListSubLevelTag sl;
  SubLevelTag s;
  while (tl) {
    t = tl->topleveltag_;
    if (t->kind == is_FileImportTag) {
      continue;
    }

    section_name = t->u.sectionTag_.ident_;
    bindings = create_section_entry(section_name, bindings);

    sl = t->u.sectionTag_.listsubleveltag_;
    f = NULL;
    while (sl) {
      s = sl->subleveltag_;

      switch (s->kind) {
        case is_FieldTag:
          switch (s->u.fieldTag_.value_->kind) {
            case is_ValueInt:
              f = create_field_entry_Integer(s->u.fieldTag_.ident_, bindings, f, s->u.fieldTag_.value_->u.valueInt_.integer_);
              break;
            case is_ValueBool:
              f = create_field_entry_Bool(s->u.fieldTag_.ident_, bindings, f, s->u.fieldTag_.value_->u.valueBool_.boolean_);
              break;
            case is_ValueString:
              f = create_field_entry_String(s->u.fieldTag_.ident_, bindings, f, s->u.fieldTag_.value_->u.valueString_.string_);
              break;
            case is_ValueNonLoc:
              switch (s->u.fieldTag_.value_->u.valueNonLoc_.nonlocvar_->kind) {
                case is_SimpleNonLoc:
                  char* ref_name = s->u.fieldTag_.value_->u.valueNonLoc_.nonlocvar_->u.simpleNonLoc_.ident_;
                  field_entry* ref_loc = search_bindings_local(bindings, ref_name);
                  f = create_field_entry_Local(s->u.fieldTag_.ident_, bindings, f, ref_loc);
                  break;
                case is_NonLoc:
                  char* ref_section_name = s->u.fieldTag_.value_->u.valueNonLoc_.nonlocvar_->u.nonLoc_.ident_1;
                  char* ref_field_name = s->u.fieldTag_.value_->u.valueNonLoc_.nonlocvar_->u.nonLoc_.ident_2;
                  field_entry* ref_nonloc = search_bindings_nonlocal(bindings, ref_section_name, ref_field_name);
                  f = create_field_entry_NonLocal(s->u.fieldTag_.ident_, bindings, f, ref_nonloc);
              }
          } 
      }
      bindings->fields = f;

      sl = sl->listsubleveltag_;
    }
    tl = tl->listtopleveltag_;
  }
  return bindings;
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
          printf("\tField %s: %i\n", f->field_name, f->value_Integer);
          break;
        case is_Boolean:
          switch(f->value_Boolean->kind) {
            case is_Boolean_true:
              printf("\tField %s: true\n", f->field_name);
              break;
            case is_Boolean_false:
              printf("\tField %s: false\n", f->field_name);
          }
          break;
        case is_String:
          printf("\tField %s: %s\n", f->field_name, f->value_String);
          break;
        case is_Local:
          printf("\tField %s -> Field %s\n", f->field_name, f->copy_from->field_name);
          break;
        case is_NonLocal:
          printf("\tField %s -> Section %s Field %s\n", f->field_name, f->copy_from->section->name, f->copy_from->field_name);
          break;
      }
      f = f->next;
    }

    bindings = bindings->next;
  }
}
