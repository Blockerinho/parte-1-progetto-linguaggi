#include <stdlib.h>
#include <stdio.h>

#include "Absyn.h"
#include "PseudoXMLParserSupport.h"

int check_toplevel_tag_order(SourceFile p) {
  ListTopLevelTag tl = p->u.mainFile_.listtopleveltag_;
  TopLevelTag t;
  int reached_section = 0;
  while (tl != 0) {
    t = tl->topleveltag_;
    switch (t->kind) {
      case is_FileImportTag:
        if (reached_section) {
          return 1;
        }
        break;
      case is_SectionTag:
        reached_section = 1;
        break;
      default:
        fprintf(stderr, "Error: bad kind for TopLevelTag");
        exit(1);
    }
    tl = tl->listtopleveltag_;
  }
  return 0;
}

int check_sublevel_tag_order(SourceFile p) {
  ListTopLevelTag tl = p->u.mainFile_.listtopleveltag_;
  TopLevelTag t;
  ListSubLevelTag sl;
  SubLevelTag s;
  int reached_field = 0;
  while (tl != 0) {
    t = tl->topleveltag_;
    if (t->kind != is_SectionTag) {
      tl = tl->listtopleveltag_;
      continue;
    }
    sl = t->u.sectionTag_.listsubleveltag_;
    while (sl != 0) {
      s = sl->subleveltag_;
      switch (s->kind) {
      case is_InheritTag:
        if (reached_field) {
          return 1;
        }
        break;
      case is_FieldTag:
        reached_field = 1;
        break;
      default:
        fprintf(stderr, "Error: bad kind for SubLevelTag");
        exit(1);
      }
      sl = sl->listsubleveltag_;
    }
    tl = tl->listtopleveltag_;
  }
  return 0;
}

section_entry* create_section_entry(char* name, section_entry* next) {
  section_entry* myself = (section_entry*) malloc(sizeof(*myself));
  myself->name = name;
  myself->next = next;
  return myself;
}

field_entry* create_field_entry(char* name, section_entry* section, field_entry* next) {
  field_entry* myself = (field_entry*) malloc(sizeof(*myself));

  myself->field_name = name;
  myself->section = section;
  myself->next = next;
  
  return myself;
}

field_entry* create_field_entry_Integer(char* name, section_entry* section, field_entry* next, Integer value) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_Integer;
  myself->value_Integer = value;
  return myself;
}

field_entry* create_field_entry_Bool(char* name, section_entry* section, field_entry* next, int value) {
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
  return myself;
}

field_entry* create_field_entry_Inherited(char* name, section_entry* section, field_entry* next, field_entry* inherit_from) {
  field_entry* myself = create_field_entry(name, section, next);
  myself->kind = is_Inherited;
  myself->inherit_from = inherit_from;
  return myself;
}

backlink* create_backlink(field_entry* ptr, backlink* next) {
  backlink* myself = (backlink*) malloc(sizeof(*myself));
  myself->ptr = ptr;
  myself->next = next;
  return myself;
}

/* section_entry* create_bindings_from_tree(SourceFile tree) {
  section_entry* bindings = NULL;  
  ListTopLevelTag tl = tree->u.mainFile_.listtopleveltag_; 
  TopLevelTag t;
  field_entry* f;
  while (tl) {
    t = tl->topleveltag_;
    if (t->kind == is_FileImportTag) {
      continue;
    }

    char* section_name = t->u.sectionTag_.ident_;
    bindings = create_section_entry(section_name, bindings);

    sl
    
  }
  return bindings;
} */
