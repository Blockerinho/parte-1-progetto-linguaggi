#include <ctype.h>   /* isspace */
#include <stddef.h>  /* size_t */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "PseudoXMLParserSupport.h"
#include "comment_tracker.h"

#define INDENT_WIDTH 2
#define BUFFER_INITIAL 2000

int _n_;
char *buf_;
size_t cur_;
size_t buf_size;

void bufAppendS(const char *s);
void bufAppendC(const char c);
void bufEscapeS(const char *s);
void bufEscapeC(const char c);
void bufReset(void);
void resizeBuffer(void);
void indent(void);
void backup(void);
void removeTrailingSpaces(void);
void removeTrailingWhitespace(void);
void onEmptyLine(void);

/* Struttura per tracciare linee già stampate (evita duplicati commenti) */
typedef struct {
    int* lines;
    int count;
    int capacity;
} printed_lines;

static printed_lines plines = {NULL, 0, 0};

static void init_printed_lines(void) {
    if (plines.lines) free(plines.lines);
    plines.capacity = 100;
    plines.lines = (int*)malloc(plines.capacity * sizeof(int));
    plines.count = 0;
}

static int is_line_printed(int line) {
    for (int i = 0; i < plines.count; i++) {
        if (plines.lines[i] == line) return 1;
    }
    return 0;
}

static void mark_line_printed(int line) {
    if (plines.count >= plines.capacity) {
        plines.capacity *= 2;
        plines.lines = (int*)realloc(plines.lines, plines.capacity * sizeof(int));
    }
    plines.lines[plines.count++] = line;
}

/* --- Rendering Functions --- */

void renderC(char c)
{
  if (c == '{') 
  {
     onEmptyLine();
     bufAppendC(c);
     _n_ = _n_ + INDENT_WIDTH;
     bufAppendC('\n');
     indent();
  }
  else if (c == '(' || c == '[')
     bufAppendC(c);
  else if (c == ')' || c == ']')
  {
     removeTrailingWhitespace();
     bufAppendC(c);
     bufAppendC(' ');
  }
  else if (c == '<') 
  {
     onEmptyLine();
     bufAppendC(c);
  }
  else if (c == '>') 
  {
     bufAppendC(c);
  }
  else if (c == ' ') bufAppendC(c);
  else if (c == 0) return;
  else
  {
     bufAppendC(c);
     bufAppendC(' ');
  }
}

int allIsSpace(char *s)
{
  char c;
  while ((c = *s++))
    if (! isspace(c)) return 0;
  return 1;
}

void renderS(const char *s)
{
  if (*s) /* s[0] != '\0' */
  {
    bufAppendS(s);
  }
}

void indent(void)
{
  int n = _n_;
  while (--n >= 0)
    bufAppendC(' ');
}

void backup(void)
{
  if (cur_ && buf_[cur_ - 1] == ' ')
    buf_[--cur_] = 0;
}

void removeTrailingSpaces(void)
{
  while (cur_ && buf_[cur_ - 1] == ' ') --cur_;
  buf_[cur_] = 0;
}

void removeTrailingWhitespace(void)
{
  while (cur_ && (buf_[cur_ - 1] == ' ' || buf_[cur_ - 1] == '\n')) --cur_;
  buf_[cur_] = 0;
}

void onEmptyLine(void)
{
  removeTrailingSpaces();
  if (cur_ && buf_[cur_ - 1 ] != '\n') bufAppendC('\n');
  indent();
}

/* --- Comment Rendering --- */
void renderComments(int line_num, int inline_mode) {
    comment_entry* curr = global_comments;
    while (curr != NULL) {
        if (curr->line_number == line_num && !is_line_printed(line_num)) {
            
            /* Filtro per posizione: solo inline o solo standalone */
            int is_inline = (curr->position == COMMENT_INLINE);
            if (is_inline != inline_mode) {
                curr = curr->next;
                continue;
            }

            if (!inline_mode) {
                onEmptyLine();
                bufAppendC('#');
                bufAppendS(curr->text);
                /* Non marchiamo ancora la linea come stampata perché 
                   potrebbe esserci anche un commento inline dopo */
            } else {
                bufAppendC(' ');
                bufAppendC('#');
                bufAppendS(curr->text);
                mark_line_printed(line_num); /* Ora è completa */
            }
        }
        curr = curr->next;
    }
}

/* Helper struct for inherit deduplication */
typedef struct printed_sec_node {
    char* name;
    struct printed_sec_node* next;
} printed_sec_node;

static int is_sec_printed(printed_sec_node* list, char* name) {
    while (list) { if (strcmp(list->name, name) == 0) return 1; list = list->next; }
    return 0;
}

