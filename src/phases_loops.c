#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "phases.h"
#include "ast.h"

unsigned int pass_while_to_do(ast_node *root)
{
    (void) root;
    return 0;
}

unsigned int pass_for_to_do(ast_node *root)
{
    (void) root;
    return 0;
}
