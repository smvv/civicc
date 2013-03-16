%{
#include "ast.h"
#include "civic_parser.h"

int yycolumn = 0;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = yycolumn; \
    yylloc.last_column = yycolumn + yyleng - 1; \
    yycolumn += yyleng;

%}

%option nounput
%option noinput
%option noyywrap
%option yylineno

%%

[ \t]                  ;

\n                     yycolumn = 0;

bool                   return TBOOL_TYPE;
int                    return TINT_TYPE;
float                  return TFLOAT_TYPE;
void                   return TVOID_TYPE;

"false"                return TFALSE;
"true"                 return TTRUE;

"if"                   return TIF;
"else"                 return TELSE;

"for"                  return TFOR;
"do"                   return TDO;
"while"                return TWHILE;

"return"               return TRETURN;

"export"               return TEXPORT;
"extern"               return TEXTERN;

"=="                   return TEQ;
"!="                   return TNEQ;
"<"                    return TLESS;
"<="                   return TLEQ;
">"                    return TGREAT;
">="                   return TGEQ;
"!"                    return TNOT;

[-+*/%(){};=,]         return yytext[0];

[a-zA-Z_][a-zA-Z0-9_]* yylval.str = strdup(yytext); return TIDENT;
[0-9]+\.[0-9]*         yylval.d = atof(yytext); return TFLOAT;
[0-9]+                 yylval.i = atoi(yytext); return TINT;

.                      { printf("unknown char %c ignored.\n", yytext[0]); }

%%
