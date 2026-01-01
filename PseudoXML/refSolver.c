#include "PseudoXMLParserSupport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FUNZIONI PER RISOLUZIONE DEI VALORI */ 

// Trova un campo in una sezione dato il nome del campo
static field_entry* find_field_in_section(section_entry* section, const char* field_name) {
	if (section == NULL || field_name == NULL) {
		return NULL; 
	}

	field_entry* current_field = section->fields; 
	while(current_field){
		if(strcmp(current_field->name, field_name) == 0){
			return current_field; 
		}
		current_field= current_field->next; 
	}
	return NULL; 
}

// Trova sezione dato nome
static section_entry* find_section(section_entry* sections, const char* section_name) {
	if (sections == NULL || section_name == NULL) {
		return NULL;
	}
	section_entry* current_section = sections; 
	while (current_section) {
		if(strcmp(current_section->name, section_name) == 0) {
			return current_section; 
		}
		current_section = current_section->next; 
	}
	return NULL; 
}

// risoluzione campi (ricorsiva)
field_entry* resolve_field(field_entry* field) {
	if (field == NULL) {
		return NULL;
	}

	switch (field->kind) {
		case is_Integer:
		case is_Boolean:
		case is_String:
			return field;
		default:
			return resolve_field(field->references);
	}
}

// risoluzione field dati nome sezione e nome campo
Value get_resolved_value(section_entry* sections, const char* section_name, const char* field_name) {
	if (sections == NULL || section_name == NULL || field_name == NULL){
		return NULL; 
	}

	section_entry* section = find_section(sections, section_name); 
	if (section == NULL) {
		fprintf(stderr, "ERRORE"); 
		return NULL; 
	}

	field_entry* field = find_field_in_section(section, field_name); 
	if (field == NULL) {
        fprintf(stderr, "ERRORE: Campo '%s' non trovato nella sezione '%s'\n", 
        field_name, section_name);
        return NULL;
	}

	return resolve_value(field); 
}

Value resolve_value(field_entry* field) {
	field_entry* resolved_field = resolve_field(field);

	if (resolved_field == NULL) {
		return NULL;
	}

	Value value = malloc(sizeof(*value));

	switch(resolved_field->kind) {
		case is_Integer:
			value->kind = is_ValueInt;
			value->value_int = resolved_field->value_Integer;
			break;
		case is_Boolean:
			value->kind = is_ValueBool;
			value->value_bool = resolved_field->value_Boolean;
			break;
		case is_String:
			value->kind = is_ValueString;
			memcpy(value->value_string, resolved_field->value_String, strlen(resolved_field->value_String));
			break;
    default:
      fprintf(stderr, "Errore interno - estrazione valore di tipo non base\n");
      break;
	}

	return value;
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
