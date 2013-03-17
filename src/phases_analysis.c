#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "phases.h"
#include "ast.h"
#include "ast_helpers.h"
#include "ast_printer.h"

#define SCOPE_LEVEL_LIMIT 42

static ast_node *scope_contains_ident(node_stack *scope, char *ident,
        size_t ident_len)
{
    size_t i;

    if (!scope)
        return NULL;

    for (i = scope->items; i > 0; i--)
        if (strncmp(scope->data[i - 1]->data.sval, ident, ident_len + 1) == 0)
            return scope->data[i - 1];

    return NULL;
}

static ast_node *find_func_head(ast_node *node)
{
    if (!node)
        return NULL;

    for (; node->parent; node = node->parent)
        if (AST_NODE_TYPE(node) == NODE_FN_HEAD)
            return node;

    return NULL;
}

static unsigned int type_check_ident(node_stack *scope, ast_node *node)
{
    char *ident = node->data.sval;
    size_t ident_len = strlen(node->data.sval);

    ast_node *def_node = scope_contains_ident(scope, ident, ident_len);

    if (!def_node) {
        ast_node *scope_node = find_func_head(node);

        if (scope_node) {
            char *scope_msg = malloc(256 * sizeof(char));
            ast_node_format(scope_node, scope_msg, 256);
            fprintf(stderr, "error: missing definition of identifier: `%s'"
                            " used in: `%s'.\n", ident, scope_msg);
            free(scope_msg);
        } else
            fprintf(stderr, "error: missing definition of identifier: `%s'"
                            " used in global scope.\n", ident);

        return 1;
    }

    if (AST_NODE_TYPE(node) == NODE_ASSIGN)
        return 0;

    //if (AST_DATA_TYPE(def_node) != AST_DATA_TYPE(node)) {
    //    fprintf(stderr, "error: data type mismatch: `%s' is of type `%s' but"
    //            " should be of type `%s'\n", ident,
    //            ast_data_type_name(AST_DATA_TYPE(node)),
    //            ast_data_type_name(AST_DATA_TYPE(def_node)));
    //    return;
    //}

    return 0;
}

static unsigned int scope_level(ast_node *node)
{
    size_t i;

    if (!node)
        return 0;

    for (i = 0; node->parent; node = node->parent)
        if (AST_NODE_TYPE(node) == NODE_BLOCK)
            i++;

    assert(i < SCOPE_LEVEL_LIMIT);

    return i;
}

static node_stack *get_scope(node_stack **scopes, ast_node *node)
{
    unsigned int scope;

    scope = scope_level(node);

    // Find the last defined scope
    while (scope && !scopes[scope])
        scope--;

    return scopes[scope];
}

unsigned int pass_context_analysis(ast_node *root)
{
    unsigned int error = 0;
    unsigned int scope;
    size_t i;

    node_stack **scopes = calloc(SCOPE_LEVEL_LIMIT, sizeof(node_stack *));

    if (!root)
        return 0;

    // Construct a list of all variables defined in the global scope.
    scopes[0] = node_stack_new();

    for (i = 0; i < root->nary; i++)
        if (AST_NODE_TYPE(root->children[i]) == NODE_VAR_DEC)
            node_stack_push(scopes[0], root->children[i]);

    AST_TRAVERSE_START(root, node)

    if (AST_NODE_TYPE(node) == NODE_FN_BODY) {
        // Construct a list of all variables defined in the nested scope.
        ast_node *vars_block = get_func_body_block(node, NODE_BLOCK_VARS);

        if (!vars_block)
            return error;

        scope = scope_level(vars_block);

        assert(scope > 0);
        assert(scopes[scope - 1]);

        //ast_node_print("node: %s\n", node->parent);
        //ast_node_print("block: %s", vars_block);
        //printf("-> scope level: %d\n", scope);

        if (scopes[scope])
            node_stack_free(scopes[scope]);

        scopes[scope] = node_stack_clone(scopes[scope - 1]);

        for (i = 0; i < vars_block->nary; i++)
            if (AST_NODE_TYPE(vars_block->children[i]) == NODE_VAR_DEC)
                node_stack_push(scopes[scope], vars_block->children[i]);
    } else if (AST_NODE_TYPE(node) == NODE_ASSIGN
               || (AST_NODE_TYPE(node) == NODE_CONST
                   && AST_DATA_TYPE(node) == NODE_FLAG_IDENT)) {
        if(!type_check_ident(get_scope(scopes, node), node))
            error = 1;
    }

    AST_TRAVERSE_END(root, node)

    for (i = 0; scopes[i]; i++)
        node_stack_free(scopes[i]);

    free(scopes);

    return error;
}

