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
    //invece di ignorare gli import, si attiva una procedura ricorsiva che apre il file, ne esegue il parsing e aggiunge le sezioni trovate alla lista globale dei bindings
    if (t->kind == is_FileImportTag) { 
      char* filename = t->u.fileImportTag_.string_;
      FILE *f_import = fopen(filename, "r");
      if (!f_import) {
        fprintf(stderr, "Errore: Impossibile aprire il file importato '%s'\n", filename);
        // Gestione errore come richiesto dalle specifiche
      } else {
        // 3. Esegui il parsing del file importato per ottenere un nuovo AST
        SourceFile imported_tree = pSourceFile(f_import);
        fclose(f_import);

        if (imported_tree) {
          // 4. CHIAMATA RICORSIVA: ottieni i bindings dal file importato
          // e uniscili a quelli attuali
          section_entry* imported_bindings = create_bindings_from_tree(imported_tree);
          
          // Logica di unione: aggiungi le sezioni importate in testa alla lista attuale
          if (imported_bindings) {
              section_entry* curr = imported_bindings;
              while (curr->next != NULL) curr = curr->next;
              curr->next = bindings; // Collega la coda degli import alla testa attuale
              bindings = imported_bindings;
          }
          free_SourceFile(imported_tree);
        }
      }
      tl = tl->listtopleveltag_;
      return bindings;    
    }

    if(t->kind= = is_SectionTag){
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
}
section_entry* create_bindings_from_tree(SourceFile tree) {
    section_entry* bindings = NULL;  
    ListTopLevelTag tl = tree->u.mainFile_.listtopleveltag_; 
    TopLevelTag t;

    while (tl) {
        t = tl->topleveltag_;
        if (t->kind == is_FileImportTag) { 
            char* filename = t->u.fileImportTag_.string_;
            
            FILE *f_import = fopen(filename, "r");
            if (!f_import) {
                fprintf(stderr, "Errore fatale: Impossibile aprire il file importato '%s'\n", filename);
                exit(1);
            }
            SourceFile imported_tree = pSourceFile(f_import); /* Parsing dell'import */ 
            fclose(f_import);
            if (imported_tree) {
                section_entry* imported_bindings = create_bindings_from_tree(imported_tree);
                if (imported_bindings) {
                    /* unione delle liste: cerca l'ultimo elemento della lista importata */
                    section_entry* last = imported_bindings;
                    while (last->next != NULL) {
                        last = last->next;
                    }
                    /* Attacca i bindings correnti alla fine della lista importata */
                    last->next = bindings; 
                    bindings = imported_bindings;
                }
                free_SourceFile(imported_tree);
            }
        } 
        //sezioni gestite come prima tranne che per il controllo dei duplicati, ora controllo di non aver importato e scritto una stessa sezione
        else if (t->kind == is_SectionTag) {
            char* section_name = t->u.sectionTag_.ident_;
            section_entry* check = bindings;
            while (check) {
                if (strcmp(check->name, section_name) == 0) {
                    fprintf(stderr, "Errore: La sezione '%s' è già stata definita. Esecuzione bloccata.\n", section_name);
                    exit(1);
                }
                check = check->next;
            }

            bindings = create_section_entry(section_name, bindings);

            ListSubLevelTag sl = t->u.sectionTag_.listsubleveltag_;
            field_entry* f_list = NULL;

            while (sl) {
                SubLevelTag s = sl->subleveltag_;
                if (s->kind == is_InheritTag) {
                    char* target_name = s->u.inheritTag_.ident_;
                    section_entry* target_sec = find_section_by_name(bindings, target_name);

                    if (!target_sec) {
                        fprintf(stderr, "Errore: Tentativo di ereditare da sezione inesistente '%s'\n", target_name);
                        exit(1);
                    } else {
                        /* Copia/Link dei campi dalla sezione target */
                        field_entry* tf = target_sec->fields;
                        while(tf) {
                            /* Controllo Conflitti Inherit  */
                            /* Cerchiamo se il campo esiste già in f_list (proveniente da un altro inherit) */
                            field_entry* conflict = find_field_in_list(f_list, tf->field_name);
                            if (conflict) {
                                fprintf(stderr, "Errore Conflitto Inherit: La variabile '%s' è definita in più sezioni ereditate ('%s' e altre).\n", tf->field_name, target_name);
                                conflict_inherit_detected = 1;
                            } else {
                                /* Aggiungi come Inherited */
                                f_list = create_field_entry_Inherited(tf->field_name, bindings, f_list, tf);
                            }
                            tf = tf->next;
                        }
                    }
                }
                
                if (s->kind == is_FieldTag) {
                    char* f_name = s->u.fieldTag_.ident_;
                    //controllo di non aver ridifinito stessa var in una sezione
                    if (search_bindings_local(bindings, f_name) != NULL) {
                        fprintf(stderr, "Warning: Ridefinizione della variabile '%s' nella sezione '%s'\n", f_name, section_name);
                    }
                    //uguale a prima
                    Value v = s->u.fieldTag_.value_;
                    switch (v->kind) {
                        case is_ValueInt:
                            f_list = create_field_entry_Integer(f_name, bindings, f_list, v->u.valueInt_.integer_);
                            break;
                        case is_ValueBool:
                            f_list = create_field_entry_Bool(f_name, bindings, f_list, v->u.valueBool_.boolean_); 
                            break;
                        case is_ValueString:
                            f_list = create_field_entry_String(f_name, bindings, f_list, v->u.valueString_.string_);
                            break;
                        case is_ValueNonLoc: {
                            NonLocVar nv = v->u.valueNonLoc_.nonlocvar_;
                            if (nv->kind == is_SimpleNonLoc) {
                                char* ref_name = nv->u.simpleNonLoc_.ident_;
                                field_entry* target = search_bindings_local(bindings, ref_name);
                                f_list = create_field_entry_Local(f_name, bindings, f_list, target);
                            } else {
                                char* r_sez = nv->u.nonLoc_.ident_1;
                                char* r_field = nv->u.nonLoc_.ident_2;
                                field_entry* target = search_bindings_nonlocal(bindings, r_sez, r_field);
                                f_list = create_field_entry_NonLocal(f_name, bindings, f_list, target);
                            }
                            break;
                        }
                    }
                } 

                /* Se ci sono stati conflitti tra inherit, bloccare tutto */
                if (conflict_inherit_detected) {
                    fprintf(stderr, "Interruzione esecuzione per conflitti in inherit nella sezione '%s'.\n", section_name);
                    exit(1);
                }

                bindings->fields = f_list;
                sl = sl->listsubleveltag_;
            }
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
