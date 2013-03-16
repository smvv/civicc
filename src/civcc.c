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

void preprocess_tree(ast_node *root, int dump_ast)
{
    size_t i;
    unsigned int changed;

    if (dump_ast) {
        printf("=== Preprocess tree ===\n");
        ast_print_tree(root, 0);
    }

    pass_fn passes[] = {
        &pass_prune_empty_nodes,
    };

    do {
        changed = 0;

        for (i = 0; i < sizeof(passes) / sizeof(pass_fn); i++)
            changed |= passes[i](root);

        if (changed && dump_ast)
            ast_print_tree(root, 0);
    } while(changed);
}

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

    preprocess_tree(root, dump_ast);

    ast_free_node(root);

    return 0;
}
