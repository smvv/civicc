#include <stdio.h>
#include <string.h>

#include "ast.h"
#include "ast_printer.h"
#include "phases.h"

const char *usage_msg =
"Usage: %s [OPTIONS] <civic_file>\n"
"\n"
"Options:\n"
"  -b  Print bison parser debug information to stdout.\n"
"  -t  Dump AST tree to stdout.\n"
;

extern int yyparse(ast_node *root);
extern int yylex_destroy();
extern int yydebug;
extern FILE *yyin;

#define DECLARE_PHASE(name) \
    unsigned int name##_tree(ast_node *root, int dump_ast) \
    { \
        size_t i; \
        unsigned int error = 0; \
    \
        if (dump_ast) { \
            printf("=== " #name " tree ===\n"); \
            ast_print_tree(root); \
        } \
    \
        for (i = 0; i < sizeof(name##_passes) / sizeof(pass_fn); i++) \
            error |= name##_passes[i](root); \
    \
        return error; \
    }

COMPILER_PHASES
DECLARE_PHASE(preprocess)
DECLARE_PHASE(analyse)
DECLARE_PHASE(loops)

ast_node *parse_file(const char *filename)
{
    ast_node *root;

    if (strncmp(filename, "-", 2) == 0)
        yyin = stdin;
    else
        yyin = fopen(filename, "r");

    if (!yyin) {
        perror("fopen");
        return NULL;
    }

    root = ast_new_node(NODE_BLOCK, (ast_data_type){.sval = NULL});

    int result = yyparse(root);

    fclose(yyin);

    yylex_destroy();

    if (result)
        return NULL;

    return root;
}

int main(int argc, const char *argv[])
{
    int i;
    ast_node *root;

    int dump_ast = 0;
    int exit_code = 0;

    if (argc < 2) {
        printf(usage_msg, argv[0]);
        return 1;
    }

    for (i = 1; i < argc - 1; i++) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) == 1)
                break;

            switch (argv[i][1]) {
                case 'b': yydebug = 1; break;
                case 't': dump_ast = 1; break;
            }
        }
    }

    root = parse_file(argv[i]);

    if (!root)
        return 1;

    if (preprocess_tree(root, dump_ast)) {
        exit_code = 2;
        goto exit;
    }

    if (analyse_tree(root, dump_ast)) {
        exit_code = 3;
        goto exit;
    }

    if (loops_tree(root, dump_ast)) {
        exit_code = 4;
        goto exit;
    }

    if (dump_ast) {
        printf("=== output tree ===\n");
        ast_print_tree(root);
    }

exit:
    ast_free_node(root);

    return exit_code;
}
