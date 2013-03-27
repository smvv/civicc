#include <string.h>
#include <assert.h>

#include "ast.h"
#include "ast_helpers.h"
#include "ast_printer.h"
#include "phases.h"

unsigned int pass_split_var_init(ast_node *root)
{
    ast_node *parent;
    ast_node *__init = NULL;
    ast_node *block;

    AST_TRAVERSE_START(root, node)

    if (AST_NODE_TYPE(node) == NODE_VAR_DEF) {
        parent = node->parent;

        // Split the variable definition into a declaration part and a
        // initialisation part.

        ast_node *var_dec = ast_new_node(NODE_VAR_DEC,
                (ast_data_type){.sval = strdup(node->data.sval)});

        ast_flag_set(var_dec, AST_DATA_TYPE(node));

        if (!parent->parent) {
            if (!__init && !(__init = create_global_init(root)))
                return 1;

            ast_node_insert(parent, var_dec, 0);

            block = get_func_body_block(__init, NODE_BLOCK_STMTS);

            if (!block)
                return 1;

            ast_node_insert(block, NEW_ASSIGN(strdup(node->data.sval)), 0);
            ast_node_append(block->children[0],
                            ast_node_remove(node, node->children[0]));
        } else {
            block = get_func_body_block(parent->parent, NODE_BLOCK_VARS);

            if (!block)
                return 1;

            ast_node_append(block, var_dec);

            block = get_func_body_block(parent->parent, NODE_BLOCK_STMTS);

            if (!block)
                return 1;

            ast_node_insert(block, NEW_ASSIGN(strdup(node->data.sval)), 0);
            ast_node_append(block->children[0],
                            ast_node_remove(node, node->children[0]));
        }

        ast_node_remove(node->parent, node);
        ast_free_node(node);
        node = NULL;
    } else if (AST_NODE_TYPE(node) == NODE_FOR) {
        ast_node *var_dec = NEW_VAR_DEC(strdup(node->data.sval));
        ast_flag_set(var_dec, NODE_FLAG_INT);

        block = get_func_body_block(find_func_body(node), NODE_BLOCK_VARS);

        if (!block)
            return 1;

        ast_node_append(block, var_dec);
    }

    AST_TRAVERSE_END(root, node)

    return 0;
}
