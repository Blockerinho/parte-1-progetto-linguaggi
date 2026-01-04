/* Generate header file for lexer. */
%defines "Bison.h"

%code requires {
#include "ParserSupport.h"
}

/* Reentrant parser */
%define api.pure full
%lex-param   { yyscan_t scanner }
%parse-param { yyscan_t scanner }

/* Turn on line/column tracking in the pseudo_xm_lgrammatica_lloc structure: */
%locations

/* Argument to the parser to be filled with the parsed tree. */
%parse-param { section_entry** bindings }
%parse-param { int* reached_section}

%{
/* Begin C preamble code */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef DETECT_IMPORT_CYCLES
#include <sys/stat.h>
#endif

#include "Absyn.h"
#include "ParserSupport.h"
#include "Parser.h"

#define YYMAXDEPTH 10000000

/* The type yyscan_t is defined by flex, but we need it in the parser already. */
#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

typedef struct yy_buffer_state *YY_BUFFER_STATE;
extern YY_BUFFER_STATE pseudo_xm_lgrammatica__scan_string(const char *str, yyscan_t scanner);
extern void pseudo_xm_lgrammatica__delete_buffer(YY_BUFFER_STATE buf, yyscan_t scanner);

extern void pseudo_xm_lgrammatica_lex_destroy(yyscan_t scanner);
extern char* pseudo_xm_lgrammatica_get_text(yyscan_t scanner);

extern yyscan_t pseudo_xm_lgrammatica__initialize_lexer(FILE * inp);

int reached_field = 0; // abbiamo incontrato un tag field nella sezione corrente?
section_entry* new_section;

#ifdef DETECT_IMPORT_CYCLES
imp_file* imp_file_list = NULL;
#endif

/* End C preamble code */
%}

%union
{
  int    _int;
  char   _char;
  double _double;
  char*  _string;
  Value value_;
}

%{
void yyerror(YYLTYPE *loc, yyscan_t scanner, section_entry** bindings, int* reached_section, const char *msg)
{
  fprintf(stderr, "error: %d,%d: %s at %s\n",
    loc->first_line, loc->first_column, msg, pseudo_xm_lgrammatica_get_text(scanner));
}

int yyparse(yyscan_t scanner, section_entry** bindings, int* reached_section);

extern int yylex(YYSTYPE *lvalp, YYLTYPE *llocp, yyscan_t scanner);
%}

%token          _ERROR_
%token          _DOLLAR      /* $ */
%token          _DOT         /* . */
%token          _SLASH       /* / */
%token          _LT          /* < */
%token          _EQ          /* = */
%token          _GT          /* > */
%token          _KW_false    /* false */
%token          _KW_field    /* field */
%token          _KW_import   /* import */
%token          _KW_inherit  /* inherit */
%token          _KW_name     /* name */
%token          _KW_section  /* section */
%token          _KW_true     /* true */
%token<_string> T_Ident      /* Ident */
%token<_string> _STRING_
%token<_int>    _INTEGER_
%token<_string> _IDENT_

%type <value_> Value

%start SourceFile

%%

SourceFile
  : ListTopLevelTag {}
;

ListTopLevelTag
  : /* empty */ {}
  | ListTopLevelTag TopLevelTag {}
;

TopLevelTag
  : _LT _KW_import _GT _STRING_ _LT _SLASH _KW_import _GT {
      if (*reached_section) {
        fprintf(stderr, "Error: imports must come before sections.\n");
        exit(1);
      }
      pnSourceFile($4, bindings);
     }
  | _LT _KW_section _KW_name  _EQ T_Ident {
      /* Passing line number @5.first_line */
      new_section = create_section_entry($5, *bindings, @5.first_line); 
  } _GT ListSubLevelTag _LT _SLASH _KW_section _GT {
      section_entry* current_section = *bindings;
      while (current_section) {
        if (!strcmp(current_section->name, $5)) {
          fprintf(stderr, "Error: section %s has already been defined.\n", $5);
          exit(1);
        }
        current_section = current_section->next;
      }
      *reached_section = 1;
      reached_field = 0;
      *bindings = new_section;
    }
