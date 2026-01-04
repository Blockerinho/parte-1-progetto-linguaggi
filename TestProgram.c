#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ParserSupport.h"
#include "Printer.h"
#include "CommentTracker.h"
#include "Parser.h"

int main(int argc, char ** argv)
{
  FILE *input;
  char *filename = NULL;

  if (argc > 1) {
    filename = argv[1];
  } else {
    input = stdin;
  }

  section_entry* bindings = NULL;
  
  if (filename) {
    pnSourceFile(filename, &bindings);
  } else {
    pSourceFile(input, &bindings);
  }
  
  if (!bindings) {
    fprintf(stderr, "Parsing failed: no bindings created\n"); 
    return 1;
  }
  
  char *output1 = printFromBindings(bindings);
  printf("%s\n", output1);
}

