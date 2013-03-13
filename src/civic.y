%code top {
#include <stdio.h>

#include "ast.h"

extern FILE *yyin;
extern char *yytext;
}

%code requires {
#define YYLTYPE YYLTYPE
typedef struct YYLTYPE
{
  int first_line;
  int first_column;
  int last_line;
  int last_column;
  char *filename;
} YYLTYPE;

void yyerror(ast_node *root, const char *msg);
int yylex();
}

%parse-param {ast_node *node}
%error-verbose
%locations

//%union {
//    ast_node *node;
//    ast_block *block;
//    ast_block *block;
//}

%token TEXTERN TEXPORT TRETURN TFOR TDO TWHILE TIF TELSE TCAST
%token TBOOL_TYPE TVOID_TYPE TINT_TYPE TFLOAT_TYPE TINT TFLOAT TIDENT
%token TTRUE TFALSE TLOGIC_OR TLOGIC_AND TNOT
%token TEQ TNEQ TLESS TLEQ TGREAT TGEQ TPLUS TMIN TMUL TDIV TMOD

// Operator precedence for mathematical operators
%right '='
%left TLOGIC_OR
%left TLOGIC_AND
%left TOR
%left TAND
%left TLESS TLEQ TGREAT TGEQ TNEQ TEQ
%left TPLUS TMIN
%left TMUL TMOD TDIV
%right TUMIN TNOT TCAST
%nonassoc TIF
%nonassoc TELSE

%start program

%%

/* --- Top level syntax of CiviC programs ---------------------------------- */

program : decls ;

decls : /* empty */
      | decls decl
      ;

decl : func_dec
     | func_def
     | global_dec
     | global_def
     ;

func_dec : TEXTERN func_header ';' ;

func_def : func_header '{' func_body '}'
         | TEXPORT func_header '{' func_body '}'
         ;

func_header : TVOID_TYPE TIDENT '(' func_params ')'
            | type TIDENT '(' func_params ')'
            ;

func_params : /* empty */
            | func_params func_param_list
            ;

func_param_list : param
                | func_param_list ',' param
                ;

global_dec : TEXTERN type TIDENT ';' ;

global_def : type TIDENT assign_expr ';'
           | TEXPORT type TIDENT assign_expr ';'
           ;

assign_expr : /* empty */
            | '=' expr
            ;

type : TINT_TYPE
     | TFLOAT_TYPE
     | TBOOL_TYPE
     ;

param : type TIDENT ;

/* --- Syntax of CiviC statement language ---------------------------------- */

local_func_def : func_header '{' func_body '}' ;

local_func_defs : /* empty */
                | local_func_def local_func_defs
                ;

func_body : var_decs local_func_defs statements
          | var_decs local_func_defs statements TRETURN expr ';' ;


var_decs : /* empty */
         | var_decs var_dec
         ;

var_dec : type TIDENT assign_expr ';' ;

statements : /* empty */
           | statements statement
           ;

statement : TIDENT '=' expr ';'
          | TIDENT '(' expr_list ')' ';'
          | TIF '(' expr ')' block %prec TIF
          | TIF '(' expr ')' block TELSE block %prec TELSE
          | TWHILE '(' expr ')' block
          | TDO block TWHILE '(' expr ')' ';'
          | TFOR '(' TINT_TYPE TIDENT '=' expr ',' expr ')' block
          | TFOR '(' TINT_TYPE TIDENT '=' expr ',' expr ',' expr ')' block
          ;

block : '{' statements '}'
      | statement
      ;

/* --- Syntax of CiviC expression language --------------------------------- */

expr : '(' expr ')'
     | TMIN expr %prec TUMIN
     | TNOT expr
     | expr TPLUS expr
     | expr TMIN expr
     | expr TMUL expr
     | expr TDIV expr
     | expr TMOD expr
     | expr TEQ expr
     | expr TNEQ expr
     | expr TLESS expr
     | expr TLEQ expr
     | expr TGREAT expr
     | expr TGEQ expr
     | expr TLOGIC_AND expr
     | expr TLOGIC_OR expr
     | expr TAND expr
     | expr TOR expr
     | '(' type ')' expr %prec TCAST
     | TIDENT '(' expr_list ')'
     | TIDENT
     | const
     ;

const : TTRUE
      | TFALSE
      | TINT
      | TFLOAT
      ;

expr_list : /* empty */
          | expr
          | expr_list ',' expr
          ;

%%

void yyerror(ast_node *root, const char *msg) {
    fprintf(stderr, "%d:%d-%d: %s before \"%s\"\n", yylloc.first_line,
            yylloc.first_column, yylloc.last_column, msg, yytext);
    (void) root;
}