static void add_printed_sec(printed_sec_node** list, char* name) {
    printed_sec_node* n = malloc(sizeof(printed_sec_node));
    n->name = name; n->next = *list; *list = n;
}

static void free_printed_sec(printed_sec_node* list) {
    while(list) { printed_sec_node* t = list; list = list->next; free(t); }
}

void ppSectionEntry(section_entry* p, int _i_, int* current_line);
void ppFieldEntry(field_entry* p, int _i_, int* current_line);
void ppValue(field_entry* p, int _i_);

/* Entry point principale */
void pretty_print(FILE* out, section_entry* sections)
{
  _n_ = 0;
  bufReset();
  init_printed_lines();
  
  int current_line = 1;

  section_entry* p = sections;
  while(p)
  {
      ppSectionEntry(p, 0, &current_line);
      p = p->next;
  }

  /* Commenti finali orfani */
  comment_entry* curr = global_comments;
  while (curr != NULL) {
      if (!is_line_printed(curr->line_number)) {
          onEmptyLine();
          bufAppendC('#');
          bufAppendS(curr->text);
      }
      curr = curr->next;
  }
  bufAppendC('\n');

  /* Output */
  if (out) fprintf(out, "%s", buf_);
  if (plines.lines) { free(plines.lines); plines.lines = NULL; }
  if (buf_) { free(buf_); buf_ = NULL; }
}

void ppSectionEntry(section_entry* p, int _i_, int* current_line)
{
    /* Commenti pre-sezione */
    renderComments(*current_line, 0);

    /* <section name="..."> */
    onEmptyLine();
    bufAppendS("<section");
    bufAppendC(' ');
    bufAppendS("name");
    bufAppendC('=');
    bufAppendC('\"');
    bufEscapeS(p->name);
    bufAppendC('\"');
    bufAppendC('>');

    /* Commenti inline */
    renderComments(*current_line, 1);
    (*current_line)++;

    _n_ += INDENT_WIDTH;

    /* Handle Inherits (Deduplicazione) */
    printed_sec_node* printed_inherits = NULL;
    field_entry* f = p->fields;
    while(f) {
        if (f->kind == is_Inherited && f->inherit_from) {
            char* src = f->inherit_from->section->name;
            if (!is_sec_printed(printed_inherits, src)) {
                renderComments(*current_line, 0);
                onEmptyLine();
                bufAppendS("<inherit>");
                bufAppendS(src);
                bufAppendS("</inherit>");
                renderComments(*current_line, 1);
                (*current_line)++;
                add_printed_sec(&printed_inherits, src);
            }
        }
        f = f->next;
    }
    free_printed_sec(printed_inherits);

    /* Handle Fields */
    f = p->fields;
    while(f) {
        if (f->kind != is_Inherited) {
            ppFieldEntry(f, 0, current_line);
        }
        f = f->next;
    }

    _n_ -= INDENT_WIDTH;

    /* </section> */
    renderComments(*current_line, 0);
    onEmptyLine();
    bufAppendS("</section>");
    renderComments(*current_line, 1);
    (*current_line)++;
}

void ppFieldEntry(field_entry* p, int _i_, int* current_line)
{
    renderComments(*current_line, 0);
    onEmptyLine();
    
    /* <field name="..."> */
    bufAppendS("<field");
    bufAppendC(' ');
    bufAppendS("name");
    bufAppendC('=');
    bufAppendC('\"');
    bufEscapeS(p->field_name);
    bufAppendC('\"');
    bufAppendC('>');

    /* Value */
    ppValue(p, 0);

    /* </field> */
    bufAppendS("</field>");
    
    renderComments(*current_line, 1);
    (*current_line)++;
}

void ppValue(field_entry* p, int _i_)
{
    char tmp[64];
    switch(p->kind) {
        case is_Integer:
            sprintf(tmp, "%d", p->value_Integer);
            bufAppendS(tmp);
            break;
        case is_Boolean:
            if (p->value_Boolean == is_Boolean_true) bufAppendS("true");
            else bufAppendS("false");
            break;
        case is_String:
            bufAppendC('\"');
            bufEscapeS(p->value_String);
            bufAppendC('\"');
            break;
        case is_Local:
            bufAppendC('$');
            if (p->copy_from) bufAppendS(p->copy_from->field_name);
            else bufAppendS("UNKNOWN");
            break;
        case is_NonLocal:
            bufAppendC('$');
            if (p->copy_from && p->copy_from->section) {
                bufAppendS(p->copy_from->section->name);
                bufAppendC('.');
                bufAppendS(p->copy_from->field_name);
            } else {
                bufAppendS("UNKNOWN.UNKNOWN");
            }
            break;
        default: break;
    }
}

/* --- Standard BNFC Buffer Implementation --- */

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