#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "ast_helpers.h"
#include "ast_printer.h"

void ast_error(const char *msg, ast_node *node)
{
    ast_node *scope_node = find_func_head(node);
    size_t buflen = 256;
    char *buf = malloc(buflen * sizeof(char));

    fprintf(stderr, "\x1b[1;31merror:\x1b[0m ");
    ast_node_format(node, buf, buflen);
    fprintf(stderr, msg, buf);

    if (scope_node) {
        ast_node_format(scope_node, buf, buflen);
        fprintf(stderr, " in: `%s'.\n", buf);
    } else
        fprintf(stderr, " in global scope.\n");

    free(buf);
}


ast_node *create_global_init(ast_node *root)
{
    size_t i;

    if (!root)
        return NULL;

    // Try to find the global function __init.
    for (i = 0; i < root->nary; i++) {
        if (AST_NODE_TYPE(root->children[i]) == NODE_FN_HEAD
                && strncmp(root->children[i]->data.sval, "__init", 7) == 0) {
            assert(root->children[i]->nary == 1);
            return root->children[i]->children[0];
        }
    }

    // Create the new function __init to initialise the global vars.
    ast_node *__init_head = ast_new_node(NODE_FN_HEAD,
            (ast_data_type){.sval = strdup("__init")});

    if (!__init_head)
        return NULL;

    ast_flag_set(__init_head, NODE_FLAG_VOID);

    ast_node_append(root, __init_head);

    // Create an empty node block which contains the function arguments.
    ast_node_append(__init_head, ast_new_node(NODE_BLOCK,
            (ast_data_type){.sval = NULL}));

    // Create the function body with three node blocks.
    ast_node *__init = ast_new_node(NODE_FN_BODY,
            (ast_data_type){.nval = NULL});

    if (!__init)
        return NULL;

    for (i = 0; i < 3; i++) {
        ast_node_append(__init, ast_new_node(NODE_BLOCK,
                (ast_data_type){.sval = NULL}));
    }

    ast_node_append(__init_head, __init);

    return __init;
}

ast_node *get_func_body_block(ast_node *fn_body, size_t b)
{
    size_t i;

    if (!fn_body)
        return NULL;

    assert(AST_NODE_TYPE(fn_body) == NODE_FN_BODY);
    assert(b <= 3);

    for (i = fn_body->nary; i < 3; i++) {
        ast_node_append(fn_body, ast_new_node(NODE_BLOCK,
                (ast_data_type){.sval = NULL}));
    }

    assert(fn_body->nary == 3 || fn_body->nary == 4);

    assert(AST_NODE_TYPE(fn_body->children[0]) == NODE_BLOCK);
    assert(AST_NODE_TYPE(fn_body->children[1]) == NODE_BLOCK);
    assert(AST_NODE_TYPE(fn_body->children[2]) == NODE_BLOCK);

    return fn_body->children[b];
}

ast_node *find_func_body(ast_node *node)
{
    if (!node)
        return NULL;

    for (; node->parent; node = node->parent)
        if (AST_NODE_TYPE(node) == NODE_FN_BODY)
            return node;

    return NULL;
}

ast_node *find_func_head(ast_node *node)
{
    if (!node)
        return NULL;

    for (; node->parent; node = node->parent)
        if (AST_NODE_TYPE(node) == NODE_FN_HEAD)
            return node;

    return NULL;
}

int ast_node_pos(ast_node *parent, ast_node *node)
{
    int i;

    for (i = 0; i < (int) parent->nary; i++)
        if (parent->children[i] == node)
            return i;

    return -1;
}
