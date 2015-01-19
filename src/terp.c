/*-----------------------------------------------------------------------------
 * terp.c
 * Interpret NFA machines
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include "nfa.h"
#include "set.h"

/*----------------------------------------------------------------------------*/ 
/* Prototype for functions in this file */

int nfa(char *(*input_func)(void));
void free_nfa(void);
set_t *e_closure(set_t *old);
set_t *move(set_t *old);

/*----------------------------------------------------------------------------*/ 

