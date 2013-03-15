#include <stdio.h>
#include <string.h>

#include "ast.h"

extern int yyparse(ast_node *root);
extern int yylex_destroy();
extern int yydebug;
extern FILE *yyin;

int main(int argc, const char *argv[])
{
    int i;

    if (argc < 2) {
        printf("Usage: %s [OPTIONS] <civic_file>\n", argv[0]);
        return 1;
    }

    for (i = 1; i < argc - 1; i++) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) == 1)
                break;

            switch (argv[i][1]) {
                case 'd': yydebug = 1; break;
            }
        }
    }

    if (strncmp(argv[i], "-", 2) == 0)
        yyin = stdin;
    else
        yyin = fopen(argv[i], "r");

    if (!yyin) {
        perror("fopen");
        return 1;
    }

    ast_node *root = ast_new_node(NODE_BLOCK, (ast_data_type){.sval = NULL});

    int result = yyparse(root);

    fclose(yyin);

    yylex_destroy();

    if (result)
        return result;

    printf("root children: %u\n", root ? root->nary : 0 );

    // TODO: print AST

    ast_free_node(root);

    return 0;
}
