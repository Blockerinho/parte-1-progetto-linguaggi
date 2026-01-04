#ifndef PRINTER_HEADER
#define PRINTER_HEADER

#include "Absyn.h"
#include "ParserSupport.h"

/* Certain applications may improve performance by changing the buffer size */
#define BUFFER_INITIAL 2048
/* You may wish to change _L_PAREN or _R_PAREN */
#define _L_PAREN '('
#define _R_PAREN ')'

/* The following are simple heuristics for rendering terminals */
/* You may wish to change them */
void renderCC(Char c);
void renderCS(String s);
void indent(void);
void backup(void);
void onEmptyLine(void);
void removeTrailingSpaces(void);
void removeTrailingWhitespace(void);

char *printFromBindings(section_entry* bindings); 
int saveToFile(const char* filename, const char* content);

void ppSourceFile(SourceFile p, int i);
void ppListTopLevelTag(ListTopLevelTag p, int i);
void ppTopLevelTag(TopLevelTag p, int i);
void ppListSubLevelTag(ListSubLevelTag p, int i);
void ppSubLevelTag(SubLevelTag p, int i);
void ppValue(Value p, int i);
void ppBoolean(int boolean_val, int i);
void ppNonLocVar(NonLocVar p, int i);

char *showSourceFile(SourceFile p);

void shSourceFile(SourceFile p);
void shListTopLevelTag(ListTopLevelTag p);
void shTopLevelTag(TopLevelTag p);
void shListSubLevelTag(ListSubLevelTag p);
void shSubLevelTag(SubLevelTag p);
void shValue(Value p);
void shBoolean(int boolean_val);
void shNonLocVar(NonLocVar p);

void ppIdent(String s, int i);
void shIdent(String s);
void ppInteger(Integer n, int i);
void ppDouble(Double d, int i);
void ppChar(Char c, int i);
void ppString(String s, int i);
void shInteger(Integer n);
void shDouble(Double d);
void shChar(Char c);
void shString(String s);
void bufEscapeS(const char *s);
void bufEscapeC(const char c);
void bufAppendS(const char *s);
void bufAppendC(const char c);
void bufReset(void);
void resizeBuffer(void);
void flush_comments_up_to(int line_limit);
void print_inline_comments(int current_line);
void flush_remaining_comments(void);

#endif