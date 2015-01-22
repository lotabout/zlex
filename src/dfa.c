#include <stdbool.h>
#include <malloc.h>
#include <stdlib.h>

#include "dfa.h"
#include "nfa.h"
#include "terp.h"


/*-----------------------------------------------------------------------------
 * dfa.c -- Make a DFA transitio table from an NFA created with Thompson's
 * construction
 * Most of the code is from the book _Compiler Design in C_ -- This file is
 * almost identical to the code of the book.
 *---------------------------------------------------------------------------*/

/* Dtrans is the deterministic transition table, It is indexed by state number
 * along the major axis and input character along the minor axis.
 * i.e. Dtrans[state_number][input character] => next state number.
 *
 * Dstates is a list if deterministic states represented as sets of NFA states.
 * Nstates is the number of valid entries in Dtrans.*/


typedef struct dfa_state {
    unsigned group; /* group id, used by minimize() */
    bool mark; /* mark used by make_dtran() */
    char *accept; /* acception string if accept state */
    anchor_t anchor; /* anchor point if accpet state */
    set_t *set;  /* set of NFA states represented by current DFA states */
}dfa_t;

static dfa_t *Dstates; /* DFA states table */
/*----------------------------------------------------------------------------*/
static ROW *Dtrans; /* DFA transition table */
static int Nstates; /* number of DFA states */
static dfa_t *Last_marked; /* most-recently marked DFA state in Dtrans */

/*----------------------------------------------------------------------------*/
/* Prototypes for subroutines in this file */

int add_to_dstates(set_t *nfa_set, char *accepting_string, anchor_t anchor);
static int in_dstates(set_t *nfa_set);
static dfa_t *get_unmarked();
static void free_sets();
static void make_dtrans(int start);

/*----------------------------------------------------------------------------*/

/* Turns an NFA machine with indicated start state into a DFA and returns the
 * number of states in the DFA transition table. dtrans is modified to point at
 * that transition table and accept is modified to point at an array of
 * accepting states(indexed by state number).
 * dfa() discards all the memory used for the initial NFA. */
int dfa(char *(*input_func)(void), ROW *dtrans[], accept_t **accept)
{
    accept_t *accept_states;
    int i;
    int start;

    start = nfa(input_func);
    Nstates = 0;
    Dstates = (dfa_t *)calloc(MAX_DFA_STATES, sizeof(*Dstates));
    Dtrans = (ROW *)calloc(MAX_NFA_STATES, sizeof(ROW));

    if (Dtrans == NULL || Dstates == NULL) {
        fprintf(stderr, "dfa: not enough memory allocating Dstates or Dtrans\n");
        exit(1);
    }

    make_dtrans(start); /* convert the NFA to a DFA */
    free_nfa();
    
    /* reallocate memory for states/accept/... */
    Dtrans = (ROW *)realloc(Dtrans, Nstates * sizeof(ROW));
    accept_states = (accept_t *)malloc(Nstates * sizeof(*accept_states));
    if (accept_states == NULL || Dtrans == NULL) {
        fprintf(stderr, "dfa: not enough memory allocating accept_states or Dtrans.\n");
        exit(1);
    }

    for (i = Nstates-1; i >= 0; i--) {
        accept_states[i].string = Dstates[i].accept;
        accept_states[i].anchor = Dstates[i].anchor;
    }

    free(Dstates);
    *dtrans = Dtrans;
    *accept = accept_states;

    return Nstates;
}

/*----------------------------------------------------------------------------*/
/* Add a new DFA state to the Dstates array and increments the *Nstates*
 * counter. the index of the new state in the array is returned. */
int add_to_dstates(set_t *nfa_set, char *accepting_string, anchor_t anchor)
{
    int next_state;

    if (Nstates > (MAX_DFA_STATES-1)) {
        fprintf(stderr, "Too many DFA states\n");
        exit(1);
    }

    next_state = Nstates++;

    Dstates[next_state].set = nfa_set;
    Dstates[next_state].accept = accepting_string;
    Dstates[next_state].anchor = anchor;
    
    return next_state;
}

/* if there is a DFA state in Dstates array whose set is identical to nfa_set,
 * then the index of the state is returned. else -1 is returned. */
static int in_dstates(set_t *nfa_set)
{
    /* use linear search here. */
    dfa_t *p = NULL;
    for (p = &Dstates[Nstates-1]; p >= Dstates; p--) {
        if (set_is_equal(nfa_set, p->set)) {
            return (p-Dstates);
        }
    }
    return -1;
}

/* return a pointer to the next unmarked state in Dstates. If no such state
 * exists, NULL is returned.
 * Print an asterisk for each state to tell the user that the program hasn't
 * died while the table is being constructed. (this is fun :))*/
static dfa_t *get_unmarked()
{
    for (; Last_marked < &Dstates[Nstates]; Last_marked++) {
        if (!Last_marked->mark) {
            putc('*', stderr);
            fflush(stderr);
        }

        return Last_marked;
    }

    return NULL;
}

/* free the memory used for the NFA sets in all Dstate entries. */
static void free_sets()
{
    dfa_t *p;
    for (p = &Dstates[Nstates-1]; p >= Dstates; p--) {
        set_del(p->set);
    }

}

/*----------------------------------------------------------------------------*/
/* Actually perform the transformation of NFA machine to DFA transition
 * table. The resources (such as Dtrans/Dstates array) should be made available
 * before this function is called. */
static void make_dtrans(int start)
{
    set_t *nfa_set; /* set of NFA states that define the next DFA state. */
    dfa_t *current; /* state currently being expanded. */
    int next_state; /* Goto DFA state for current char, i.e. Dtrans[cur][c] */
    char *accept; /* accept string, NULL if not accepting state */

    anchor_t anchor; /* anchor point if any */
    int c; /* current input character */

    /* 1. Initialize the starting DFA state */
    nfa_set = set_new();
    set_add(nfa_set, start);

    Nstates = 1;
    Dstates[0].set = e_closure(nfa_set, &accept, &anchor);
    Dstates[0].mark = false;

    while ((current = get_unmarked()) != NULL) {
        current->mark = true;

        for (c = 0; c < MAX_CHARS; c++) {
            nfa_set = move(current->set, c);
            if (nfa_set != NULL) {
                nfa_set = e_closure(nfa_set, &accept, &anchor);
            }

            /* no outgoing transition */
            if (nfa_set == NULL) {
                next_state = F;
            } else if ((next_state = in_dstates(nfa_set)) != -1) {
                /* the GOTO state is already exist. */
                set_del(nfa_set);
            } else {
                next_state = add_to_dstates(nfa_set, accept, anchor);
            }

            Dtrans[current-Dstates][c] = next_state;
        }
    }

    /* Terminate string of *'s printed in get_unmarked(); */
    putc('\n', stderr);

    free_sets();
}
