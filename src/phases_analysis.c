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

static unsigned int add_scope_node(node_stack **scopes, size_t scope, ast_node
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
        assert(AST_NODE_TYPE(node->children[i]) == NODE_PARAM
                || AST_NODE_TYPE(node->children[i]) == NODE_VAR_DEC
                || AST_NODE_TYPE(node->children[i]) == NODE_FN_HEAD);

        if (!detect_duplicated_var(scopes[scope]->data + prev_items,
                    new_items, node->children[i])) {
            node_stack_push(scopes[scope], node->children[i]);
            new_items++;
            assert(scopes[scope]->items == new_items + prev_items);
        }
        else {
            ast_error("redeclaration of variable `%s' in same scope",
                        node->children[i]);
            error = 1;
        }
    }

    return error;
}

static ast_data_type_flag node_type_inference(node_stack *scope, ast_node
        *node)
{
    (void) scope;

    ast_data_type_flag a, b;

    switch (AST_NODE_TYPE(node)) {
    case NODE_CONST:
        if (AST_DATA_TYPE(node) != NODE_FLAG_IDENT)
            return AST_DATA_TYPE(node);

        ast_node *def_node;

        if (!(def_node = scope_contains_ident(scope, node)))
            return 0;

        return AST_DATA_TYPE(def_node);
    case NODE_BIN_OP:
        a = node_type_inference(scope, node->children[0]);
        b = node_type_inference(scope, node->children[1]);

        if (!a || !b)
            return 0;

        if ((a != NODE_FLAG_INT && a != NODE_FLAG_FLOAT)
                || (b != NODE_FLAG_INT && b != NODE_FLAG_FLOAT)) {
            char *msg = malloc(256 * sizeof(char));
            snprintf(msg, 256, "operand type mismatch: `%%s' requires float"
                     " or int types but `%s' and `%s' were given",
                     ast_data_type_name(a), ast_data_type_name(b));
            ast_error(msg, node);
            free(msg);

            return 0;
        }

        // Implicit casting from int to float is not supported by CiviC.
        if (a != b) {
            char *msg = malloc(256 * sizeof(char));
            snprintf(msg, 256, "operand type mismatch: `%%s' requires "
                     " two similar types but `%s' and `%s' were given",
                     ast_data_type_name(a), ast_data_type_name(b));
            ast_error(msg, node);
            free(msg);

            return 0;
        }

        return a;
    default:
        ast_error("type inference got an unknown node type: `%s'", node);
    }

    return 0;
}

static unsigned int type_check_return_node(node_stack *scope, ast_node *node)
{
    assert(AST_NODE_TYPE(node) == NODE_FN_BODY);
    assert(node->parent && AST_NODE_TYPE(node->parent) == NODE_FN_HEAD);

    if (node->nary != 4)
        return 0;

    ast_data_type_flag def_type = AST_DATA_TYPE(node->parent);
    ast_data_type_flag node_type = node_type_inference(scope,
            node->children[3]);

    if (!def_type || !node_type)
        return 1;

    if (def_type != node_type) {
        char *msg = malloc(256 * sizeof(char));
        snprintf(msg, 256, "data type mismatch: `%%s' cannot return the"
                 " expression of type `%s'", ast_data_type_name(node_type));
        ast_error(msg, node->parent);
        free(msg);

        return 1;
    }

    return 0;
}

