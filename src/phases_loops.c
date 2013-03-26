#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "phases.h"
#include "ast.h"
#include "ast_helpers.h"
#include "ast_printer.h"

unsigned int pass_while_to_do(ast_node *root)
{
    (void) root;
    return 0;
}

unsigned int pass_for_to_do(ast_node *root)
{
    AST_TRAVERSE_START(root, node)

    if (AST_NODE_TYPE(node) == NODE_FOR) {

        assert(node->nary == 3 || node->nary == 4);
        assert(AST_NODE_TYPE(node->children[0]) == NODE_CONST);
        assert(AST_DATA_TYPE(node->children[0]) == NODE_FLAG_INT);
        assert(AST_NODE_TYPE(node->children[1]) == NODE_CONST);
        assert(AST_DATA_TYPE(node->children[1]) == NODE_FLAG_INT);

        //printf("Found for-node:\n");
        //ast_print_tree(node);

        // Create the initialization statement of the loop counter
        ast_node *loop_counter = ast_new_node(NODE_ASSIGN,
                        (ast_data_type){.sval = strdup(node->data.sval)});

        ast_node_append(loop_counter, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.ival = node->children[0]->data.ival}),
                    NODE_FLAG_INT));

        // Create the body of the loop and the loop condition
        ast_node *do_body = node->children[node->nary - 1];

        ast_node *if_cond = ast_new_node(NODE_BIN_OP,
                (ast_data_type){.ival = OP_LT});

        ast_node_append(if_cond, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.sval = strdup(node->data.sval)}),
                    NODE_FLAG_IDENT));

        ast_node_append(if_cond, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.ival = node->children[1]->data.ival}),
                    NODE_FLAG_INT));

        ast_node *do_stmt = ast_new_node(NODE_DO_WHILE,
                (ast_data_type){.nval = NULL});

        ast_node_append(do_stmt, if_cond);
        ast_node_append(do_stmt, do_body);

        // Append loop counter increment statement to loop body
        ast_node *counter_incr = ast_new_node(NODE_ASSIGN,
                        (ast_data_type){.sval = strdup(node->data.sval)});

        ast_node *counter_add = ast_new_node(NODE_BIN_OP,
                (ast_data_type){.ival = OP_ADD});

        ast_node_append(counter_incr, counter_add);

        ast_node_append(counter_add, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.sval = strdup(node->data.sval)}),
                    NODE_FLAG_IDENT));

        ast_node_append(counter_add, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.ival = node->nary == 4
                            ? node->children[2]->data.ival : 1 }),
                    NODE_FLAG_INT));

        ast_node_append(do_body, counter_incr);

        // Create if statement and its condition (e.g. "if (i < 4) ...")
        if_cond = ast_new_node(NODE_BIN_OP,
                (ast_data_type){.ival = OP_LT});

        ast_node_append(if_cond, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.sval = strdup(node->data.sval)}),
                    NODE_FLAG_IDENT));

        ast_node_append(if_cond, ast_flag_set(ast_new_node(NODE_CONST,
                        (ast_data_type){.ival = node->children[1]->data.ival}),
                    NODE_FLAG_INT));

        ast_node *if_stmt = ast_new_node(NODE_IF,
                (ast_data_type){.nval = NULL});

        ast_node_append(if_stmt, if_cond);
        ast_node_append(if_stmt, do_stmt);

        if (!if_stmt)
            return 1;

        //printf("Generated loop_counter:\n");
        //ast_print_tree(loop_counter);

        //printf("Generated if-statement:\n");
        //ast_print_tree(if_stmt);

        int pos = ast_node_pos(node->parent, node);

        // Insert the loop-counter-var at the for-loop's position
        ast_node_insert(node->parent, loop_counter, pos);

        // Insert the if-statement after the loop-counter-var
        ast_node_insert(node->parent, if_stmt, pos + 1);

        // Remove the for-loop from the AST
        ast_node_remove(node->parent, node);
        // TODO: free for-loop node (but not its children!)
    }

    AST_TRAVERSE_END(root, node)

    return 0;
}
