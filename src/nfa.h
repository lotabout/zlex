#ifndef NFA_H
#define NFA_H
/*-----------------------------------------------------------------------------
 * nfa.h -- header file containning all the global information about NFA
 *---------------------------------------------------------------------------*/
#include "set.h"

/*---------------------------------------------------------------------------*/
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
    int nfa_id;     /* ID of nfa state */
} nfa_t;

#define EPSILON -1
#define CCL -2
#define EMPTY -3

/*---------------------------------------------------------------------------*/
/* extern const int MAX_NFA_STATES;     /\* max states in a NFA machine *\/ */
#define MAX_NFA_STATES 788     /* max states in a NFA machine */

/* construct NFA machine. return the state array. */
nfa_t *thompson(char *(*input_func)(void), nfa_t **start, int *max_state);

/* free all the resources allocated by calling thompson() */
void destory_thompson(void);


typedef enum {
    NFA_PLAIN,      /* plain text output */
    NFA_GRAPHVIZ,   /* graphviz output */
} nfa_print_t;

void print_nfa(nfa_t *nfa, nfa_print_t type);

/*---------------------------------------------------------------------------*/
/* macro support */
/* parse a macro definition and add it to the table */
void new_macro(const char *def);

/* print all macros to the stdout */
void printmacs();

#endif /* end of include guard: NFA_H */
