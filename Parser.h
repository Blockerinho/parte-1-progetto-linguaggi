#ifndef PARSER_HEADER_FILE
#define PARSER_HEADER_FILE

#include <stdio.h>

#include "ParserSupport.h"

void pSourceFile(FILE *inp, section_entry** bindings);
void pnSourceFile(char *filename, section_entry** bindings);

#endif
