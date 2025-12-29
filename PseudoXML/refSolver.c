#include "PseudoXMLParserSupport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FUNZIONI PER RISOLUZIONE DEI VALORI */ 

/* per prevenire i cicli mantengo una list di campi visitati.
Se ne incontro uno già visitato allora so di avere un ciclo -> errore. */
typedef struct visit_node{
	field_entry* field; 
	struct visit_node* next; 
}visit_node; 

static int is_visited(visit_node* visited, field_entry* field){
	visit_node* curr = visited; 
	while (curr != NULL) {
		if(curr->field == field) {
			return 1;
		}
		curr = curr->next; 
	}
	return 0; 
}

static visit_node* add_visited(visit_node* visited, field_entry* field){
	visit_node* new_node = (visit_node*)malloc(sizeof(visit_node));
	new_node->field = field; 
	new_node->next = visited; 
	return new node; 
}

static void free_visited(visit_node* visited){
	while(visited!=NULL){
		visit_node* tmp = visited; 
		visited = visited->next; 
		free(tmp);
	}
}

// Trova un campo in una sezione dato il nome del campo
static field_entry* find_field_in_section(section_entry* section, cont char* field_name){
	if(section==NULL || field_name==NULL){
		return NULL; 
	}
	field_entry* curr = section->fields; 
	while(curr!=NULL){
		if(strcmp(curr->field_name, field_name)==0){
			return curr; 
		}
		curr = curr->next; 
	}
	return NULL; 
}

// Trova sezione dato nome
static section_entry* find_section(section_entry* sections, const char* section_name){
	if(sections==NULL || section_name==NULL){
		return NULL;
	}
	section_entry* curr = sections; 
	while(curr!=NULL){
		if(strcmp(curr->name, section_name)==0){
			return curr; 
		}
		curr = curr->next; 
	}
	return NULL; 
}

//risoluzione interna tenendo traccia dei cicli (ricorsiva)
static field_entry* resolve_value_internal(field_entry* field, visit_node* visited){
	
	if(field==NULL){
		return NULL; 
	}
	
	if(is_visited(visited,field)){
		fprintf(stderr, "ERRORE: Riferimento circolare, campo '%s'\n",field->field_name);
        return NULL;
	}

	visit_node* new_visited = add_visited(visited,field); 

	field_entry* result = NULL; 

	switch(field->kind){
		case is_Integer: 
		case is_Boolean: 
		case is_String: 
			result = field; 
			break; 
		//seguo copy_from
		case is_Local: 
		case is_NonLocal: 
			if(field->copy_from != NULL){
				result = resolve_value_internal(field->copy_from, new_visited); 
			}else{
				fprintf(stderr, "ERRORE: riferimento non valido per '%s'\n",field->field_name); 
				result = NULL; 
			}
			break; 
		//seguo inherit_from
		case is_Inherited: 
			if(field->inherit_from != NULL){
				result = resolve_value_internal(field->inherit_from, new_visited); 
			}else{
				fprintf(stderr, "ERRORE: inherit non valido per '%s'\n",field->field_name); 
				result = NULL; 
			}
			break; 

		default: 
			fprintf(stderr, "ERRORE"); 
			result = NULL; 
			break; 
	}
	return result; 

}

field_entry* resolve_value(field_entry* field){
	if(field==NULL){
		return NULL; 
	}
	field_entry* result = resolve_value_internal(field,NULL);
	return result; 
}

//dato sezione, nome sezione e nome field restituisce valore base
field_entry* get_resolved_value(section_entry* sections, const char* section_name, const char* field_name){
	if(sections==NULL || section_name==NULL || field_name==NULL){
		return NULL; 
	}

	section_entry* section = find_section(sections, section_name); 
	if(section==NULL){
		fprintf(stderr, "ERRORE"); 
		return NULL; 
	}

	field_entry* field = find_field_in_section(section, field_name); 
	if(field == NULL){
        fprintf(stderr, "ERRORE: Campo '%s' non trovato nella sezione '%s'\n", 
        field_name, section_name);
        return NULL;
	}

	return resolve_value(field); 
}

//printing del valore risolto per i vari tipi che gestiamo è solo per il test
void print_resolved_value(field_entry* resolved) {
    if (resolved == NULL) {
        printf("(valore non risolvibile)\n");
        return;
    }
    
    switch (resolved->kind) {
        case is_Integer:
            printf("%d\n", resolved->value_Integer);
            break;
        case is_Boolean:
            printf("%s\n", resolved->value_Boolean ? "true" : "false");
            break;
        case is_String:
            printf("\"%s\"\n", resolved->value_String);
            break;
        default:
            printf("(tipo non base - errore interno)\n");
            break;
    }
}