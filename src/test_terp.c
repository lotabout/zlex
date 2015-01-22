/* test_terp.c
 * Test interpret NFA machine.
 * Most of the code here belongs to book _Compiler Design in C_
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include "set.h"
#include "nfa.h"
#include "terp.h"

#define BSIZE 256

static char Buf[BSIZE]; /* input buffer */
static char *pBuf = Buf; /* current position in input buffer */
static char *Expr;   /* regular expression from command line */

/* Wrapper of input function intended to pass to thompson() */
int nextchar()
{
    if (! *pBuf) {
        if (!fgets(Buf, BSIZE, stdin)) {
            return '\0';
        }
        pBuf = Buf;
    }

    return *pBuf++;
}

static void printbuf()
{
    fputs(Buf, stdout);
    *pBuf = 0;
}

static char *my_getline()
{
    static bool first_time = true;
    if (!first_time) {
        return NULL;
    }

    first_time = false;
    return Expr;
}

int main(int argc, char *argv[])
{
    int start; /* start NFA state number */
    set_t *start_dfastate; /* start NFA states */
    set_t *current; /* current DFA state */
    set_t *next;
    char *accept; /* if current DFA state is an accept */
    int c; /* current input character */
    anchor_t anchor; 

    if (argc == 2) {
        fprintf(stderr, "exprssion is %s\n", argv[1]);
    } else {
        fprintf(stderr, "usage: test_terp pattern < input\n");
        exit(1);
    }

    /* 1. compile the NFA. */
    Expr = argv[1];
    start = nfa(my_getline);

    /* create the initial state. */
    next = set_new();
    set_add(next, start);
    if ((start_dfastate = e_closure(next, &accept, &anchor)) == NULL) {
        fprintf(stderr, "Internal error: State machine is empty");
        exit(1);
    }

    current = set_new();
    set_assign(current, next);

    /* now interpret the NFA. */
    while ((c = nextchar()) != '\0') {
        if ((next = e_closure(move(current, c), &accept, &anchor)) != NULL) {
            if (accept) {
                printbuf();
            } else {
                set_del(current);
                current = next;
                continue;
            }
        }

        /* reset */
        set_del(next);
        set_assign(current, start_dfastate);
    }

    return 0;
}
