#ifndef GUARD_AST_NODE__

#include <stdlib.h>

typedef union {
    int ival;
    double dval;
    char *sval;
} ast_data_type;

typedef struct ast_node {
    char type;
    char nary;
    struct ast_node *children;
    ast_data_type data;
} ast_node;

typedef enum {
    NODE_ROOT,
} node_type;

typedef enum {
    TYPE_UNKNOWN,
    TYPE_VOID,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_FLOAT,
} ast_ident_type;

typedef enum {
    OP_NEG,
    OP_NOT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_LE,
    OP_LT,
    OP_GE,
    OP_GT,
    OP_EQ,
    OP_NE,
    OP_AND,
    OP_OR,
    OP_LAND,
    OP_LOR,
} ast_op_type;

static inline ast_node *ast_new_node(node_type type, ast_data_type data)
{
    ast_node *root = malloc(sizeof(ast_node));

    if (!root)
        return NULL;

    root->type = type;
    root->data = data;

    return root;
}

const char *ast_ident_type_name(ast_ident_type type);
const char *ast_op_type_name(ast_op_type type);

#define GUARD_AST_NODE__
#endif
