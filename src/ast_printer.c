#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ast.h"
#include "ast_printer.h"

size_t ast_node_format(ast_node *node, size_t buflen, char *buf)
{
    if (!node || !buf)
        return 0;

    size_t i = 0, j;
    const char *msg;

    msg = ast_modifier_name(AST_MODIFIER(node));
    j = strlen(msg);

    strncpy(buf + i, msg, j);
    i += j;

    if (j) {
        buf[i] = ' ';
        i++;
    }

    msg = ast_data_type_name(AST_DATA_TYPE(node));
    j = strlen(msg);

    strncpy(buf + i, msg, j);
    i += j;

    if (i) {
        buf[i] = ' ';
        i++;
    }

    msg = ast_node_type_name(AST_NODE_TYPE(node));
    j = strlen(msg);

    strncpy(buf + i, msg, j);
    i += j;

    buf[i] = 0;

    assert(i < buflen);

    return i;
}

void ast_node_print(ast_node *node)
{
    unsigned int buflen = 256;
    char *buf = malloc(buflen * sizeof(char));

    if (!buf)
        return;

    size_t i = ast_node_format(node, buflen, buf);

    if (i) {
        printf("[%s; c=%d]", buf, node->nary);
    } else {
        printf("(nil)");
    }

    free(buf);
}

void ast_print_tree(ast_node *node, unsigned int level)
{
    unsigned int i;

    if (!node)
        return;

    assert(level < 80);

    unsigned int buflen = 256;
    char *buf = malloc(buflen * sizeof(char));

    if (!buf)
        return;

    for (i = 0; i < 2 * level; i++)
        buf[i] = ' ';

    i += ast_node_format(node, buflen - i, buf + i);

    if (i > 2 * level) {
        printf("%s (%u)\n", buf, node->nary);
    } else {
        buf[i] = 0;
        printf("%s(nil)\n", buf);
    }

    if (node->children) {
        for (i = 0; i < node->nary; i++) {
            ast_print_tree(node->children[i], level + 1);
        }
    }

    free(buf);
}
