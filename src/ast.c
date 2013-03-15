#include "ast.h"
#include <stdio.h>
#include <string.h>
#include <assert.h>

static const char *ast_node_type_names[] = {
    "root",
    "func_head",
    "func_body",
    "var_dec",
    "var_def",
    "param",
    "assign",
    "call",
    "if",
    "while",
    "do_while",
    "for",
    "unary",
    "binary",
    "cast",
    "const",
};

static const char *ast_modifier_names[] = {
    "",
    "extern",
    "export",
};

static const char *ast_data_type_names[] = {
    "",
    "void",
    "bool",
    "int",
    "float",
    "ident",
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

const char *ast_node_type_name(ast_node_type_flag flag)
{
    if (flag > sizeof(ast_node_type_names) / sizeof(char *)) {
        printf("node type (%d) > %lu", flag, sizeof(ast_node_type_names) /
                sizeof(char *));
        return "";
    }

    return ast_node_type_names[flag];
}

const char *ast_modifier_name(ast_modifier_flag flag)
{
    if (flag > sizeof(ast_modifier_names) / sizeof(char *)) {
        printf("modifier (%d) > %lu", flag, sizeof(ast_modifier_names) /
                sizeof(char *));
        return "";
    }

    return ast_modifier_names[flag];
}

const char *ast_data_type_name(ast_data_type_flag flag)
{
    if (flag > sizeof(ast_data_type_names) / sizeof(char *)) {
        printf("node flag (%d) > %lu", flag, sizeof(ast_data_type_names) /
                sizeof(char *));
        return "";
    }

    return ast_data_type_names[flag];
}

const char *ast_op_type_name(ast_op_type type)
{
    return ast_op_type_names[type];
}

char *ast_node_format(ast_node *node)
{
    if (!node)
        return NULL;

    char *buf = malloc(256 * sizeof(char));

    if (!buf)
        return NULL;

    int i = 0, j;
    const char *msg;

    msg = ast_modifier_name((node->type >> 4) & 0x3);
    j = strlen(msg);

    strncpy(buf + i, msg, j);
    i += j;

    if (j) {
        buf[i] = ' ';
        i++;
    }

    msg = ast_data_type_name((node->type >> 6) & 0x7);
    j = strlen(msg);

    strncpy(buf + i, msg, j);
    i += j;

    if (i) {
        buf[i] = ' ';
        i++;
    }

    msg = ast_node_type_name(node->type & 0xf);
    j = strlen(msg);

    strncpy(buf + i, msg, j);
    i += j;

    buf[i] = 0;

    assert(i < 256);

    return buf;
}

void ast_node_print(ast_node *node)
{
    char *msg = ast_node_format(node);

    if (msg) {
        printf("[%s; c=%d]", msg, node->nary);
        free(msg);
    } else {
        printf("(nil)");
    }
}

ast_node *ast_new_node(ast_node_type_flag flag, ast_data_type data)
{
    ast_node *node = malloc(sizeof(ast_node));

    if (!node)
        return NULL;

    node->type = flag;
    node->data = data;
    node->nary = 0;
    node->children = NULL;

    printf("new: ");
    ast_node_print(node);
    printf("\n");

    return node;
}

void ast_free_node(ast_node *node)
{
    unsigned int i;

    if (!node)
        return;

    if (node->children) {
        for (i = 0; i < node->nary; i++)
            ast_free_node(node->children[i]);

        free(node->children);
    }

    if (node->data) {
        free(node->data);
    }

    free(node);
}

ast_node *ast_node_append(ast_node *parent, ast_node *child)
{
    if (!parent)
        return NULL;

    printf("append: ");
    ast_node_print(child);
    printf(" to: ");
    ast_node_print(parent);
    printf("\n");

    if (!child)
        return parent;

    if (!parent->nary || (parent->nary) % (AST_NODE_BUFFER_SIZE) == 0) {
        parent->children = realloc(parent->children, (parent->nary +
                    (AST_NODE_BUFFER_SIZE)) * sizeof(ast_node *));

        if (!parent->children)
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
