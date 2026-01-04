#include <ctype.h>   /* isspace */
#include <stddef.h>  /* size_t */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Printer.h"
#include "CommentTracker.h"
#include "ParserSupport.h"

#define INDENT_WIDTH 2

int _n_;
char *buf_;
size_t cur_;
size_t buf_size;

void indent(void)
{
  int n = _n_;
  while (--n >= 0)
    bufAppendC(' ');
}

void removeTrailingSpaces()
{
  while (cur_ && buf_[cur_ - 1] == ' ') --cur_;
  buf_[cur_] = 0;
}

void onEmptyLine()
{
  removeTrailingSpaces();
  if (cur_ && buf_[cur_ - 1 ] != '\n') bufAppendC('\n');
  indent();
}

void renderC(char c)
{
  /* Per XML usiamo una logica diversa gestita manualmente in printFromBindings */
  bufAppendC(c);
  bufAppendC(' '); 
}

int allIsSpace(char* s)
{
  char c;
  while ((c = *s++))
    if (! isspace(c)) return 0;
  return 1;
}

void renderS(char* s)
{
  if (*s) 
  {
    bufAppendS(s);
    bufAppendC(' ');
  }
}

void backup(void)
{
  if (cur_ && buf_[cur_ - 1] == ' ')
    buf_[--cur_] = 0;
}

/* --- FUNZIONI PER I COMMENTI --- */

void flush_comments_up_to(int line_limit){
  comment_entry *curr = glob_commentListHead; 
  while(curr){
    /* Se troviamo un commento che viene prima della linea corrente */
    if(curr->line_number < line_limit && !curr->already_printed){
      //onEmptyLine(); // Vai a capo prima di stampare il commento
      bufAppendS(curr->text); 
      onEmptyLine(); // Vai a capo dopo il commento
      curr->already_printed = 1; 
    }
    curr = curr->next; 
  }
}

void print_inline_comments(int current_line){
  comment_entry *curr = glob_commentListHead; 
  while(curr){ 
    if(curr->line_number == current_line && !curr->already_printed){
      if(*(curr->text) != '>'){
        bufAppendC(' '); // Spazio prima del commento inline
        bufAppendS(curr->text); 
        curr->already_printed = 1; 
      }
    }
    curr = curr->next; 
  }
}

void flush_remaining_comments(){

  comment_entry *curr = glob_commentListHead;
  int printed_something = 0;
  while(curr){
      if(!curr->already_printed){
        if (!printed_something) onEmptyLine();
        bufAppendS(curr->text);
        bufAppendC('\n');
        curr->already_printed = 1;
        printed_something = 1;
      }
      curr = curr->next;
  }
}

/* Funzioni per invertire le liste */

