#include "ast.h"
#include "ast_printer.h"
#include "phases.h"

unsigned int pass_prune_empty_nodes(ast_node *root)
{
    unsigned int changed = 0;

    AST_TRAVERSE_START(root, node)

    if (node->nary == 0
            && AST_NODE_TYPE(node) != NODE_CONST
            && AST_NODE_TYPE(node) != NODE_VAR_DEC
            && AST_NODE_TYPE(node) != NODE_FN_HEAD) {
        ast_node_print(" == remove: %s ==\n", node);

        node = ast_node_remove(node->parent, node);
        ast_free_node(node);
        node = NULL;
        changed = 1;
    }

    AST_TRAVERSE_END(root, node)

    return changed;
}
