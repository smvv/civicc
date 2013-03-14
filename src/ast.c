#include "ast.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static const char *ast_node_type_names[] = {
    "root",
    "func_head",
    "global_dec",
    "global_def",
};

static const char *ast_node_flag_names[] = {
    "",
    "extern",
    "export",
    "void",
    "bool",
    "int",
    "float",
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

const char *ast_node_type_name(ast_node_type type)
{
    return ast_node_type_names[type];
}

const char *ast_node_flag_name(ast_node_flag flag)
{
    return ast_node_flag_names[flag];
}

const char *ast_op_type_name(ast_op_type type)
{
    return ast_op_type_names[type];
}

char *ast_node_format(ast_node *node)
{
    char *buf = malloc(256 * sizeof(char));

    if (!buf)
        return NULL;

    printf("type: %d (flag: %d) -> ", node->type, node->type >> 4);
    fflush(stdout);

    const char *msg = ast_node_flag_name(node->type >> 4);
    int i = strlen(msg);

    strncpy(buf, msg, i);

    if (i) {
        buf[i] = ' ';
        i++;
    }

    msg = ast_node_type_name(node->type & 0x7);
    int j = strlen(msg);

    strncpy(buf + i, msg, j);

    buf[i + j] = 0;

    assert(i + j < 256);

    return buf;
}

void ast_node_print(ast_node *node)
{
    char *msg = ast_node_format(node);

    if (!msg)
        return;

    printf("%s", msg);

    free(msg);
}

ast_node *ast_new_node(ast_node_type type, ast_data_type data)
{
    ast_node *node = malloc(sizeof(ast_node));

    if (!node)
        return NULL;

    node->type = type;
    node->data = data;
    node->children = NULL;

    printf("new: ");
    ast_node_print(node);
    printf("\n");

    return node;
}

ast_node *ast_node_append(ast_node *parent, ast_node *child)
{
    if (!parent)
        return NULL;

    printf("append to: ");
    ast_node_print(parent);
    printf("\n");

    if (!child)
        return parent;

    if (!parent->nary || (parent->nary) % (AST_NODE_BUFFER_SIZE) == 0) {
        parent->children = realloc(parent->children, (parent->nary +
                    (AST_NODE_BUFFER_SIZE)) * sizeof(ast_node *));

        if (parent->children)
            return NULL;
    }

    parent->children[parent->nary++] = child;

    return parent;
}

ast_node *ast_flag_set(ast_node *node, unsigned int type)
{
    if (!node)
        return NULL;

    node->type |= type;

    return node;
}

