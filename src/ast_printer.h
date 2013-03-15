#ifndef GUARD_AST_PRINTER__

#include "ast.h"

void ast_print_tree(ast_node *node, unsigned int level);
void ast_node_print(ast_node *node);
size_t ast_node_format(ast_node *node, size_t buflen, char *buf);

#define GUARD_AST_PRINTER__
#endif
