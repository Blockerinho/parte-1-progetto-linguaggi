#ifndef ABSYN_HEADER
#define ABSYN_HEADER

/* Tipo di dato per mantenere i valori dei campi. */

typedef struct Value_ *Value;
struct Value_
{
  enum { is_ValueInt, is_ValueBool, is_ValueString, is_ValueLocal, is_ValueNonLocal} kind;
  union {
    int value_int;
    int value_bool;
    char* value_string;
    char* value_local;
    struct { char* section_name; char* field_name; } value_nonlocal;
  };
};

Value make_ValueInt(int val);
Value make_ValueBool(int val);
Value make_ValueString(char* val);
Value make_ValueLocal(char* field_name);
Value make_ValueNonLocal(char* section_name, char* field_name);

void free_Value(Value p);

#endif
