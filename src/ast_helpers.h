#ifndef GUARD_AST_HELPERS__

#include "ast.h"

void ast_error(const char *msg, ast_node *node);
ast_node *create_global_init(ast_node *root);
ast_node *get_func_body_block(ast_node *fn_body, size_t b);
ast_node *find_func_body(ast_node *node);
ast_node *find_func_head(ast_node *node);

#define GUARD_AST_HELPERS__
#endif
