#ifndef GUARD_AST_NODE__

#include <stdlib.h>

typedef union {
    int ival;
    double dval;
    char *sval;
} data_type;

typedef struct ast_node {
    char type;
    char nary;
    struct ast_node *children;
    data_type data;
} ast_node;

typedef enum {
    NODE_ROOT,
} node_type;

static inline ast_node *ast_new_node(node_type type, data_type data)
{
    ast_node *root = malloc(sizeof(ast_node));

    if (!root)
        return NULL;

    root->type = type;
    root->data = data;

    return root;
}

#define GUARD_AST_NODE__
#endif
