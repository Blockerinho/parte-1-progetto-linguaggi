#ifndef PRINTER_HEADER
#define PRINTER_HEADER

#include "ParserSupport.h"

/* Certain applications may improve performance by changing the buffer size */
#define BUFFER_INITIAL 2048
/* You may wish to change _L_PAREN or _R_PAREN */
#define _L_PAREN '('
#define _R_PAREN ')'

void renderCC(char c);
void renderCS(char* s);
void indent(void);
void backup(void);
void onEmptyLine(void);
void removeTrailingSpaces(void);
void removeTrailingWhitespace(void);

char *printFromBindings(section_entry* bindings); 
int saveToFile(const char* filename, const char* content);

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
