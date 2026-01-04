#include "PseudoXMLParserSupport.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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