#ifndef GUARD_PHASES__

#include "ast.h"

typedef unsigned int (*pass_fn)(ast_node *root);

unsigned int pass_prune_empty_nodes(ast_node *root);

#define GUARD_PHASES__
#endif
