#ifndef GUARD_AST_PRINTER__

#include "ast.h"

void ast_print_tree(ast_node *node, unsigned int level);
void ast_node_print(const char *msg, ast_node *node);
size_t ast_node_format(ast_node *node, char *buf, size_t buflen);

#define GUARD_AST_PRINTER__
#endif