/* Inverte la lista delle sezioni per processarle in ordine di apparizione */
section_entry* reverse_sections(section_entry* head) {
    section_entry* prev = NULL;
    section_entry* current = head;
    section_entry* next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

field_entry* reverse_fields(field_entry* head) {
    field_entry* prev = NULL;
    field_entry* current = head;
    field_entry* next = NULL;
    while (current != NULL) {
        next = current->next;
        current->next = prev;
        prev = current;
        current = next;
    }
    return prev;
}

void ppInteger(int n, int i);
void ppBoolean(int boolean_val, int _i_);
void ppString(char* s, int i);
void ppIdent(char* s, int i);

/* STAMPA LA SYMBOL TABLE COME CODICE SORGENTE */
char *printFromBindings(section_entry* bindings)
{
  _n_ = 0;
  bufReset();
  
  /* INVERSIONE TEMPORANEA (Cruciale per l'ordine di stampa) */
  /* Invertiamo la lista sezioni per stamparle nell'ordine originale
     il parser costruisce la lista concatenata in memoria (LIFO) rispetto a come deve essere stampata (FIFO). 
     Durante il parsing, ogni volta che viene trovata una nuova sezione o un nuovo campo, questo viene inserito in testa alla lista esistente */

  section_entry* reversed_bindings = reverse_sections(bindings);
  
  /* Invertiamo i campi dentro ogni sezione */
  section_entry* s = reversed_bindings;
  while(s) {
      s->fields = reverse_fields(s->fields);
      s = s->next;
  }

  if(reversed_bindings){
    flush_comments_up_to(reversed_bindings->line); 
  }
  
  section_entry* current_section = reversed_bindings; 
  
  while (current_section) {

    flush_comments_up_to(current_section->line);
    onEmptyLine();
    
    bufAppendC('<');
    bufAppendS("section");
    bufAppendC(' ');
    bufAppendS("name");
    bufAppendC('=');
    ppIdent(current_section->name, 0);
    bufAppendC('>');
    
    print_inline_comments(current_section->line);
    
    _n_ += INDENT_WIDTH;
    
    field_entry* current_field = current_section->fields;
    while (current_field) {
        
        flush_comments_up_to(current_field->line);
        onEmptyLine();

        if (current_field->kind == is_Inherited) {
             bufAppendC('<');
             bufAppendS("inherit");
             bufAppendC('>');
             if (current_field->references && current_field->references->section)
                 ppIdent(current_field->references->section->name, 0);
             bufAppendC('<');
             bufAppendC('/');
             bufAppendS("inherit");
             bufAppendC('>');

        } else {
             bufAppendC('<');
             bufAppendS("field");
             bufAppendC(' ');
             bufAppendS("name");
             bufAppendC('=');
             ppIdent(current_field->name, 0);
             bufAppendC('>');
             switch (current_field->kind) {
                case is_Integer: ppInteger(current_field->value_Integer, 0); break;
                case is_Boolean: ppBoolean(current_field->value_Boolean, 0); break;
                case is_String:  ppString(current_field->value_String, 0);
                  break;
                case is_Local:   
                    bufAppendC('$'); 
                    if(current_field->references) ppIdent(current_field->references->name, 0); 
                    break;
                case is_NonLocal: 
                    bufAppendC('$');
                    if(current_field->references && current_field->references->section) {
                        ppIdent(current_field->references->section->name, 0);
                        bufAppendC('.');
                        ppIdent(current_field->references->name, 0);
                    }
                    break;
                default: break;
            }
            bufAppendC('<');
            bufAppendC('/');
            bufAppendS("field");
            bufAppendC('>');
        }
        print_inline_comments(current_field->line);

        current_field = current_field->next;
    }

    _n_ -= INDENT_WIDTH;
    onEmptyLine();
    bufAppendC('<');
    bufAppendC('/');
    bufAppendS("section");
    bufAppendC('>');
    
    current_section = current_section->next;
  }
  
  flush_remaining_comments();

  /* 3. RIPRISTINO STATO (Restore original list order) */
  /* Re-invertiamo i campi */
  s = reversed_bindings;
  while(s) {
      s->fields = reverse_fields(s->fields);
      s = s->next;
  }
  /* Re-invertiamo le sezioni per tornare allo stato originale */
  reverse_sections(reversed_bindings);

  return buf_;
}

void ppBoolean(int boolean_val, int _i_) {
  if (boolean_val) bufAppendS("true");
  else bufAppendS("false");
}

void ppInteger(int n, int i) {
  char tmp[20];
  sprintf(tmp, "%d", n);
  bufAppendS(tmp);
}

void ppString(char* s, int i) {
  bufAppendC('\"');
  bufAppendS(s); 
  bufAppendC('\"');
}

void ppIdent(char* s, int i) {
  bufAppendS(s);
}

/* Salva l'output del pretty-printer su file */
int saveToFile(const char* filename, const char* content)
{
  if (!content) {
    fprintf(stderr, "Error: Failed to generate output\n");
    return 0;
  }
  
  FILE* file = fopen(filename, "w");
  if (!file) {
    fprintf(stderr, "Error: Cannot open file '%s' for writing\n", filename);
    return 0;
  }
  
  fprintf(file, "%s", content);
  fclose(file);
  
  return 1;
}


void bufEscapeS(const char *s)
{
  if (s) while (*s) bufEscapeC(*s++);
}

void bufEscapeC(const char c)
{
  switch(c)
  {
    case '\f': bufAppendS("\\f" ); break;
    case '\n': bufAppendS("\\n" ); break;
    case '\r': bufAppendS("\\r" ); break;
    case '\t': bufAppendS("\\t" ); break;
    case '\v': bufAppendS("\\v" ); break;
    case '\\': bufAppendS("\\\\"); break;
    case '\'': bufAppendS("\\'" ); break;
    case '\"': bufAppendS("\\\""); break;
    default: bufAppendC(c);
  }
}

void bufAppendS(const char *s)
{
  size_t len = strlen(s);
  size_t n;
  while (cur_ + len >= buf_size)
  {
    buf_size *= 2;
    resizeBuffer();
  }
  for(n = 0; n < len; n++)
  {
    buf_[cur_ + n] = s[n];
  }
  cur_ += len;
  buf_[cur_] = 0;
}

void bufAppendC(const char c)
{
  if (cur_ + 1 >= buf_size)
  {
    buf_size *= 2;
    resizeBuffer();
  }
  buf_[cur_] = c;
  cur_++;
  buf_[cur_] = 0;
}

void bufReset(void)
{
  cur_ = 0;
  buf_size = BUFFER_INITIAL;
  resizeBuffer();
  memset(buf_, 0, buf_size);
}

void resizeBuffer(void)
{
  char *temp = (char *) malloc(buf_size);
  if (!temp)
  {
    fprintf(stderr, "Error: Out of memory while attempting to grow buffer!\n");
    exit(1);
  }
  if (buf_)
  {
    strncpy(temp, buf_, buf_size);
    free(buf_);
  }
  buf_ = temp;
}
