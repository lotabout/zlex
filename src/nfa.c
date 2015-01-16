#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>

#include "nfa.h"
#include "escape.h"
/*---------------------------------------------------------------------------*/
/* Debug Macros */
#ifdef DEBUG
    int Level = 0;
    #define ENTER(f) printf("%*senter %s [%c][%1.10s] \n", Level++ * 4, \
                            "", f, Lexeme, Input_pos);
    #define LEAVE(f) printf("%*sleave %s [%c][%1.10s] \n", --Level * 4, \
                            "", f, Lexeme, Input_pos);
#else
    #define ENTER(f)
    #define LEAVE(f)
#endif

/*---------------------------------------------------------------------------*/
/* Token types */
enum token {
    EOS = 1,     /* end of string      */
    ANY,         /* .                  */
    AT_BOL,      /* ^                  */
    AT_EOL,      /* $                  */
    CCL_START,   /* [                  */
    CCL_END,     /* ]                  */
    PAREN_OPEN,  /* (                  */
    PAREN_CLOSE, /* )                  */
    CURLY_OPEN,  /* {                  */
    CURLY_CLOSE, /* }                  */
    CLOSURE,     /* *                  */
    DASH,        /* -                  */
    END_OF_INPUT,/* EOF                */
    L,           /* literal characters */
    OPTIONAL,    /* ?                  */
    OR,          /* |                  */
    PLUS_CLOSE,  /* +                  */
};

/*---------------------------------------------------------------------------*/
/* lexical analyzer */
static int advance();
static inline bool match(enum token t);


/* parser */
nfa_t *machine();
nfa_t *rule();
void expr(void);
void cat_expr(void);
void factor(void);
bool first_in_cat(enum token t);
void term(void);
void dodash(void);


/*---------------------------------------------------------------------------*/
/* token map: characters -> tokens
 * all are single character lexeme.
 * code from book _Compiler Design in C_ */
static enum token Tokmap[] = {
/* ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/* ^O  ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\  ^] */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/* ^^  ^_ SPACE !   "   #   $        %   &   ' */
    L,  L,  L,  L,  L,  L,  AT_EOL,  L,  L,  L,
/*  (           )            *        +           '  -     .   */
    PAREN_OPEN, PAREN_CLOSE, CLOSURE, PLUS_CLOSE, L, DASH, ANY,
/*  /   0   1   2   3   4   5   6   7   8   9   :   ;   <   = */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/*  >   ?        */
    L,  OPTIONAL,
/*  @   A   B   C   D   E   F   G   H   I   J   K   L   M   N */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/*  O   P   Q   R   S   T   U   V   W   X   Y   Z */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/*  [          \  ]        ^     */
    CCL_START, L, CCL_END, AT_BOL,
/*  ^   `  a  b   c   d   e   f   g   h   i   j   k   l   m */
    L,  L, L, L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/*  n  o   p   q   r   s   t   u   v   w   x   y   z */
    L, L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/*  {           |   }            DEL  */
    CURLY_OPEN, OR, CURLY_CLOSE, L,
};

/*---------------------------------------------------------------------------*/
static char *Input_pos = "";    /* current position in input string */
static char *Input_str = NULL;  /* beginning of input string */
static enum token Current_tok;  /* current token */
static int  Lexeme;             /* value associated with literal */
static char *(*Input_func)() = NULL; /* function to get input string */

/*---------------------------------------------------------------------------*/
/* Lexical analyzer
 *
 * The lexical analysis process is quite simple because all single character
 * lexemes. The only complications are escape sequences and quoted strings.
 *
 * The advance() routine handles all the above as well as macro expansion
 * which are identified by "{macro name}". advance() will advances past the
 * current token and save the token in Current_tok and the corresponding
 * lexeme in Lexeme if the token is literal character. If the character is
 * escaped, Lexeme holds the actual value. Escaped characters are handled by
 * the "escape" module.
 */
