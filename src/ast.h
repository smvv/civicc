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

#define AST_NODE_TYPE_SHIFT 0
#define AST_MODIFIER_SHIFT 4
#define AST_DATA_TYPE_SHIFT 7

typedef enum {
    // Packed in 4 bits
    NODE_BLOCK = (1 << AST_NODE_TYPE_SHIFT) - 1,
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
    // Packed in 3 bits
    NODE_FLAG_EXTERN = 1 << AST_MODIFIER_SHIFT,
    NODE_FLAG_EXPORT = 2 << AST_MODIFIER_SHIFT,
    NODE_FLAG_RETURN = 4 << AST_MODIFIER_SHIFT,
} ast_modifier_flag;

typedef enum {
    // Packed in 3 bits
    NODE_FLAG_VOID = 1 << AST_DATA_TYPE_SHIFT,
    NODE_FLAG_BOOL = 2 << AST_DATA_TYPE_SHIFT,
    NODE_FLAG_INT = 3 << AST_DATA_TYPE_SHIFT,
    NODE_FLAG_FLOAT = 4 << AST_DATA_TYPE_SHIFT,
    NODE_FLAG_IDENT = 5 << AST_DATA_TYPE_SHIFT,
} ast_data_type_flag;

#define AST_NODE_TYPE_MASK (0xf << AST_NODE_TYPE_SHIFT)
#define AST_DATA_TYPE_MASK (0x7 << AST_DATA_TYPE_SHIFT)
#define AST_MODIFIER_MASK (0x7 << AST_MODIFIER_SHIFT)

#define AST_NODE_TYPE(node) ((node)->type & AST_NODE_TYPE_MASK)
#define AST_DATA_TYPE(node) ((node)->type & AST_DATA_TYPE_MASK)
#define AST_MODIFIER(node) ((node)->type & AST_MODIFIER_MASK)

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
ast_node *ast_node_remove(ast_node *parent, unsigned int index);
ast_node *ast_flag_set(ast_node *node, unsigned int type);
void ast_free_node(ast_node *node);

const char *ast_modifier_name(ast_modifier_flag flag);
const char *ast_data_type_name(ast_data_type_flag flag);
const char *ast_node_type_name(ast_node_type_flag flag);
const char *ast_op_type_name(ast_op_type type);

#define GUARD_AST_NODE__
#endif
