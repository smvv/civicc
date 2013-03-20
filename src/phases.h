#ifndef GUARD_PHASES__

#include "ast.h"

typedef unsigned int (*pass_fn)(ast_node *root);

// Preprocessor phase
//unsigned int pass_prune_empty_nodes(ast_node *root);
unsigned int pass_split_var_init(ast_node *root);

// Analysis phase
unsigned int pass_context_analysis(ast_node *root);

// Loops phase
unsigned int pass_while_to_do(ast_node *root);
unsigned int pass_for_to_do(ast_node *root);

#define COMPILER_PHASES \
pass_fn preprocess_passes[] = { \
    /*&pass_prune_empty_nodes,*/ \
    &pass_split_var_init, \
}; \
 \
pass_fn analyse_passes[] = { \
    &pass_context_analysis, \
}; \
 \
pass_fn loops_passes[] = { \
    &pass_for_to_do, \
    &pass_while_to_do, \
}; \

#define GUARD_PHASES__
#endif
