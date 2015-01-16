#include <stdio.h>
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
static void print_nfa_plain(nfa_t *nfa, int max_state, nfa_t *start);
static void print_nfa_graphviz(nfa_t *nfa, int max_state, nfa_t *start);

/*---------------------------------------------------------------------------*/
void print_nfa(nfa_t *nfa, int max_state, nfa_t *start, nfa_print_t type)
{
    if (type == NFA_PLAIN) {
        print_nfa_plain(nfa, max_state, start);
    } else if (type == NFA_GRAPHVIZ) {
        print_nfa_graphviz(nfa, max_state, start);
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

static int get_id(nfa_t *nfa, nfa_t *start)
{
    return nfa - start;
}

static void print_nfa_graphviz(nfa_t *nfa, int max_state, nfa_t *start)
{
    /* in order to efficient check if a node is visited, we'll make advantage
     * of the assumption of implementation detail: all NFA states are
     * allocated from an array. Note that this is wrong if the implementation
     * is changed. 
     * Due to the discarded states, the output might contain them as noises. */
    printf("digraph {\n");

    nfa_t *s = nfa;
    for (; --max_state >= 0; nfa++) {
        if (!nfa->next1) {
            printf("%d[shape=doublecircle, xlabel=\"%s<%s>%s\"];\n",
                   get_id(nfa, s), nfa->anchor & START ? "^" : "",
                   nfa->accept,
                   nfa->anchor & END ? "$" : "");
        } else {
            if (nfa->next2) {
                printf("%d -> %d[label=\"(ɛ)\"];\n",
                       get_id(nfa, s), get_id(nfa->next2, s));
            }

            printf("%d -> %d[label=\"", get_id(nfa, s), get_id(nfa->next1, s));
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

        if (nfa->accept) {
            printf("%d[shape=doublecircle, xlabel=\"%s<%s>%s\"];\n",
                   get_id(nfa, s), nfa->anchor & START ? "^" : "",
                   nfa->accept,
                   nfa->anchor & END ? "$" : "");
        }
    }

    printf("}\n");
}

static void print_nfa_plain(nfa_t *nfa, int max_state, nfa_t *start)
{

    nfa_t *s = nfa ;

    printf( "\n----------------- NFA ---------------\n" );

    for(; --max_state>= 0 ; nfa++ )
    {
        printf( "NFA state %d: ", get_id(nfa, s) );

        if( !nfa->next1 )
            printf("(TERMINAL)");
        else
        {
            printf( "--> %d ",  get_id(nfa->next1, s));
            if (nfa->next2) {
                printf( "(%d) on ", get_id(nfa->next2, s));
            }

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

        if( nfa == start )
            printf(" (START STATE)");

        if( nfa->accept )
            printf(" accepting %s<%s>%s", nfa->anchor & START ? "^" : "",
                   nfa->accept,
                   nfa->anchor & END   ? "$" : "" );
        printf( "\n" );
    }
    printf( "\n-------------------------------------\n" );
}
