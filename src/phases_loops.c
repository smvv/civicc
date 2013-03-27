#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "phases.h"
#include "ast.h"
#include "ast_helpers.h"
#include "ast_printer.h"

static void free_for_loop(ast_node *node)
{
    if (!node)
        return;

    // Free the for-loop start and end constant
    ast_free_leaf(node->children[0]);
    ast_free_leaf(node->children[1]);

    // Free the optional for-loop increment constant
    if (node->nary == 4)
        ast_free_leaf(node->children[2]);

    // Free for-loop node itself
    ast_free_leaf(node);
}

static void free_while_loop(ast_node *node)
{
    if (!node)
        return;

    // Free the while-loop node itself
    ast_free_leaf(node);
}

unsigned int pass_while_to_do(ast_node *root)
{
    AST_TRAVERSE_START(root, node)

    if (AST_NODE_TYPE(node) == NODE_WHILE) {
        // Create the body of the loop
        ast_node *do_stmt = NEW_DO_WHILE();
        ast_node_append(do_stmt, node->children[0]);
        ast_node_append(do_stmt, node->children[1]);

        // Create if statement and its condition
        ast_node *if_stmt = NEW_IF();
        ast_node_append(if_stmt, ast_node_clone(node->children[0]));
        ast_node_append(if_stmt, do_stmt);

        if (!if_stmt)
            return 1;

        // Insert the if-statement before the while-loop node
        int pos = ast_node_pos(node->parent, node);
        ast_node_insert(node->parent, if_stmt, pos);

        // Remove the while-loop from the AST and free its memory
        ast_node_remove(node->parent, node);
        free_while_loop(node);
        node = NULL;
    }

    AST_TRAVERSE_END(root, node)

    return 0;
}

unsigned int pass_for_to_do(ast_node *root)
{
    AST_TRAVERSE_START(root, node)

    if (AST_NODE_TYPE(node) == NODE_FOR) {
        // Create the initialization statement of the loop counter
        ast_node *loop_counter = NEW_ASSIGN(strdup(node->data.sval));
        ast_node_append(loop_counter, NEW_INT(node->children[0]->data.ival));

        // Create the body of the loop and the loop condition
        ast_node *do_body = node->children[node->nary - 1];

        ast_node *if_cond = NEW_BIN_OP(OP_LT);
        ast_node_append(if_cond, NEW_IDENT(strdup(node->data.sval)));
        ast_node_append(if_cond, NEW_INT(node->children[1]->data.ival));

        ast_node *do_stmt = NEW_DO_WHILE();

        ast_node_append(do_stmt, if_cond);
        ast_node_append(do_stmt, do_body);

        // Append loop counter increment statement to loop body
        ast_node *counter_incr = NEW_ASSIGN(strdup(node->data.sval));
        ast_node *counter_add = NEW_BIN_OP(OP_ADD);

        ast_node_append(counter_add, NEW_IDENT(strdup(node->data.sval)));
        ast_node_append(counter_add, NEW_INT(
                    node->nary == 4 ? node->children[2]->data.ival : 1));

        ast_node_append(counter_incr, counter_add);
        ast_node_append(do_body, counter_incr);

        // Create if statement and its condition (e.g. "if (i < 4) ...")
        if_cond = NEW_BIN_OP(OP_LT);

        ast_node_append(if_cond, NEW_IDENT(strdup(node->data.sval)));
        ast_node_append(if_cond, NEW_INT(node->children[1]->data.ival));

        ast_node *if_stmt = NEW_IF();

        ast_node_append(if_stmt, if_cond);
        ast_node_append(if_stmt, do_stmt);

        if (!if_stmt)
            return 1;

        // Insert the loop-counter-var at the for-loop's position and the
        // if-statement after the loop-counter-var
        int pos = ast_node_pos(node->parent, node);
        ast_node_insert(node->parent, loop_counter, pos);
        ast_node_insert(node->parent, if_stmt, pos + 1);

        // Remove the for-loop from the AST and free its memory
        ast_node_remove(node->parent, node);
        free_for_loop(node);
        node = NULL;
    }

    AST_TRAVERSE_END(root, node)

    return 0;
}
