#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "phases.h"
#include "ast.h"
#include "ast_helpers.h"
#include "ast_printer.h"

#define SCOPE_LEVEL_LIMIT 42

static ast_node *scope_contains_ident(node_stack *scope, ast_node *node)
{
    size_t i;

    assert(scope);

    for (i = scope->items; i > 0; i--)
        if (strcmp(scope->data[i - 1]->data.sval, node->data.sval) == 0)
            return scope->data[i - 1];

    ast_error("missing definition of identifier: `%s'", node);

    return NULL;
}

#if 0
static unsigned int type_check_ident(node_stack *scope, ast_node *node)
{
    if (AST_NODE_TYPE(node) == NODE_ASSIGN)
        return 0;

    //if (AST_DATA_TYPE(def_node) != AST_DATA_TYPE(node)) {
    //    fprintf(stderr, "error: data type mismatch: `%s' is of type `%s' but"
    //            " should be of type `%s'\n", ident,
    //            ast_data_type_name(AST_DATA_TYPE(node)),
    //            ast_data_type_name(AST_DATA_TYPE(def_node)));
    //    return 1;
    //}

    return 0;
}
#endif

static unsigned int scope_level(ast_node *node)
{
    size_t i = 0;

    if (!node)
        return 0;

    // The return value of a function is sometimes not contained in a
    // NODE_BLOCK node. Therefore, check if the function has a return value and
    // that the current node is the return node.
    if (AST_NODE_TYPE(node->parent) == NODE_FN_BODY &&
            node->parent->nary == 4 && node->parent->children[3] == node)
        i++;

    for (; node->parent; node = node->parent)
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

static unsigned int detect_duplicated_var(ast_node **stack_data, size_t
        stack_size, ast_node *node)
{
    size_t i;

    if (!stack_data || !node)
        return 0;

    for (i = 0; i < stack_size; i++)
        if (strcmp(stack_data[i]->data.sval, node->data.sval) == 0)
            return 1;

    return 0;
}

static unsigned int add_scope_vars(node_stack **scopes, size_t scope, ast_node
        *node)
{
    size_t i;
    unsigned int error = 0;

    node_stack *prev_scope;
    size_t prev_items;
    size_t new_items;

    if (!scopes || !node)
        return 1;

    prev_scope = scopes[scope - 1];
    prev_items = prev_scope->items;
    new_items = scopes[scope]->items - prev_items;

    for (i = 0; i < node->nary; i++) {
        if (AST_NODE_TYPE(node->children[i]) == NODE_VAR_DEC) {
            if (!detect_duplicated_var(scopes[scope]->data + prev_items,
                        new_items, node->children[i])) {
                node_stack_push(scopes[scope], node->children[i]);
                new_items++;
                assert(scopes[scope]->items == new_items + prev_items);
            }
            else {
                ast_error("redeclaration of variable `%s'", node->children[i]);
                error = 1;
            }
        }
    }

    return error;
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

        if (scopes[scope])
            node_stack_free(scopes[scope]);

        scopes[scope] = node_stack_clone(scopes[scope - 1]);

        if (add_scope_vars(scopes, scope, vars_block))
            error = 1;
    } else if (AST_NODE_TYPE(node) == NODE_ASSIGN
               || (AST_NODE_TYPE(node) == NODE_CONST
                   && AST_DATA_TYPE(node) == NODE_FLAG_IDENT)) {
        ast_node *def_node;

        if (!(def_node = scope_contains_ident(get_scope(scopes, node), node)))
            error = 1;
        else {
            ast_node_print("def_node: `%s'", def_node);
            ast_node_print(" of node: `%s'\n", node);
        }
    }

    AST_TRAVERSE_END(root, node)

    for (i = 0; scopes[i]; i++)
        node_stack_free(scopes[i]);

    free(scopes);

    return error;
}

