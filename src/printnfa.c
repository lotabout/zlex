#include <stdio.h>
#include <stdlib.h>
#include "nfa.h"
#include "set.h"
#include <malloc.h>
#include <string.h>


/* Notes about graphviz output:
 *
 * 1. copy the output of print_nfa_graphviz() into a file, say test.gv
 * 2. compile the output via graphviz for example:
 *    $ dot -Tpdf test.gv -o test.pdf
 * 3. check the output
 *    $ okular test.pdf
 */

/*---------------------------------------------------------------------------*/
/* generate graphviz output for a NFA machine */
static void print_nfa_plain(nfa_t *nfa);
static void print_nfa_graphviz(nfa_t *nfa);

/*---------------------------------------------------------------------------*/
void print_nfa(nfa_t *nfa, nfa_print_t type)
{
    if (type == NFA_PLAIN) {
        print_nfa_plain(nfa);
    } else if (type == NFA_GRAPHVIZ) {
        print_nfa_graphviz(nfa);
    }
}

static inline void print_char(int c)
{
    if( c < ' ' )
        printf( "^%c", c + '@' );
    else
        printf( "%c", c );
}

static void printccl(set_t *set)
{
    int	i;
    int prev = -1;
    int num_skip = 0;

    putchar('[');
    for( i = 0 ; i <= 0x7f; i++ ) {
	if( set_is_member(set, i) ) {
            if (i == prev+1) {
                num_skip++;
            } else {
                if (num_skip > 2) {
                    print_char('-');
                }
                if (prev > 0) {
                    print_char(prev);
                }
                print_char(i);
                num_skip = 1;
            }
            prev = i;
	}
    }

    if (num_skip > 2) {
        print_char('-');
    }

    if (num_skip > 1) {
        print_char(prev);
    }

    putchar(']');
}

/* print the NFA state machine recursively */
static void print_state_graphviz(nfa_t *nfa)
{
    static bool *visited = NULL;
    if (visited == NULL) {
        visited = (bool *)malloc(MAX_NFA_STATES * sizeof(visited));
        if (visited == NULL) {
            fprintf(stderr, "print_state_graphviz: not enough memory.\n");
            exit(1);
        }
        /* remember that memset can only set zero(false) */
        memset(visited, false, MAX_NFA_STATES * sizeof(visited)); 
    }

    /* set nfa to NULL to reset the visited status. */
    if (nfa == NULL) {
        /* remember that memset can only set zero(false) */
        memset(visited, false, MAX_NFA_STATES * sizeof(visited)); 
        return;
    }

    visited[nfa->nfa_id] = true;

    /* print current state */
    if (nfa->accept) {
        printf("%d[shape=doublecircle, xlabel=\"%s<%s>%s\"];\n",
               nfa->nfa_id, nfa->anchor & START ? "^" : "",
               nfa->accept,
               nfa->anchor & END ? "$" : "");
    }

    /* print the links to next states */
    if (nfa->next1) {
        printf("%d -> %d[label=\"", nfa->nfa_id, nfa->next1->nfa_id);
        switch (nfa->edge) {
            case CCL:
                printccl(nfa->bitset);
                break;
            case EPSILON:
                printf("ɛ");
                break;
            default:
                printf("%c", nfa->edge);
                break;
        }
        printf("\"];\n");
    }

    if (nfa->next2) {
        printf("%d -> %d[label=\"(ɛ)\"];\n", nfa->nfa_id, nfa->next2->nfa_id);
    }

    if (nfa->next1 && !visited[nfa->next1->nfa_id]) {
        print_state_graphviz(nfa->next1);
    }

    if (nfa->next2 && !visited[nfa->next2->nfa_id]) {
        print_state_graphviz(nfa->next2);
    }
}

static void print_nfa_graphviz(nfa_t *nfa)
{
    /* in order to efficient check if a node is visited, we'll make advantage
     * of the assumption of implementation detail: all NFA states are
     * allocated from an array. Note that this is wrong if the implementation
     * is changed. 
     * Due to the discarded states, the output might contain them as noises. */

    printf("digraph {\n");
    print_state_graphviz(NULL);
    print_state_graphviz(nfa);
    printf("}\n");
}

static void print_state_plain(nfa_t *nfa)
{
    static bool *visited = NULL;
    if (visited == NULL) {
        visited = (bool *)malloc(MAX_NFA_STATES * sizeof(visited));
        if (visited == NULL) {
            fprintf(stderr, "print_state_plain: not enough memory.\n");
            exit(1);
        }
        /* remember that memset can only set zero(false) */
        memset(visited, false, MAX_NFA_STATES * sizeof(visited)); 
    }

    /* set nfa to NULL to reset the visited status. */
    if (nfa == NULL) {
        /* remember that memset can only set zero(false) */
        memset(visited, false, MAX_NFA_STATES * sizeof(visited)); 
        return;
    }

    visited[nfa->nfa_id] = true;

    /* print current state */
    printf( "NFA state %d: ", nfa->nfa_id);
    if (nfa->accept) {
        printf("(TERMINAL) accepting %s<%s>%s", nfa->anchor & START ? "^" : "",
               nfa->accept,
               nfa->anchor & END   ? "$" : "" );
    }

    /* print the links to next states */
    if (nfa->next1) {
        switch( nfa->edge )
        {
            case CCL:    
                printccl(nfa->bitset);
                break;
            case EPSILON: 
                printf("EPSILON ");
                break;
            default: 
                printf("%c", (char)nfa->edge);
                break;
        }
    }
    if (nfa->next2) {
        printf( "(%d) on ", nfa->next2->nfa_id);
    }
    printf("\n");

    if (nfa->next1 && !visited[nfa->next1->nfa_id]) {
        print_state_plain(nfa->next1);
    }
    if (nfa->next2 && !visited[nfa->next2->nfa_id]) {
        print_state_plain(nfa->next2);
    }
}


static void print_nfa_plain(nfa_t *nfa)
{
    printf( "\n----------------- NFA ---------------\n" );
    print_state_plain(NULL);
    print_state_plain(nfa);
    printf( "\n-------------------------------------\n" );
}
