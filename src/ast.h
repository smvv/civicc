#ifndef GUARD_AST_NODE__

#include <stdlib.h>
#include <stdint.h>

#define AST_NODE_BUFFER_SIZE 16

typedef struct ast_node ast_node;

typedef union {
    int ival;
    double dval;
    char *sval;
    struct ast_node* nval;
} ast_data_type;

struct ast_node {
    uint32_t type;
    uint32_t nary;
    struct ast_node **children;
    ast_data_type data;
};

typedef enum {
    // Packed in 4 bits
    NODE_BLOCK,
    NODE_FN_HEAD,
    NODE_FN_BODY,
    NODE_VAR_DEC,
    NODE_VAR_DEF,
    NODE_PARAM,
    NODE_ASSIGN,
    NODE_CALL,
    NODE_IF,
    NODE_WHILE,
    NODE_DO_WHILE,
    NODE_FOR,
    NODE_UNARY_OP,
    NODE_BIN_OP,
    NODE_CAST,
    NODE_CONST,
} ast_node_type_flag;

typedef enum {
    // Packed in 2 bits
    NODE_FLAG_EXTERN = 1 << 4,
    NODE_FLAG_EXPORT = 1 << 5,
} ast_modifier_flag;

typedef enum {
    // Packed in 3 bits
    NODE_FLAG_VOID = 1 << 6,
    NODE_FLAG_BOOL = 2 << 6,
    NODE_FLAG_INT = 3 << 6,
    NODE_FLAG_FLOAT = 4 << 6,
    NODE_FLAG_IDENT = 5 << 6,
} ast_data_type_flag;

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

ast_node *ast_new_node(ast_node_type_flag flag, ast_data_type data);
ast_node *ast_node_append(ast_node *parent, ast_node *child);
ast_node *ast_flag_set(ast_node *node, unsigned int type);
void ast_free_node(ast_node *node);

void ast_node_print(ast_node *node);
char *ast_node_format(ast_node *node);

const char *ast_modifier_name(ast_modifier_flag flag);
const char *ast_data_type_name(ast_data_type_flag flag);
const char *ast_node_type_name(ast_node_type_flag flag);
const char *ast_op_type_name(ast_op_type type);

#define AST_NODE_TYPE(node) ((node)->type & 0xf)
#define AST_DATA_TYPE(node) (((node)->type >> 6) & 0x7)
#define AST_MODIFIER(node) (((node)->type >> 4) & 0x3)

#define GUARD_AST_NODE__
#endif
