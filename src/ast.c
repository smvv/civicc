#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <inttypes.h>

#include "ast.h"
#include "ast_printer.h"

static const char *ast_node_type_names[] = {
    "block",
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
    "extern export",
    "return",
    "extern return",
    "export return",
    "extern export return",
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
    flag >>= AST_NODE_TYPE_SHIFT;

    if (flag > sizeof(ast_node_type_names) / sizeof(char *)) {
        printf("node type (%d) > %zu", flag, sizeof(ast_node_type_names) /
                sizeof(char *));
        return "";
    }

    return ast_node_type_names[flag];
}

const char *ast_modifier_name(ast_modifier_flag flag)
{
    flag >>= AST_MODIFIER_SHIFT;

    if (flag > sizeof(ast_modifier_names) / sizeof(char *)) {
        printf("modifier (%d) > %zu", flag, sizeof(ast_modifier_names) /
                sizeof(char *));
        return "";
    }

    return ast_modifier_names[flag];
}

const char *ast_data_type_name(ast_data_type_flag flag)
{
    flag >>= AST_DATA_TYPE_SHIFT;

    if (flag > sizeof(ast_data_type_names) / sizeof(char *)) {
        printf("node flag (%d) > %zu", flag, sizeof(ast_data_type_names) /
                sizeof(char *));
        return "";
    }

    return ast_data_type_names[flag];
}

const char *ast_op_type_name(ast_op_type type)
{
    return ast_op_type_names[type];
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

    if (node->data.sval) {
        if (AST_NODE_TYPE(node) == NODE_CONST && AST_DATA_TYPE(node) ==
                NODE_FLAG_IDENT)
            free(node->data.sval);
        else
            switch (AST_NODE_TYPE(node)) {
                case NODE_FN_HEAD:
                case NODE_VAR_DEC:
                case NODE_VAR_DEF:
                case NODE_PARAM:
                case NODE_ASSIGN:
                case NODE_CALL:
                case NODE_FOR:
                    free(node->data.sval);
                break;
            }
    }

    free(node);
}

ast_node *ast_node_append(ast_node *parent, ast_node *child)
{
    if (!parent)
        return NULL;

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

ast_node *ast_node_remove(ast_node *parent, unsigned int index)
{
    assert(index < parent->nary);

    ast_node *child = parent->children[index];

    memmove(parent->children + index, parent->children + index + 1,
            parent->nary - index - 1);

    parent->nary--;

    return child;
}

ast_node *ast_flag_set(ast_node *node, unsigned int type)
{
    if (!node)
        return NULL;

    node->type |= type;

    return node;
}
