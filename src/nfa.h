/*-----------------------------------------------------------------------------
 * nfa.h -- header file containning all the global information about NFA
 *---------------------------------------------------------------------------*/

typedef struct nfa
{
}nfa_t;

nfa_t *thompson(char *(*input_func)(void), int max_state, nfa_t **start_state);
