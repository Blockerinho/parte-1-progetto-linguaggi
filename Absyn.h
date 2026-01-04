#ifndef ABSYN_HEADER
#define ABSYN_HEADER

#define _POSIX_C_SOURCE 200809L
#include <stddef.h> 
#include <string.h> 

/* --- 1. DEFINIZIONI TIPI BASE --- */
typedef char* String;
typedef char* Ident;
typedef int Integer;
typedef int Boolean;
typedef double Double;
typedef char Char;

/* Definizioni per il printer (Forward declaration) */
typedef struct NonLocVar_ *NonLocVar; 
typedef struct Boolean_ *Boolean_ptr;

/* --- 2. FORWARD DECLARATIONS --- */
struct SourceFile_;
typedef struct SourceFile_ *SourceFile;
struct ListTopLevelTag_;
typedef struct ListTopLevelTag_ *ListTopLevelTag;
struct TopLevelTag_;
typedef struct TopLevelTag_ *TopLevelTag;
struct ListSubLevelTag_;
typedef struct ListSubLevelTag_ *ListSubLevelTag;
struct SubLevelTag_;
typedef struct SubLevelTag_ *SubLevelTag;

/* --- 3. DEFINIZIONE VALUE --- */
typedef struct Value_ *Value;
struct Value_
{
  enum { 
      is_ValueInt, is_ValueBool, is_ValueString, 
      is_ValueLocal, is_ValueNonLocal
  } kind;
  union {
    int value_int;
    int value_bool;
    char* value_string;
    char* value_local;
    struct { char* section_name; char* field_name; } value_nonlocal;
  };
  int line_number; 
};

/* --- 4. DEFINIZIONE NONLOCVAR (Aggiunta per Printer) --- */
struct NonLocVar_ {
    enum { is_SimpleNonLoc, is_NonLoc } kind;
    union {
        struct { Ident ident_; } simpleNonLoc_;
        struct { Ident ident_1; Ident ident_2; } nonLoc_;
    } u;
};

/* --- 4b. DEFINIZIONE BOOLEAN (per Printer) --- */
struct Boolean_ {
    enum { is_Boolean_true, is_Boolean_false } kind;
};

/* --- 5. STRUTTURE AST --- */
struct SourceFile_ {
  enum { is_MainFile } kind;
  union { struct { ListTopLevelTag listtopleveltag_; } mainFile_; } u;
};

SourceFile make_MainFile(ListTopLevelTag p0);

struct ListTopLevelTag_ {
  TopLevelTag topleveltag_;
  ListTopLevelTag listtopleveltag_;
};
ListTopLevelTag make_ListTopLevelTag(TopLevelTag p1, ListTopLevelTag p2);

struct TopLevelTag_ {
  enum { is_FileImportTag, is_SectionTag } kind;
  union {
    struct { String string_; } fileImportTag_;
    struct { Ident ident_; ListSubLevelTag listsubleveltag_; } sectionTag_;
  } u;
  int line_number; 
};
TopLevelTag make_FileImportTag(String p0, int reached_section, int line);
TopLevelTag make_SectionTag(Ident p0, ListSubLevelTag p1, int line);

struct ListSubLevelTag_ {
  SubLevelTag subleveltag_;
  ListSubLevelTag listsubleveltag_;
};
ListSubLevelTag make_ListSubLevelTag(SubLevelTag p1, ListSubLevelTag p2);

struct SubLevelTag_ {
  enum { is_FieldTag, is_InheritTag } kind;
  union {
    struct { Ident ident_; Value value_; } fieldTag_; 
    struct { Ident ident_; } inheritTag_;
  } u;
  int line_number; 
};
SubLevelTag make_FieldTag(Ident p0, Value p1, int line);
SubLevelTag make_InheritTag(Ident p0, int reached_field, int line);

/* --- 6. COSTRUTTORI VALUE --- */
Value make_ValueInt(int val);
Value make_ValueBool(int val);
Value make_ValueString(char* val);
Value make_ValueLocal(char* field_name);
Value make_ValueNonLocal(char* section_name, char* field_name);

/* --- 7. CLONING & FREE --- */
void free_SourceFile(SourceFile p);
void free_ListTopLevelTag(ListTopLevelTag p);
void free_TopLevelTag(TopLevelTag p);
void free_ListSubLevelTag(ListSubLevelTag p);
void free_SubLevelTag(SubLevelTag p);
void free_Value(Value p);

#endif