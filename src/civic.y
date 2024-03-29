%code top {
#include <stdio.h>

#include "ast.h"
#include "ast_printer.h"
#include <string.h>

extern FILE *yyin;
extern char *yytext;

#define APPEND(parent, child) (ast_node_append(parent, child))
#define MARK(node, flag) (ast_flag_set(node, NODE_FLAG_##flag))
#define TYPE(node, type) (ast_flag_set(node, type))
#define NEW(type, data) (ast_new_node(NODE_##type, data))

#define STR(data) ((ast_data_type){.sval = data})
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
                    ((return) ? MARK( \
                    NEW(FN_BODY, (ast_data_type){.sval = NULL}) \
                    , RETURN) : \
                    NEW(FN_BODY, (ast_data_type){.sval = NULL})), \
                    vars), \
                defs), \
            stmts), \
        return)

#undef NEW_FOR
#undef NEW_BOOL
#undef NEW_INT
#undef NEW_FLOAT
#undef NEW_IDENT

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

#define NEW_BOOL(data) MARK(NEW(CONST, (ast_data_type){.ival = data}), BOOL)
#define NEW_INT(data) MARK(NEW(CONST, (ast_data_type){.ival = data}), INT)
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
%token TEQ TNE TLT TLE TGT TGE TADD TSUB TMUL TDIV TMOD
%token TOPAR TCPAR TOCB TCCB TSEMI TCOMMA TASSIGN

// Operator precedence for mathematical operators
%right TASSIGN
%left TLOR
%left TLAND
%left TOR
%left TAND
%left TLT TLE TGT TGE TNE TEQ
%left TADD TSUB
%left TMUL TMOD TDIV
%right TNEG TNOT TCAST
%nonassoc TIF
%nonassoc TELSE

%start program

%type <node> decl func_dec func_def func_header func_body func_params
%type <node> param global_dec global_def expr
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

func_dec : TEXTERN func_header TSEMI
           { MARK($2, EXTERN); $$ = $2; }
         ;

func_def : func_header TOCB func_body TCCB
           { $$ = APPEND($1, $3); }
         | TEXPORT func_header TOCB func_body TCCB
           { MARK($2, EXPORT); $$ = APPEND($2, $4); }
         ;

func_header : TVOID_TYPE TIDENT TOPAR TCPAR
              { $$ = APPEND(MARK(NEW(FN_HEAD, STR($2)), VOID),
                            NEW(BLOCK, NODE(NULL))); }
            | TVOID_TYPE TIDENT TOPAR func_params TCPAR
              { $$ = APPEND(MARK(NEW(FN_HEAD, STR($2)), VOID), $4); }
            | type TIDENT TOPAR TCPAR
              { $$ = APPEND(TYPE(NEW(FN_HEAD, STR($2)), $1),
                            NEW(BLOCK, NODE(NULL))); }
            | type TIDENT TOPAR func_params TCPAR
              { $$ = APPEND(TYPE(NEW(FN_HEAD, STR($2)), $1), $4); }
            ;

func_params : param { $$ = APPEND(NEW(BLOCK, NODE(NULL)), $1); }
            | func_params TCOMMA param { $$ = APPEND($1, $3); }
            ;

global_dec : TEXTERN type TIDENT TSEMI
             { $$ = MARK(TYPE(NEW(VAR_DEC, STR($3)), $2), EXTERN); }
           ;

global_def : type TIDENT TSEMI
             { $$ = TYPE(NEW(VAR_DEC, STR($2)), $1); }
           | type TIDENT TASSIGN expr TSEMI
             { $$ = TYPE(NEW(VAR_DEF, STR($2)), $1); APPEND($$, $4); }
           | TEXPORT type TIDENT TSEMI
             { $$ = MARK(TYPE(NEW(VAR_DEC, STR($3)), $2), EXPORT); }
           | TEXPORT type TIDENT TASSIGN expr TSEMI
             { $$ = MARK(TYPE(NEW(VAR_DEF, STR($3)), $2), EXPORT);
               APPEND($$, $5); }
           ;

type : TINT_TYPE { $$ = NODE_FLAG_INT; }
     | TFLOAT_TYPE { $$ = NODE_FLAG_FLOAT; }
     | TBOOL_TYPE { $$ = NODE_FLAG_BOOL; }
     ;

param : type TIDENT { $$ = TYPE(NEW(PARAM, STR($2)), $1); } ;

/* --- Syntax of CiviC statement language ---------------------------------- */

local_func_def : func_header TOCB func_body TCCB
                 { $$ = APPEND($1, $3); }
               ;

local_func_defs : /* empty */ { $$ = NEW(BLOCK, NODE(NULL)); }
                | local_func_def local_func_defs { $$ = APPEND($2, $1); }
                ;

func_body : var_decs local_func_defs statements
            { $$ = NEW_FN_BODY($1, $2, $3, NULL); }
          | var_decs local_func_defs statements TRETURN expr TSEMI
            { $$ = NEW_FN_BODY($1, $2, $3, $5); }
          ;

