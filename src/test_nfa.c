/* test of NFA lexical analyzer and parser */

#include <stdio.h>
#include <malloc.h>
#include "nfa.h"

char *rules[] = {
    "[0-9]+ return ICON;",
    "([0-9]+|[0-9]*\\.[0-9]+|[0-9]+\\.[0-9]*)(e[0-9]+)? return FCON",
    NULL
};

#define MAXBUF 2048
char **line = rules-1;

char *get_expr(void)
{
    line++;
    return *line;
}

int main(int argc, char *argv[])
{
    int max_state;
    nfa_t *start = thompson(get_expr, &max_state, NULL);
    /*print_nfa(start, max_state, start, NFA_GRAPHVIZ);*/
    print_nfa(start, max_state, start, NFA_PLAIN);

    return 0;
}
