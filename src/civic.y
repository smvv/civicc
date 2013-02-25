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

%token TEXTERN TEXPORT TRETURN TFOR TDO TWHILE TIF TELSE
%token TBOOL_TYPE TVOID_TYPE TINT_TYPE TFLOAT_TYPE TINT TFLOAT TIDENT
%token TTRUE TFALSE TLOGIC_OR TLOGIC_AND
%token TEQ TNEQ TLESS TLEQ TGREAT TGEQ TPLUS TMIN TMUL TDIV TMOD

// Operator precedence for mathematical operators
%right '='
%left TOR
%left TAND
%left TNOT
%left TLESS TLEQ TGREAT TGEQ TNEQ TEQ
%left TPLUS TMINUS
%left TMUL TMOD TDIV

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

//ret_type : TVOID_TYPE | type ;

global_dec : TEXTERN type TIDENT ';' ;

global_def : type TIDENT assign_expr ';'
           | TEXPORT type TIDENT assign_expr ';'
           ;

//export : /* empty */
//       | TEXPORT
//       ;

assign_expr : /* empty */ | '=' expr ;

type : TINT_TYPE | TFLOAT_TYPE | TBOOL_TYPE ;

param : type TIDENT ;

/* --- Syntax of CiviC statement language ---------------------------------- */

func_body : var_decs statements return ;

var_decs : /* empty */
         | var_decs var_dec
         ;

var_dec : type TIDENT assign_expr ';' ;

statements : /* empty */
           | statements statement
           ;

statement : TIDENT '=' expr ';'
          | TIDENT '(' expr_list ')' ';'
          | TIF '(' expr ')' block
          | TIF '(' expr ')' block TELSE block
          | TWHILE '(' expr ')' block
          | TDO block TWHILE '(' expr ')' ';'
          | TFOR '(' TINT_TYPE TIDENT '=' expr ',' expr ')' block
          | TFOR '(' TINT_TYPE TIDENT '=' expr ',' expr ',' expr ')' block
          ;

block : '{' statements '}'
      | statement
      ;

return : TRETURN expr ';' ;

/* --- Syntax of CiviC expression language --------------------------------- */

expr : '(' expr ')'
     | mon_op expr
     | expr bin_op expr
     | '(' type ')' expr
     | TIDENT '(' expr_list ')'
     | TIDENT
     | const
     ;

expr_list : /* empty */
          | expr
          | expr_list ',' expr
          ;

bin_op : arith_op
       | rel_op
       | logic_op
       ;

arith_op : TPLUS
         | TMIN
         | TMUL
         | TDIV
         | TMOD
         ;

rel_op : TEQ
       | TNEQ
       | TLESS
       | TLEQ
       | TGREAT
       | TGEQ
       ;

logic_op : TLOGIC_AND
         | TLOGIC_OR
         ;

mon_op : TMIN
       | '!'
       ;

const : bool_const
      | TINT
      | TFLOAT
      ;

bool_const : TTRUE
           | TFALSE
           ;

%%

void yyerror(ast_node *root, const char *msg) {
    fprintf(stderr, "%d:%d-%d: %s before \"%s\"\n", yylloc.first_line,
            yylloc.first_column, yylloc.last_column, msg, yytext);
    (void) root;
}