static int advance()
{
    /* currently, no macro is support */
    static bool inquote = false;
    bool escaped = false;        /* if the current characer are escaped */

    if (Current_tok == EOS) {  /* Need a new line */
        if (inquote) {
            fprintf(stderr, "advance: newline in quotes\n");
            exit(1);
        }

        do {
            Input_pos = Input_func();
            if (Input_pos == NULL) {    /* end of file */
                Current_tok = END_OF_INPUT;
                goto exit;
            }
            while(isspace(*Input_pos)) { /* skip over leading spaces */
                Input_pos ++;
            }
        } while((*Input_pos) == '\0'); /* skip over blank lines */
        Input_str = Input_pos;
    }

    /* check the end of string '\0' */
    if (*Input_pos == '\0') {
        Current_tok = EOS;
        Lexeme = '\0';
        goto exit;
    }

    /* recognize tokens */
    if (*Input_pos == '"') {
        /* change the quote state. i.e. if in quote, all characters are
         * treated as plain characters */
        inquote = ~inquote;
        Input_pos ++;
        if (*Input_pos == '\0') {
            Current_tok = EOS;
            Lexeme = '\0';
            goto exit;
        }
    }

    escaped = (*Input_pos) == '\\';

    if (!inquote) {
        if (isspace(*Input_pos)) {
            Current_tok = EOS;
            Lexeme = '\0';
            goto exit;
        }
        Lexeme = escape(&Input_pos);
    } else {
        if (escaped && (Input_pos[1] == '"')) {
            Input_pos += 2;
            Lexeme = '"';
        } else {
            Lexeme = *Input_pos++;
        }
    }

    Current_tok = (escaped || inquote) ? L : Tokmap[Lexeme];

exit:
    return Current_tok;
}

static inline bool match(enum token t)
{
    return (Current_tok == t);
}

/*---------------------------------------------------------------------------*/
/* NFA parser
 * A simple recursive top-down parser that creates a Thompson NFA for a
 * regular expression.
 *
 * Now only stubs.
 * */

nfa_t *thompson(char *(*input_func)(void), int *max_state, nfa_t **start_state)
{
    Input_func = input_func;
    Current_tok = EOS;  /* load the first token */
    advance();
    return machine();
}

nfa_t *machine()
{
    ENTER("machine");
    /* machine  ::= ( rule )* END_OF_INPUT */
    nfa_t *start = NULL;

    while(!match(END_OF_INPUT)) {
        printf("machine: match rule.\n");
        start = rule();
    }

    LEAVE("machine");
    return start;
}

nfa_t *rule()
{
    ENTER("rule");
    /* rule     ::=  expr  EOS action
                  | ^expr  EOS action
                  |  expr$ EOS action */
    if (match(AT_BOL)) {
        printf("--> match at the beginning of line\n");
        advance();
    }
    expr();

    if (match(AT_EOL)) {
        printf("--> match at the end of line\n");
        advance();
    }

    if (!match(EOS)) {
        fprintf(stderr, "rule: expected EOS before action\n");
        exit(1);
    }

    while(isspace(*Input_pos)) { /* skip over blank spaces */
        Input_pos ++;
    }

    printf("action: \"%s\"\n", Input_pos);

    advance();  /* skip the EOS token */
    LEAVE("rule");
}

void expr(void)
{
    ENTER("expr");
    /* expr     ::= cat_expr ( OR cat_expr )*      ; OR has high precedence
     */
    cat_expr();
    while(match(OR)) {
        advance();
        cat_expr();
        printf("--> expr: doing OR operator\n");
    }
    LEAVE("expr");
}

void cat_expr(void)
{
    ENTER("cat_expr");
    /* cat_expr ::= (factor)+
     */
    if (first_in_cat(Current_tok)) {
        factor();
        printf("--> cat_expr = (factor)+\n");
    } else {
        fprintf(stderr, "cat_expr: expecting a factor.\n");
        exit(1);
    }

    while(first_in_cat(Current_tok)) {
        factor();
        printf("--> cat_expr = (factor)+\n");
    }
    LEAVE("cat_expr");
}

bool first_in_cat(enum token t)
{
    /* check if a token is in the FIRST set of cat_expr.
     * i.e. check if token t can be the start token of factor.
     */
    switch (t) {
        case CCL_START:
        case CLOSURE:
        case PAREN_OPEN:
        case L:
            return true;
            break;
        default:
            return false;
            break;
    }
    return false;
}

void factor(void)
{
    ENTER("factor");
    /* factor   ::= term* | term+ | term? | term
     */
    term();
    if (match(CLOSURE) || match(PLUS_CLOSE) || match(OPTIONAL)) {
        printf("--> term* || term+ || term? \n");
        advance();
    } else {
        printf("--> term\n");
    }
    LEAVE("factor");
}

