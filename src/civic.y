%code top {
#include <stdio.h>

#include "ast.h"
#include <string.h>

extern FILE *yyin;
extern char *yytext;

#define APPEND(parent, child) (ast_node_append(parent, child))
#define MARK(node, flag) (ast_flag_set(node, NODE_FLAG_##flag))
#define TYPE(node, type) (ast_flag_set(node, type))
#define NEW(type, data) (ast_new_node(NODE_##type, data))

#define STR(data) ((ast_data_type){.sval = data ? strdup(data) : NULL})
#define INT(data) ((ast_data_type){.ival = data})
#define NODE(data) ((ast_data_type){.nval = data})

#define UNARY_OP(type, a) \
    (APPEND(NEW(UNARY_OP, INT(OP_##type)), a))

#define BINARY_OP(type, a, b) \
    (APPEND(APPEND(NEW(BIN_OP, INT(OP_##type)), a), b))

#define NEW_FN_BODY(vars, defs, stmts, return) \
    APPEND( \
        APPEND( \
            APPEND( \
                APPEND( \
                    NEW(FN_BODY, (ast_data_type){.sval = NULL}), \
                    vars), \
                defs), \
            stmts), \
        return)

#define NEW_FOR(ident, start, stop, step) \
    APPEND( \
        APPEND( \
            APPEND( \
                NEW(FOR, STR(ident)), \
                start \
            ), \
            stop \
        ), \
        step \
    )

#define NEW_INT(data) MARK(NEW(CONST, (ast_data_type){.ival = data}), INT)
#define NEW_BOOL(data) MARK(NEW(CONST, (ast_data_type){.ival = data}), BOOL)
#define NEW_FLOAT(data) MARK(NEW(CONST, (ast_data_type){.dval = data}), FLOAT)
#define NEW_IDENT(data) MARK(NEW(CONST, (ast_data_type){.sval = data}), IDENT)
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
    double d;
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
%type <node> var_decs local_func_defs statements local_func_def var_dec
%type <node> statement expr_list block const
%type <str> TIDENT
%type <i> type

%%

/* --- Top level syntax of CiviC programs ---------------------------------- */

program : decls ;


decls : /* empty */
      | decls decl
        { APPEND(root, $2); }
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
             { $$ = MARK(TYPE(NEW(VAR_DEC, STR($3)), $2), EXTERN); }
           ;

global_def : type TIDENT assign_expr ';'
             { $$ = TYPE(NEW(VAR_DEF, STR($2)), $1); APPEND($$, $3); }
           | TEXPORT type TIDENT assign_expr ';'
             { $$ = MARK(TYPE(NEW(VAR_DEF, STR($3)), $2), EXPORT);
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

local_func_defs : /* empty */ { $$ = NULL; }
                | local_func_def local_func_defs { $$ = APPEND($2, $1); }
                ;

func_body : var_decs local_func_defs statements
            { $$ = NEW_FN_BODY($1, $2, $3, NULL); }
          | var_decs local_func_defs statements TRETURN expr ';'
            { $$ = NEW_FN_BODY($1, $2, $3, $5); }
          ;

var_decs : /* empty */ { $$ = NULL; }
         | var_decs var_dec { $$ = APPEND($1, $2); }
         ;

var_dec : type TIDENT assign_expr ';'
          { $$ = APPEND(TYPE(NEW(VAR_DEC, STR($2)), $1), $3); }
        ;

statements : /* empty */ { $$ = NULL }
           | statements statement { $$ = APPEND($1, $2); }
           ;

statement : TIDENT '=' expr ';'
            { $$ = APPEND(NEW(ASSIGN, STR($1)), $3); }
          | TIDENT '(' expr_list ')' ';'
            { $$ = APPEND(NEW(CALL, STR($1)), $3); }
          | TIF '(' expr ')' block %prec TIF
            { $$ = APPEND(NEW(IF, NODE($3)), $5); }
          | TIF '(' expr ')' block TELSE block %prec TELSE
            { $$ = APPEND(APPEND(NEW(IF, NODE($3)), $5), $7); }
          | TWHILE '(' expr ')' block
            { $$ = APPEND(NEW(WHILE, NODE($3)), $5); }
          | TDO block TWHILE '(' expr ')' ';'
            { $$ = APPEND(NEW(DO_WHILE, NODE($2)), $5); }
          | TFOR '(' TINT_TYPE TIDENT '=' expr ',' expr ')' block
            { $$ = APPEND(NEW_FOR($4, $6, $8, NULL), $10); }
          | TFOR '(' TINT_TYPE TIDENT '=' expr ',' expr ',' expr ')' block
            { $$ = APPEND(NEW_FOR($4, $6, $8, $10), $12); }
          ;

block : '{' statements '}' { $$ = $2; }
      | statement { $$ = $1; }
      ;

/* --- Syntax of CiviC expression language --------------------------------- */

expr : '(' expr ')' { $$ = $2; }
     | TMIN expr %prec TUMIN { $$ = UNARY_OP(NEG, $2); }
     | TNOT expr { $$ = UNARY_OP(NOT, $2); }
     | expr TPLUS expr { $$ = BINARY_OP(ADD, $1, $3); }
     | expr TMIN expr { $$ = BINARY_OP(SUB, $1, $3); }
     | expr TMUL expr { $$ = BINARY_OP(MUL, $1, $3); }
     | expr TDIV expr { $$ = BINARY_OP(DIV, $1, $3); }
     | expr TMOD expr { $$ = BINARY_OP(MOD, $1, $3); }
     | expr TEQ expr { $$ = BINARY_OP(EQ, $1, $3); }
     | expr TNEQ expr { $$ = BINARY_OP(NE, $1, $3); }
     | expr TLESS expr { $$ = BINARY_OP(LT, $1, $3); }
     | expr TLEQ expr { $$ = BINARY_OP(LE, $1, $3); }
     | expr TGREAT expr { $$ = BINARY_OP(GT, $1, $3); }
     | expr TGEQ expr { $$ = BINARY_OP(GE, $1, $3); }
     | expr TLOGIC_AND expr { $$ = BINARY_OP(LAND, $1, $3); }
     | expr TLOGIC_OR expr { $$ = BINARY_OP(LOR, $1, $3); }
     | expr TAND expr { $$ = BINARY_OP(AND, $1, $3); }
     | expr TOR expr { $$ = BINARY_OP(OR, $1, $3); }
     | '(' type ')' expr %prec TCAST { $$ = APPEND(NEW(CAST, INT($2)), $4); }
     | TIDENT '(' expr_list ')' { $$ = APPEND(NEW(CALL, STR($1)), $3); }
     | TIDENT { $$ = NEW_IDENT($1); }
     | const { $$ = $1; }
     ;

const : TTRUE { $$ = NEW_BOOL(1); }
      | TFALSE { $$ = NEW_BOOL(0); }
      | TINT { $$ = NEW_INT(yyval.i); }
      | TFLOAT { $$ = NEW_FLOAT(yyval.d); }
      ;

expr_list : /* empty */ { $$ = NULL; }
          | expr { $$ = $1; }
          | expr_list ',' expr { $$ = APPEND($1, $3); }
          ;

%%

void yyerror(ast_node *root, const char *msg) {
    fprintf(stderr, "%d:%d-%d: %s before \"%s\"\n", yylloc.first_line,
            yylloc.first_column, yylloc.last_column, msg, yytext);
    (void) root;
}
