#ifndef NFA_H
#define NFA_H
/*-----------------------------------------------------------------------------
 * nfa.h -- header file containning all the global information about NFA
 *---------------------------------------------------------------------------*/
#include "set.h"

/* anchor of regular expression */
typedef enum {
    NONE = 0,
    START = 1,
    END = (1 << 1),
    BOTH = (START | END),
} anchor_t;

/* struct for NFA state */
typedef struct nfa
{
    int edge; /* label for edge: character, CCL, EMPTY or EPSILON */

    set_t *bitset;  /* set to store character class(CCL) */

    struct nfa *next1; /* next state (or NULL if none) */
    struct nfa *next2; /* another next state, not NULL only if edget ==
                          EPSILON */
    char *accept;   /* action string for accepting state. NULL if not
                       accepting state */
    anchor_t anchor;    /* anchor of regular expression */
} nfa_t;

#define EPSiLON -1
#define CCL -2
#define EMPTY -3

/*---------------------------------------------------------------------------*/
nfa_t *thompson(char *(*input_func)(void), int max_state, nfa_t **start_state);

#endif /* end of include guard: NFA_H */
