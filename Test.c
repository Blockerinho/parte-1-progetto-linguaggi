#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Absyn.h"
#include "PseudoXMLParserSupport.h"
#include "Printer.h"
#include "comment_tracker.h"

/* Forward declarations from parser */
extern void pSourceFile(FILE *inp, section_entry** bindings);
extern void pnSourceFile(const char *str, section_entry** bindings);

void usage(void) {
  printf("usage: Call with one of the following argument combinations:\n");
  printf("\t--help\t\tDisplay this help message.\n");
  printf("\t(no arguments)\tParse stdin verbosely.\n");
  printf("\t(files)\t\tParse content of files verbosely.\n");
  printf("\t-s (files)\tSilent mode. Parse content of files silently.\n");
}

int main(int argc, char ** argv)
{
  FILE *input = NULL;
  int quiet = 0;
  char *filename = NULL;
  
  if (argc > 1) {
    if (strcmp(argv[1], "-s") == 0) {
      quiet = 1;
      if (argc > 2) {
        filename = argv[2];
      } else {
        input = stdin;
      }
    } else {
      filename = argv[1];
    }
  }
  
  section_entry* bindings = NULL;
  
  if (filename) {
    pnSourceFile(filename, &bindings);
  } else {
    pSourceFile(stdin, &bindings);
  }
  
  if (!bindings) {
    fprintf(stderr, "Parsing failed: no bindings created\n"); 
    return 1;
  }
  
  char *output1 = printFromBindings(bindings);
  printf("%s\n", output1);
  
  /* Salva su file */
  if (!saveToFile("output.pseudoxml", output1)) {
    fprintf(stderr, "Error: Failed to save output to file\n");
    return 1;
  }
  
  /* Stampa bindings originali */
  print_bindings(bindings);
  
  /* (Round-trip test) */
  printf(" (Round-trip test) \n");
  
  /* Parse dell'output salvato */
  section_entry* bindings2 = NULL;
  pnSourceFile("output.pseudoxml", &bindings2);
  
  if (!bindings2) {
    fprintf(stderr, "Round-trip parsing failed!\n");
    return 1;
  }
  
  /* Genera il secondo output */
  char *output2 = printFromBindings(bindings2);
  printf("%s\n", output2);
  
  /* Salva il secondo output */
  if (!saveToFile("output2.pseudoxml", output2)) {
    fprintf(stderr, "Warning: Failed to save second output\n");
  } else {
    printf("\n[Second output saved to output2.pseudoxml]\n");
  }
  
  /* Stampa bindings del secondo parsing */
  print_bindings(bindings2);
  
  /* Cleanup */
  free_comments();
  
  return 0;
}