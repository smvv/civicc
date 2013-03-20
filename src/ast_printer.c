#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ast.h"
#include "ast_printer.h"

static size_t _ast_node_format_add(const char *msg, size_t msglen, char *buf,
        size_t buflen)
{
    strncpy(buf, msg, msglen > buflen ? buflen : msglen);

    return msglen;
}

size_t ast_node_format(ast_node *node, char *buf, size_t buflen)
{
    if (!node || !buf)
        return 0;

    size_t i = 0;
    const char *msg;

#define APPEND(pattern, data, ...) \
    i += snprintf(buf + i, buflen - i, pattern, data, ##__VA_ARGS__)

    switch (AST_NODE_TYPE(node)) {
    case NODE_BLOCK: APPEND("block (%u)", node->nary); break;
    case NODE_ASSIGN: APPEND("%s =", node->data.sval); break;
    case NODE_CONST:
        switch (AST_DATA_TYPE(node)) {
        case NODE_FLAG_BOOL: APPEND("%d", node->data.ival); break;
        case NODE_FLAG_INT: APPEND("%d", node->data.ival); break;
        case NODE_FLAG_FLOAT: APPEND("%f", node->data.dval); break;
        case NODE_FLAG_IDENT: APPEND("%s", node->data.sval); break;
        }
    break;
    case NODE_VAR_DEC:
    case NODE_VAR_DEF:
        msg = ast_data_type_name(AST_DATA_TYPE(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);

        if (i && buf[i - 1] != ' ')
            buf[i++] = ' ';

        if (AST_NODE_TYPE(node) == NODE_VAR_DEF)
            APPEND("%s =", node->data.sval);
        else
            APPEND("%s", node->data.sval);
    break;
    case NODE_CALL:
        APPEND("%s(", node->data.sval);

        assert(node->nary == 1);
        assert(AST_NODE_TYPE(node->children[0]) == NODE_BLOCK);

        for (unsigned int j = 0; j < node->children[0]->nary; j++) {
            APPEND("$%d,", j);
        }

        if (buf[i-1] == '(')
            i++;

        buf[i-1] = ')';
    break;
    case NODE_PARAM:
        APPEND("%s ", ast_data_type_name(AST_DATA_TYPE(node)));
        APPEND("%s", node->data.sval);
    break;
    case NODE_FN_HEAD:
        msg = ast_modifier_name(AST_MODIFIER(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);

        if (i && buf[i - 1] != ' ')
            buf[i++] = ' ';

        msg = ast_data_type_name(AST_DATA_TYPE(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);

        if (i && buf[i - 1] != ' ')
            buf[i++] = ' ';

        // TODO: display function arguments
        APPEND("%s()", node->data.sval);
    break;
    case NODE_FN_BODY:
        msg = ast_node_type_name(AST_NODE_TYPE(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);

        APPEND(" return=%u", !!(AST_MODIFIER(node) & NODE_FLAG_RETURN));
    break;
    case NODE_FOR:
    case NODE_WHILE:
        msg = ast_node_type_name(AST_NODE_TYPE(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);

        APPEND(" %s =", node->data.sval);
    break;
    case NODE_UNARY_OP:
    case NODE_BIN_OP:
        msg = ast_node_type_name(AST_NODE_TYPE(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);
        APPEND(" %s", ast_op_type_name(node->data.ival));
    break;
    default:
        msg = ast_node_type_name(AST_NODE_TYPE(node));
        i += _ast_node_format_add(msg, strlen(msg), buf + i, buflen - i);
        break;
    }

#undef APPEND

    buf[i] = 0;

    assert(i < buflen);

    return i;
}

void ast_node_print(const char *msg, ast_node *node)
{
    unsigned int buflen = 256;
    char *buf = malloc(buflen * sizeof(char));

    if (!buf)
        return;

    size_t i = ast_node_format(node, buf, buflen);

    if (i) {
        printf(msg, buf);
    } else {
        printf(msg, "(nil)");
    }

    free(buf);
}

static void ast_print_tree_r(ast_node *node, unsigned int level)
{
    unsigned int i;

    unsigned int buflen = 256;
    char *buf;

    if (!node)
        return;

    buf = malloc(buflen * sizeof(char));

    if (!buf)
        return;

    for (i = 0; i < 2 * level; i++)
        buf[i] = ' ';

    i += ast_node_format(node, buf + i, buflen - i);

    if (i > 2 * level) {
        printf("%s\n", buf);
    } else {
        buf[i] = 0;
        printf("%s(nil)\n", buf);
    }

    if (node->children)
        for (i = 0; i < node->nary; i++)
            ast_print_tree_r(node->children[i], level + 1);

    free(buf);
}

void ast_print_tree(ast_node *root)
{
    ast_print_tree_r(root, 0);
}
