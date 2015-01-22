#ifndef TERP_H
#define TERP_H

#include "nfa.h"
#include "set.h"


/*----------------------------------------------------------------------------*/ 
/* Prototype for functions in this file */

int nfa(char *(*input_func)(void));
void free_nfa(void);
set_t *e_closure(set_t *old, char **accept, anchor_t *anchor);
set_t *move(set_t *old, int c);

#endif /* TERP_H */