;

ListSubLevelTag
  : /* empty */ {}
  | ListSubLevelTag SubLevelTag {}
;

SubLevelTag
  : _LT _KW_field _KW_name _EQ T_Ident _GT Value _LT _SLASH _KW_field _GT {
      reached_field = 1;

      field_entry* current_field = new_section->fields;
      while (current_field) {
        if (!strcmp(current_field->name, $5)) {
          if (current_field->kind == is_Inherited) {
            fprintf(stderr, "Info: overwriting field %s in section %s that was inherited from section %s.\n", $5, new_section->name, current_field->references->section->name);
            delete_field_entry(current_field);
            new_section->fields = create_field_entry($5, $7, new_section, new_section->fields, *bindings, @5.first_line);
            break;
          } else {
            fprintf(stderr, "Warning: section %s: field %s has already been defined, skipping it.\n", new_section->name, $5);
            break;
          }
        }
        current_field = current_field->next;
      }
      if (!current_field) {
        new_section->fields = create_field_entry($5, $7, new_section, new_section->fields, *bindings, @5.first_line);
      }
    }
  | _LT _KW_inherit _GT T_Ident _LT _SLASH _KW_inherit _GT {
      if (reached_field) {
        fprintf(stderr, "Error: inherited fields must come before normal fields.\n");
        exit(1);
      }
      new_section->fields = inherit_fields($4, new_section, *bindings);
    }
;

Value
  : _INTEGER_ { $$ = make_ValueInt($1); }
  | _KW_true { $$ = make_ValueBool(1); }
  | _KW_false { $$ = make_ValueBool(0); }
  | _STRING_ { $$ = make_ValueString($1); }
  | _DOLLAR T_Ident { $$ = make_ValueLocal($2); }
  | _DOLLAR T_Ident _DOT T_Ident { $$ = make_ValueNonLocal($2, $4); }
;

%%

/* Entrypoint: parse SourceFile from file. */
void pSourceFile(FILE *inp, section_entry** bindings)
{
  int reached_section = 0;
  yyscan_t scanner = pseudo_xm_lgrammatica__initialize_lexer(inp);
  if (!scanner) {
    fprintf(stderr, "Failed to initialize lexer.\n");
    return;
  }
  int error = yyparse(scanner, bindings, &reached_section);
  pseudo_xm_lgrammatica_lex_destroy(scanner);
  if (error)
  { /* Failure */
    return;
  }
  else
  { /* Success */
    return;
  }
}

/* Entrypoint: parse SourceFile from filename. */
void pnSourceFile(char* filename, section_entry** bindings)
{
  int reached_section = 0;

  FILE* i = fopen(filename, "r");
  #ifdef DETECT_IMPORT_CYCLES
  if (!i) {
    fprintf(stderr, "Error opening imported file %s\n", filename);
    perror("Error");
    exit(1);
  }
  struct stat st;
  if (stat(filename, &st) != 0) {
    fprintf(stderr, "Could not stat file %s\n", filename);
    exit(1);
  }

  if (search_imp_file(st.st_ino, imp_file_list) != NULL) {
    fprintf(stderr, "Warning: file %s imported twice. There might be an import cycle. Skipping.\n", filename);
    return;
  }
  imp_file_list = create_imp_file(st.st_ino, imp_file_list);
  #endif

  yyscan_t scanner = pseudo_xm_lgrammatica__initialize_lexer(i);
  if (!scanner) {
    fprintf(stderr, "Failed to initialize lexer.\n");
    return;
  }
  int error = yyparse(scanner, bindings, &reached_section);
  pseudo_xm_lgrammatica_lex_destroy(scanner);
  if (error)
  { /* Failure */
    return;
  }
  else
  { /* Success */
    return;
  }
}