static unsigned int type_check_assign_node(node_stack *scope, ast_node *node,
        ast_node *def_node)
{
    if (AST_NODE_TYPE(def_node) == NODE_FN_HEAD) {
        ast_error("invalid assignment: cannot assign expression to function"
                  "`%s'", def_node);
        return 1;
    }

    assert(AST_NODE_TYPE(def_node) == NODE_VAR_DEC);
    assert(AST_NODE_TYPE(node) == NODE_ASSIGN);

    ast_data_type_flag def_type = AST_DATA_TYPE(def_node);
    ast_data_type_flag node_type = node_type_inference(scope,
            node->children[0]);

    if (!def_type || !node_type)
        return 1;

    if (def_type != node_type) {
        char *msg = malloc(256 * sizeof(char));
        snprintf(msg, 256, "data type mismatch: `%s %%s' cannot assign the"
                 " expression of type `%s'", ast_data_type_name(def_type),
                 ast_data_type_name(node_type));
        ast_error(msg, node);
        free(msg);

        return 1;
    }

    return 0;
}
static unsigned int type_check_call_node(node_stack *scope, ast_node *node,
        ast_node *def_node)
{
    unsigned int i;
    unsigned int error = 0;

    if (AST_NODE_TYPE(def_node) != NODE_FN_HEAD) {
        ast_error("invalid callee: cannot call variable `%s' as a function",
                  def_node);
        return 1;
    }

    assert(AST_NODE_TYPE(def_node) == NODE_FN_HEAD);
    assert(AST_NODE_TYPE(node) == NODE_CALL);

    ast_node *params = def_node->children[0];
    ast_node *arguments = node->children[0];

    assert(params && arguments);

    if (params->nary > arguments->nary) {
        ast_error("invalid function call: not enough arguments given for"
                  " function `%s'", node);
        return 1;
    }

    if (params->nary < arguments->nary) {
        ast_error("invalid function call: too much arguments given for"
                  " function `%s'", node);
        return 1;
    }

    for (i = 0; i < arguments->nary; i++) {
        ast_data_type_flag param_type = AST_DATA_TYPE(params->children[i]);
        ast_data_type_flag arg_type = node_type_inference(scope,
                arguments->children[i]);

        if (!param_type || !arg_type)
            error = 1;
        else if (param_type != arg_type) {
            char *msg = malloc(256 * sizeof(char));
            snprintf(msg, 256, "data type mismatch: argument %d is of type"
                    " `%s' but expected type `%s'", i,
                    ast_data_type_name(arg_type),
                    ast_data_type_name(param_type));
            ast_error(msg, node);
            free(msg);

            error = 1;
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

    scopes[0] = node_stack_new();

    // Construct a list of all variables defined in the global scope.
    for (i = 0; i < root->nary; i++)
        if (AST_NODE_TYPE(root->children[i]) == NODE_VAR_DEC
                || AST_NODE_TYPE(root->children[i]) == NODE_FN_HEAD)
            node_stack_push(scopes[0], root->children[i]);

    AST_TRAVERSE_START(root, node)

    if (AST_NODE_TYPE(node) == NODE_FN_BODY) {
        scope = scope_level(node->children[0]);

        assert(scope > 0);
        assert(scopes[scope - 1]);

        if (scopes[scope])
            node_stack_free(scopes[scope]);

        scopes[scope] = node_stack_clone(scopes[scope - 1]);

        // Append the list of current function's arguments to the nested scope.
        assert(AST_NODE_TYPE(node->parent) == NODE_FN_HEAD);
        assert(node->parent->nary == 2);
        assert(AST_NODE_TYPE(node->parent->children[0]) == NODE_BLOCK);
        assert(node->parent->children[1] == node);

        if (add_scope_node(scopes, scope, node->parent->children[0]))
            error = 1;

        // Construct a list of all variables defined in the nested scope.
        ast_node *vars_block = get_func_body_block(node, NODE_BLOCK_VARS);

        if (!vars_block)
            return error;

        if (add_scope_node(scopes, scope, vars_block))
            error = 1;

        // Append the list of all function declarations to the nested scope.
        ast_node *func_block = get_func_body_block(node, NODE_BLOCK_FUNCS);

        if (!func_block)
            return error;

        if (add_scope_node(scopes, scope, func_block))
            error = 1;

        // Use type inference to check if the returned value's type matches the
        // return type of the function header.
        if (type_check_return_node(scopes[scope], node))
            error = 1;
    } else if (AST_NODE_TYPE(node) == NODE_CALL) {
        // Use type inference to check if the argument types match the
        // parameter types of the function header.

        ast_node *def_node;
        node_stack *vars_scope = get_scope(scopes, node);

        if (!(def_node = scope_contains_ident(vars_scope, node))
                || type_check_call_node(vars_scope, node, def_node))
            error = 1;
    } else if (AST_NODE_TYPE(node) == NODE_ASSIGN) {
        // Use type inference to check if the assigned expression type matches
        // the type of the identifier on the left side of the assignment.
        ast_node *def_node;
        node_stack *vars_scope = get_scope(scopes, node);

        if (!(def_node = scope_contains_ident(vars_scope, node))
                || type_check_assign_node(vars_scope, node, def_node))
            error = 1;
    }

    AST_TRAVERSE_END(root, node)

    for (i = 0; scopes[i]; i++)
        node_stack_free(scopes[i]);

    free(scopes);

    return error;
}

