#include <stdio.h>

#include "ast.h"

extern int yyparse(ast_node *root);
extern int yydebug;
extern FILE *yyin;

int main(int argc, const char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <civic_file>\n", argv[0]);
        return 1;
    }

    yyin = fopen(argv[1], "r");

    if (!yyin) {
        perror("fopen");
        return 1;
    }

    ast_node *root = ast_new_node(NODE_ROOT, (data_type){.sval = "root"});

    if (!root)
        return 1;

    yydebug = 1;
    int result = yyparse(root);

    return result;
}
