#ifndef GUARD_AST_NODE__

#include <stdlib.h>
#include <stdint.h>

#define AST_NODE_BUFFER_SIZE 16

typedef union {
    int ival;
    double dval;
    char *sval;
} ast_data_type;

typedef struct ast_node {
    uint32_t type;
    uint32_t nary;
    struct ast_node **children;
    ast_data_type data;
} ast_node;

typedef enum {
    // Packed in 4 bits
    NODE_ROOT,
    NODE_FN_HEAD,
    NODE_GBL_DEC,
    NODE_GBL_DEF,
    NODE_PARAM,
} ast_node_type;

typedef enum {
    // Packed in 2 bits
    NODE_FLAG_EXTERN = 0x20,
    NODE_FLAG_EXPORT = 0x40,

    // Packed in 3 bits
    NODE_FLAG_VOID = 0x80,
    NODE_FLAG_BOOL = 0x100,
    NODE_FLAG_INT = 0x180,
    NODE_FLAG_FLOAT = 0x200,

} ast_node_flag;

typedef enum {
    OP_NEG,
    OP_NOT,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_LE,
    OP_LT,
    OP_GE,
    OP_GT,
    OP_EQ,
    OP_NE,
    OP_AND,
    OP_OR,
    OP_LAND,
    OP_LOR,
} ast_op_type;

ast_node *ast_new_node(ast_node_type type, ast_data_type data);
ast_node *ast_node_append(ast_node *parent, ast_node *child);
ast_node *ast_flag_set(ast_node *node, unsigned int type);

void ast_node_print(ast_node *node);
char *ast_node_format(ast_node *node);

const char *ast_node_flag_name(ast_node_flag flag);
const char *ast_node_type_name(ast_node_type type);
const char *ast_op_type_name(ast_op_type type);

#define GUARD_AST_NODE__
#endif
