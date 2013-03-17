#ifndef GUARD_AST_HELPERS__

#include "ast.h"

ast_node *create_global_init(ast_node *root);
ast_node *get_func_body_block(ast_node *fn_body, size_t b);

#define GUARD_AST_HELPERS__
#endif
