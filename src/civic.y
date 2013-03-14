%code top {
#include <stdio.h>

#include "ast.h"
#include <string.h>

extern FILE *yyin;
extern char *yytext;

//#define YYSTYPE ast_node*

#define APPEND(parent, child) (ast_node_append(parent, child))
#define MARK(node, flag) (ast_flag_set(node, NODE_FLAG_##flag))
#define TYPE(node, type) (ast_flag_set(node, type))
#define NEW(type, data) (ast_new_node(NODE_##type, data))

#define STR(data) ((ast_data_type){.sval = strdup(data)})
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

%parse-param {ast_node *root}
%error-verbose
%locations

%union {
    ast_node *node;
    char *str;
    unsigned int i;
}

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

%type <node> decl func_dec func_def func_header func_body func_params
%type <node> func_param_list param global_dec global_def assign_expr expr
%type <str> TIDENT
%type <i> type

%%

/* --- Top level syntax of CiviC programs ---------------------------------- */

program : decls ;


decls : /* empty */
      | decls decl
        { ast_node_append(root, $2); }
      ;

decl : func_dec
     | func_def
     | global_dec
     | global_def
     ;

func_dec : TEXTERN func_header ';'
           { MARK($2, EXTERN); $$ = $2; }
         ;

func_def : func_header '{' func_body '}'
           { $$ = APPEND($1, $3); }
         | TEXPORT func_header '{' func_body '}'
           { MARK($2, EXPORT); $$ = APPEND($2, $4); }
         ;

func_header : TVOID_TYPE TIDENT '(' func_params ')'
              { $$ = APPEND(MARK(NEW(FN_HEAD, STR($2)), VOID), $4); }
            | type TIDENT '(' func_params ')'
              { $$ = APPEND(TYPE(NEW(FN_HEAD, STR($2)), $1), $4); }
            ;

func_params : /* empty */ { $$ = NULL; }
            | func_params func_param_list { $$ = APPEND($2, $1); }
            ;

func_param_list : param
                | func_param_list ',' param { $$ = APPEND($1, $3); }
                ;

global_dec : TEXTERN type TIDENT ';'
             { $$ = MARK(TYPE(NEW(GBL_DEC, STR($3)), $2), EXTERN); }
           ;

global_def : type TIDENT assign_expr ';'
             { $$ = TYPE(NEW(GBL_DEF, STR($2)), $1); APPEND($$, $3); }
           | TEXPORT type TIDENT assign_expr ';'
             { $$ = MARK(TYPE(NEW(GBL_DEF, STR($3)), $2), EXPORT);
               APPEND($$, $4); }
           ;

assign_expr : /* empty */ { $$ = NULL; }
            | '=' expr { $$ = $2; }
            ;

type : TINT_TYPE { $$ = NODE_FLAG_INT; }
     | TFLOAT_TYPE { $$ = NODE_FLAG_FLOAT; }
     | TBOOL_TYPE { $$ = NODE_FLAG_BOOL; }
     ;

param : type TIDENT { $$ = TYPE(NEW(PARAM, STR($2)), $1); } ;

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
