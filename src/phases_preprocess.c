#include <string.h>
#include <assert.h>

#include "ast.h"
#include "ast_printer.h"
#include "phases.h"

//unsigned int pass_prune_empty_nodes(ast_node *root)
//{
//    unsigned int changed = 0;
//
//    AST_TRAVERSE_START(root, node)
//
//    if (node->nary == 0
//            && AST_NODE_TYPE(node) != NODE_CONST
//            && AST_NODE_TYPE(node) != NODE_VAR_DEC
//            && AST_NODE_TYPE(node) != NODE_FN_HEAD) {
//        ast_node_print(" == remove: %s ==\n", node);
//
//        node = ast_node_remove(node->parent, node);
//        ast_free_node(node);
//        node = NULL;
//        changed = 1;
//    }
//
//    AST_TRAVERSE_END(root, node)
//
//    return changed;
//}

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

    assert(fn_body->nary == 3);

    assert(AST_NODE_TYPE(fn_body->children[0]) == NODE_BLOCK);
    assert(AST_NODE_TYPE(fn_body->children[1]) == NODE_BLOCK);
    assert(AST_NODE_TYPE(fn_body->children[2]) == NODE_BLOCK);

    return fn_body->children[b];
}

unsigned int pass_split_var_init(ast_node *root)
{
    unsigned int changed = 0;
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
            ast_node_print(" == split global var init: %s ==\n", node);

            if (!__init && !(__init = create_global_init(root)))
                return changed;

            ast_node_insert(parent, var_dec, 0);

            block = get_func_body_block(__init, NODE_BLOCK_STMTS);

            if (!block)
                return changed;

            ast_node_append(block, ast_new_node(NODE_ASSIGN,
                        (ast_data_type){.sval = strdup(node->data.sval)}));

            ast_node_append(block->children[block->nary - 1],
                    ast_node_remove(node, node->children[0]));
        } else {
            ast_node_print(" == split var init: %s ==\n", node);

            block = get_func_body_block(parent->parent, NODE_BLOCK_VARS);

            if (!block)
                return changed;

            ast_node_append(block, var_dec);

            block = get_func_body_block(parent->parent, NODE_BLOCK_STMTS);

            if (!block)
                return changed;

            ast_node_append(block, ast_new_node(NODE_ASSIGN,
                        (ast_data_type){.sval = strdup(node->data.sval)}));

            ast_node_append(block->children[block->nary - 1],
                    ast_node_remove(node, node->children[0]));
        }

        ast_node_remove(node->parent, node);
        ast_free_node(node);
        node = NULL;

        changed = 1;
    }

    AST_TRAVERSE_END(root, node)

    return changed;
}
