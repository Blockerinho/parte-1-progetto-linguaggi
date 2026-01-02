#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Parser.h"
#include "Absyn.h"
#include "PseudoXMLParserSupport.h"
#include "Printer.h"
#include "comment_tracker.h"

void usage(void) {
  printf("usage: Call with one of the following argument combinations:\n");
  printf("\t--help\t\tDisplay this help message.\n");
  printf("\t(no arguments)\tParse stdin verbosely.\n");
  printf("\t(files)\t\tParse content of files verbosely.\n");
  printf("\t-s (files)\tSilent mode. Parse content of files silently.\n");
}

int main(int argc, char ** argv)
{
  FILE *input;
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
  SourceFile parse_tree = NULL; 
  section_entry* bindings = NULL;
  if (filename) {
    parse_tree = pnSourceFile(filename, &bindings);
    if (!input) {
      usage();
      exit(1);
    }
  }
  else {
    parse_tree = pSourceFile(stdin, &bindings);
  }
  
  if(parse_tree){
    char *output = printSourceFile(parse_tree); //pretty-printing
    printf("%s\n", output); 

    if (bindings) {
      print_bindings(bindings);
    }
    return 0;
  }else{
    fprintf(stderr, "Parsing failed: no AST generated\n"); 
    return 1; 
  }
}