var_decs : /* empty */ { $$ = NEW(BLOCK, NODE(NULL)); }
         | var_decs var_dec { $$ = APPEND($1, $2); }
         ;

var_dec : type TIDENT TSEMI
          { $$ = TYPE(NEW(VAR_DEC, STR($2)), $1); }
        |  type TIDENT TASSIGN expr TSEMI
          { $$ = APPEND(TYPE(NEW(VAR_DEF, STR($2)), $1), $4); }
        ;

statements : /* empty */ { $$ = NEW(BLOCK, NODE(NULL)); }
           | statements statement { $$ = APPEND($1, $2); }
           ;

statement : TIDENT TASSIGN expr TSEMI
            { $$ = APPEND(NEW(ASSIGN, STR($1)), $3); }
          | TIDENT TOPAR expr_list TCPAR TSEMI
            { $$ = APPEND(NEW(CALL, STR($1)), $3); }
          | TIF TOPAR expr TCPAR block %prec TIF
            { $$ = APPEND(APPEND(NEW(IF, NODE(NULL)), $3), $5); }
          | TIF TOPAR expr TCPAR block TELSE block %prec TELSE
            { $$ = APPEND(APPEND(APPEND(NEW(IF, NODE(NULL)), $3), $5), $7); }
          | TWHILE TOPAR expr TCPAR block
            { $$ = APPEND(APPEND(NEW(WHILE, NODE(NULL)), $3), $5); }
          | TDO block TWHILE TOPAR expr TCPAR TSEMI
            { $$ = APPEND(APPEND(NEW(DO_WHILE, NODE(NULL)), $5), $2); }
          | TFOR TOPAR TINT_TYPE TIDENT TASSIGN expr TCOMMA expr TCPAR block
            { $$ = APPEND(NEW_FOR($4, $6, $8, NULL), $10); }
          | TFOR TOPAR TINT_TYPE TIDENT TASSIGN expr TCOMMA expr TCOMMA expr TCPAR block
            { $$ = APPEND(NEW_FOR($4, $6, $8, $10), $12); }
          ;

block : TOCB statements TCCB { $$ = $2; }
      | statement { $$ = APPEND(NEW(BLOCK, NODE(NULL)), $1); }
      ;

/* --- Syntax of CiviC expression language --------------------------------- */

expr : TOPAR expr TCPAR { $$ = $2; }
     | TSUB expr %prec TNEG { $$ = UNARY_OP(NEG, $2); }
     | TNOT expr { $$ = UNARY_OP(NOT, $2); }
     | expr TADD expr { $$ = BINARY_OP(ADD, $1, $3); }
     | expr TSUB expr { $$ = BINARY_OP(SUB, $1, $3); }
     | expr TMUL expr { $$ = BINARY_OP(MUL, $1, $3); }
     | expr TDIV expr { $$ = BINARY_OP(DIV, $1, $3); }
     | expr TMOD expr { $$ = BINARY_OP(MOD, $1, $3); }
     | expr TEQ expr { $$ = BINARY_OP(EQ, $1, $3); }
     | expr TNE expr { $$ = BINARY_OP(NE, $1, $3); }
     | expr TLT expr { $$ = BINARY_OP(LT, $1, $3); }
     | expr TLE expr { $$ = BINARY_OP(LE, $1, $3); }
     | expr TGT expr { $$ = BINARY_OP(GT, $1, $3); }
     | expr TGE expr { $$ = BINARY_OP(GE, $1, $3); }
     | expr TLAND expr { $$ = BINARY_OP(LAND, $1, $3); }
     | expr TLOR expr { $$ = BINARY_OP(LOR, $1, $3); }
     | expr TAND expr { $$ = BINARY_OP(AND, $1, $3); }
     | expr TOR expr { $$ = BINARY_OP(OR, $1, $3); }
     | TOPAR type TCPAR expr %prec TCAST { $$ = APPEND(NEW(CAST, INT($2)), $4); }
     | TIDENT TOPAR expr_list TCPAR { $$ = APPEND(NEW(CALL, STR($1)), $3); }
     | TIDENT { $$ = NEW_IDENT($1); }
     | const { $$ = $1; }
     ;

const : TTRUE { $$ = NEW_BOOL(1); }
      | TFALSE { $$ = NEW_BOOL(0); }
      | TINT { $$ = NEW_INT(yyval.i); }
      | TFLOAT { $$ = NEW_FLOAT(yyval.d); }
      ;

expr_list : /* empty */ { $$ = NEW(BLOCK, NODE(NULL)); }
          | expr { $$ = APPEND(NEW(BLOCK, NODE(NULL)), $1); }
          | expr_list TCOMMA expr { $$ = APPEND($1, $3); }
          ;

%%

void yyerror(ast_node *root, const char *msg) {
    fprintf(stderr, "%d:%d-%d: %s before \"%s\"\n", yylloc.first_line,
            yylloc.first_column, yylloc.last_column, msg, yytext);
    (void) root;
}
