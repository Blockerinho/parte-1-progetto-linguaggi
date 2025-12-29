#include "PseudoXMLParserSupport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// dato un backlink specifico lo rimuove dalla lista dei backlinks
static void remove_backlink(field_entry* target, field_entry* referrer){
	if(target == NULL || referrer == NULL){
		return; 
	}

	backlink** curr = &(target->backlinks); 

	while(*curr!=NULL){
		if((*curr)->ptr == referrer) {
			backlink* to_free = *curr; 
			*curr = (*curr)->next; 
			free(to_free); 
			return; 
		}
		curr = &((*curr)->next);
	}
}

// invalida i riferimenti entranti
static void invalid_backlinks(field_entry* field) {
	if(field==NULL){
		return;
	}
	backlink* curr = field->backlinks;
	while(curr!=NULL){
		field_entry* referrer = curr->ptr; 
		if(referrer->kind == is_Local || referrer->kind == is_NonLocal){
			referrer->copy_from = NULL;
            fprintf(stderr, "WARNING: Campo '%s' in sezione '%s' referenzia un campo cancellato\n", referrer->field_name, referrer->section->name);
		}else if(referrer->kind == is_Inherited){
			referrer->inherit_from = NULL; 
			fprintf(stderr, "WARNING: Campo '%s' in sezione '%s' eredita da un campo cancellato\n",referrer->field_name, referrer->section->name);
		}

		curr = curr->next; 
	}

	//libera i backlinks 
	curr = field->backlinks; 
	while(curr!=NULL){
		backlink* to_free = curr; 
		curr = curr->next; 
		free(to_free); 
	}
	field->backlinks = NULL; 
}

//invalida i backlinks uscenti 
static void remove_outgoing_backlinks(field_entry* field){
	if(field==NULL){
		return; 
	}

	//capo che referenzia campo
	if((field->kind==is_Local || field->kind==is_NonLocal) && field->copy_from != NULL){
		remove_backlink(field->copy_from, field);
	}

	//campo che eredita da un altro
	if(field->kind==is_Inherited && field->inherit_from != NULL){
		remove_backlink(field->inherit_from, field); 
	}
}

// cancellazione di un field
void delete_field(field_entry* field){
	if(field==NULL){
		return;
	}

	// 1) invalida tutti i suoi riferimenti 
	invalid_backlinks(field); 
	// 2) rimuove outgoing backlinks
	remove_outgoing_backlinks(field); 
	// 3) lo rimuove dalla lista della sezione che lo conteneva
	if(field->prev!=NULL){
		field->prev->next = field->next; 
	}else{
		//primo campo della sezione
		field->section->fields = field->next; 
	}

	if(field->next != NULL){
		field->next->prev = field->prev; 
	}

	free(field->field_name); 
	if (field->kind == is_String) {
        free(field->value_String);
    }
    free(field);
}

// cancella field dato il suo nome
int delete_field_by_name(section_entry* section, const char* field_name){
	if(section==NULL || field_name==NULL){
		return -1; 
	}
	field_entry* curr = section->fields; 
	while(curr!=NULL){
		if(strcmp(curr->field_name, field_name)==0){
			delete_field(curr); 
			return 0; 
		}
		curr = curr->next; 
	}
	return -1; 
}

// cancella tutti i field 
static void delete_all_fields(section_entry* section){
	if(section==NULL){
		return; 
	}
	field_entry* curr = section->fields; 
	while(curr!=NULL){
		field_entry* next = curr->next; 
		delete_field(curr); 
		curr = next; 
	}
	section->fields=NULL; 
}

//cancella tutta la sezione
void delete_section(section_entry* section){
	if(section==NULL){
		return; 
	}

	delete_all_fields(section);

	//rimuove section dalla lista delle section
	if(section->prev != NULL){
		section->prev->next = section->next; 
	}

	if(section->next!=NULL){
		section->next->prev = section->prev; 
	}

	//libera la memoria
	free(section->name);
	free(section); 
}

//cancella sezione dato nome
void delete_section_by_name(section_entry* sections, const char* section_name){
	if(sections==NULL || section_name == NULL){
		return sections; 
	}

	section_entry* curr = sections; 
	section_entry* head = sections; //se la sezione da cancellare è la prima della lista di sezioni devo aggiornare

	while(curr!=NULL){
		if(strcmp(curr->name, section_name)==0){
			if(curr == head){
				head = curr->next; 
			}
			section_entry* next = curr->next; 
			delete_section(curr); 
			return head; 
		}
		curr = curr->next;
	}
	return head; 
}

//libera tutto 
void free_all_sections(section_entry* sections){
	section_entry* curr = sections; 
	while(curr!=NULL){
		section_entry* next = curr->next; 
		delete_section(curr); 
		curr = next; 
	}
}

// qua si può aggiungere qualche roba per testare