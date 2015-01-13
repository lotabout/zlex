/* test of NFA lexical analyzer and parser */

#include <stdio.h>
#include <malloc.h>
#include "nfa.h"

char *rules[] = {
    "[0-9]+ return TK_NUM;",
    "[0-9]*\\.[0-9]+",
    NULL
};

#define MAXBUF 2048
char **line = rules-1;

char *get_expr(void)
{
    line++;
    printf("---- Line Got: %s ----\n", *line);
    return *line;
}

int main(int argc, char *argv[])
{
    thompson(get_expr, 100, NULL);
    return 0;
}
