#include "ast.h"

static const char *ast_ident_type_names[] = {
    "?",
    "void",
    "bool",
    "int",
    "float",
    "bool[]",
    "int[]",
    "float[]",
};

static const char *ast_op_type_names[] = {
    "-",
    "!",
    "+",
    "-",
    "*",
    "/",
    "%",
    "<=",
    "<",
    ">=",
    ">",
    "==",
    "!==",
    "&",
    "|",
    "&&",
    "||",
};

const char *ast_ident_type_name(ast_ident_type type)
{
    return ast_ident_type_names[type];
}

const char *ast_op_type_name(ast_op_type type)
{
    return ast_op_type_names[type];
}
