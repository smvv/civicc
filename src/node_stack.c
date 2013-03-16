#include "ast.h"

node_stack *node_stack_new()
{
    node_stack *stack = malloc(sizeof(node_stack));

    stack->data = NULL;
    stack->items = 0;
    stack->size = 0;

    return stack;
}

void node_stack_free(node_stack *stack)
{
    if (!stack)
        return;

    if (stack->data)
        free(stack->data);

    free(stack);
}

void node_stack_push(node_stack *stack, ast_node *node)
{
    if (!stack || !node)
        return;

    if (stack->items >= stack->size) {
        stack->size = stack->items + NODE_STACK_SIZE;
        stack->data = realloc(stack->data, stack->size * sizeof(ast_node *));

        if (!stack)
            return;
    }

    stack->data[stack->items++] = node;
}

ast_node *node_stack_pop(node_stack *stack)
{
    if (!stack || stack->items <= 0)
        return NULL;

    return stack->data[--stack->items];
}

int node_stack_empty(node_stack *stack)
{
    if (!stack || stack->items <= 0)
        return 1;

    return 0;
}
