/* A Bison parser, made by GNU Bison 2.3.  */

/* Skeleton interface for Bison's Yacc-like parsers in C

   Copyright (C) 1984, 1989, 1990, 2000, 2001, 2002, 2003, 2004, 2005, 2006
   Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     _ERROR_ = 258,
     _DOLLAR = 259,
     _DOT = 260,
     _SLASH = 261,
     _LT = 262,
     _EQ = 263,
     _GT = 264,
     _KW_false = 265,
     _KW_field = 266,
     _KW_import = 267,
     _KW_inherit = 268,
     _KW_name = 269,
     _KW_section = 270,
     _KW_true = 271,
     T_Ident = 272,
     _STRING_ = 273,
     _INTEGER_ = 274,
     _IDENT_ = 275
   };
#endif
/* Tokens.  */
#define _ERROR_ 258
#define _DOLLAR 259
#define _DOT 260
#define _SLASH 261
#define _LT 262
#define _EQ 263
#define _GT 264
#define _KW_false 265
#define _KW_field 266
#define _KW_import 267
#define _KW_inherit 268
#define _KW_name 269
#define _KW_section 270
#define _KW_true 271
#define T_Ident 272
#define _STRING_ 273
#define _INTEGER_ 274
#define _IDENT_ 275




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
#line 60 "pseudoXMLgrammatica.y"
{
  int    _int;
  char   _char;
  double _double;
  char*  _string;
  Value value_;
}
/* Line 1529 of yacc.c.  */
#line 97 "Parser.h"
	YYSTYPE;
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
# define YYSTYPE_IS_TRIVIAL 1
#endif



#if ! defined YYLTYPE && ! defined YYLTYPE_IS_DECLARED
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
} YYLTYPE;
# define yyltype YYLTYPE /* obsolescent; will be withdrawn */
# define YYLTYPE_IS_DECLARED 1
# define YYLTYPE_IS_TRIVIAL 1
#endif


