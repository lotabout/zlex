/* test of NFA lexical analyzer and parser */

#include <stdio.h>
#include <malloc.h>
#include "nfa.h"

#define MAXBUF 2048
char *line = NULL;

char *get_expr(void)
{
    size_t len;
    ssize_t read;
    read = getline(&line, &len, stdin);
    if (read < 0) {
        free(line);
        return NULL;
    }
    printf("Line Got: %s", line);
    return line;
}

int main(int argc, char *argv[])
{
    thompson(get_expr, 100, NULL);
    return 0;
}
