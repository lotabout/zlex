/*-----------------------------------------------------------------------------
 * terp.c
 * Interpret NFA machines
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include "nfa.h"
#include "set.h"
#include "terp.h"

/*----------------------------------------------------------------------------*/ 
static nfa_t *NFA_states;
static int max_states;

/* Compile the NFA and initialize the various global variables used by
 * move() and e_clsure(). Return the state number(index) of the NFA start
 * state. This routine must be called before either e_closure() or move()
 * are called. */
int nfa(char *(*input_func)(void))
{
    nfa_t *start;
    NFA_states = thompson(input_func, &start, &max_states);

    return start->nfa_id;
}


void free_nfa(void)
{
    destory_thompson();
}


/* old: is the set of start states to examine
 * *accept: is modified to point to the string associated with an accepting
 *          state (NULL if the state isn't an accepting state).
 * *anchor: is modified to hold the anchor point, if any.
 *
 * Compute the epsilon closure set for the input states. The output set will
 * contain all states that can be reached by making epsilon transitios set or
 * the closure set is empty. */
set_t *e_closure(set_t *old, char **accept, anchor_t *anchor)
{
    int stack[MAX_NFA_STATES];  /* stack of untested states */
    int *top = stack-1;         /* stack pointer */
    nfa_t *run = NULL;          /* the current running NFA state */
    int i;                      /* state number of */
    int accept_num = INT_MAX;
        
    if (old == NULL) {
        goto exit;
    }
    *accept = NULL;

    /* push all states into stack */
    for (set_next_member(NULL); (i = set_next_member(old)) >= 0; ) {
        *++top = i;
    }

    while (top >= stack) {
        i = *top--;

        run = &NFA_states[i];
        if (run->accept && (i < accept_num)) {
            accept_num = i;
            *accept = run->accept;
            *anchor = run->anchor;
        }

        if (run->edge == EPSILON) {
            if (run->next1) {
                i = run->next1->nfa_id;
                if (!set_is_member(old, i)) {
                    set_add(old, i);
                    *++top = i;
                }
            }

            if (run->next2) {
                i = run->next2->nfa_id;
                if (!set_is_member(old, i)) {
                    set_add(old, i);
                    *++top = i;
                }
            }
        }
    }

exit:
    return old;
}

/* return a set that contains all NFA states that can be reached by making
 * transitions on *c* from any NFA state in *old*, returns NULL if there's no
 * such transitios, the *old* set is not modified. */
set_t *move(set_t *old, int c)
{
    int i; 
    nfa_t *run = NULL; /* current NFA state. */
    set_t *output = NULL; /* output set */


    for (set_next_member(NULL); (i=set_next_member(old) >= 0); ) {
        run = &NFA_states[i];

        if (run->edge == c ||
            (run->edge == CCL && set_is_member(run->bitset, c))) {
            if (output == NULL) {
                output = set_new();
            }
            set_add(output, run->next1->nfa_id);
        }
    }

    return output;
}