void term(void)
{
    ENTER("term");
    /* term     ::= [string] | [^string] | [] | [^] | . | (expr) | <character>
     */
    if (match(PAREN_OPEN)) {
        expr();
        if (match(PAREN_CLOSE)) {
            advance();
        } else {
            fprintf(stderr, "term: missing parentheses\n");
            exit(1);
        }
    } else {
        /* match [string] [^string] */
        if (match(CCL_START)) {
            advance();
            if (match(AT_BOL)) {
                printf("negtive character class\n");
                advance();
            }

            /* match strings */
            dodash();
            if (match(CCL_END)) {
                advance();
            } else {
                fprintf(stderr, "term: ] not matched.\n");
            }

        } else if (match(ANY)) {
            printf("match any character\n");
            advance();
        } else {
            printf("match character: %c\n", Lexeme);
            advance();
        }
    }

    LEAVE("term");
}

void dodash(void)
{
    /* match the string compnent in [string] or [^string]
     * note that a-z are interpret as abcd...z etc. */
    int first = 0;
    while(!match(CCL_END)) {
        if (match(DASH)) {
            advance();
            printf("matching range: %c-%c\n", first, Lexeme);
        } else {
            first = Lexeme;
            printf("matching character %c\n", Lexeme);
        }
        advance();
    }
}

/*---------------------------------------------------------------------------*/
/* Memory management -- state and strings
 * states: allocate a large pool and manage allocations and destruction
 * strings: routine save() to allocate strings and embed line number in it. */

const int MAX_NFA_STATES = 788;     /* max states in a NFA machine */
nfa_t *NFA_states; /* pointer to the allocated pool of states */
int Num_states = 0;     /* number of states in NFA machine */
int Next_alloc = 0;     /* Index of next elements in the array */


#define SSIZE 32                /* stack size */
nfa_t *Sstack[SSIZE];           /* stack to save discarded pointer */
nfa_t **Sp = &Sstack[-1];       /* stack pointer */

#define PUSH(x) (*++Sp = (x))   /* push x onto the stack */
#define POP()   (*Sp--)         /* get x from the top of the stack */
#define STACK_EMPTY() (Sp < Sstack)     /* true if stack is empty */
#define STACK_FULL()  ((Sp-Sstack+1) >= SSIZE) /* true if stack is full */

/* Allocate new NFA state */
static nfa_t *new_state(void)
{
    static bool first_time = true;
    if (first_time) {
        NFA_states = (nfa_t *)calloc(MAX_NFA_STATES, sizeof(*NFA_states));
        if (NFA_states == NULL) {
            fprintf(stderr, "new_state: not enough memroy.\n");
            exit(1);
        }
        first_time = false;
        Sp = &Sstack[-1];
    }

    if (++Num_states >= MAX_NFA_STATES) {
        fprintf(stderr, "new_state: MAX NFA states reached.\n");
        exit(1);
    }

    nfa_t *rval;
    rval = !STACK_EMPTY() ? POP() : &NFA_states[Next_alloc++];
    rval->edge = EPSILON;

    return rval;
}

/* discard a NFA state */
static void discard_state(nfa_t *state)
{
    /* note that the state might contain a bitset, we'll free it before push
     * it to the stack
     * also, *state* might contain accept strings, they are saved in the pool
     * allocated by save(), if we discard an accepting state, the
     * corresponding part in the pool will leak(no way to fetch any more),
     * though we normally do not discard an accepting state */
    assert(state != NULL);

    if (state->bitset != NULL) {
        set_del(state->bitset);
        state->bitset = NULL;
    }
    memset(state, 0, sizeof(*state));
    state->edge = EMPTY;
    if (STACK_FULL()) {
        fprintf(stderr, "discard_state: stack full, abort.\n");
        exit(1);
    }
    PUSH(state);
}

/*---------------------------------------------------------------------------*/
/* Just like what we do to NFA states, we'll save accepting strings in a large
 * pool of memory, also, we'll embed the line number of *str* into the saved
 * string, so that ((int*)(p->accept))[-1] is the line number. */
static char *save(char *str)
{
}
