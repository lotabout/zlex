#ifndef DFA_H
#define DFA_H

/*-----------------------------------------------------------------------------
 * dfa.h -- header file containning all the global information about DFA
 *---------------------------------------------------------------------------*/
#include "set.h"
#include "nfa.h"


#define MAX_DFA_STATES 254 /* maximum number of DFA states, states are numbered
                            * from 0 to MAX_DFA_STATES-1 */

typedef unsigned char TTYPE; /* the type of the output DFA transition table,
                              * with MAX_NFA_STATES<255, we could use unsigned
                              * char to save space used */
#define F -1 /* failure state */
#define MAX_CHARS 128 /* maximum width of DFA transition table */
typedef int ROW[MAX_CHARS]; /* one full row of Dtrans, which is itself an array,
                             * MAX_NFA_STATES elements long, of ROWs */

/*----------------------------------------------------------------------------*/
typedef struct
{
    char *string; /* accepting string, NULL if not an accept state */
    anchor_t anchor; /* anchor point. if any */
} accept_t;

/*----------------------------------------------------------------------------*/
/* External subroutines */
int dfa(char *(*input_func)(void), ROW*dtrans[], accept_t **accept); /* dfa.c */
int min_dfa(char *(*input_func)(void), ROW*[], accept_t **accept); /* minimiz.c */

#endif /* DFA_H */
