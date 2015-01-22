#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <limits.h>

#include "nfa.h"
#include "escape.h"
#include "hash.h"

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
static nfa_t *machine();
static nfa_t *rule();
static void expr(nfa_t **start, nfa_t **end);
static void cat_expr(nfa_t **start, nfa_t **end);
static void factor(nfa_t **start, nfa_t **end);
static bool first_in_cat(enum token t);
static void term(nfa_t **start, nfa_t **end);
static void dodash(set_t *set);

/* memory management */
static nfa_t *new_state(void);
static void discard_state(nfa_t *state);
static void assign_state(nfa_t **dst, const nfa_t *src);
static char *save(char *str);

/* macro support */
static char *expand_macro(char **input);

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
 *
 * Macros are support by manage input source in a stack.
 */
#define INPUT_SOURCE_STACK_SIZE 32
static int advance()
{
    /* currently, no macro is support */
    static bool inquote = false;
    bool escaped = false;        /* if the current characer are escaped */
    static char *stack[INPUT_SOURCE_STACK_SIZE];
    static char **sp = stack-1;

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
    while (*Input_pos == '\0') {
        /* try to restore input sources */
        if (sp >= stack) {
            Input_pos = *sp--;
        }
    }

    if (*Input_pos == '\0') {
        Current_tok = EOS;
        Lexeme = '\0';
        goto exit;
    }

    /* check for macro, might be nested */
    if (!inquote) {
        while (*Input_pos == '{') {
            *++sp = Input_pos; /* save current input source, will be
                                * modified by expand_macro() */
            Input_pos = expand_macro(sp);
        }
    }



    /* recognize tokens */
    if (*Input_pos == '"') {
        /* change the quote state. i.e. if in quote, all characters are
         * treated as plain literals */
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
/* Memory management -- state and strings
 * states: allocate a large pool and manage allocations and destruction
 * strings: routine save() to allocate strings and embed line number in it. */

const int MAX_NFA_STATES = 788;     /* max states in a NFA machine */
static nfa_t *NFA_states; /* pointer to the allocated pool of states */
static int Num_states = 0;     /* number of states in NFA machine */
static int Next_alloc = 0;     /* Index of next elements in the array */


#define SSIZE 32                /* stack size */
static nfa_t *Sstack[SSIZE];           /* stack to save discarded pointer */
static nfa_t **Sp = &Sstack[-1];       /* stack pointer */

#define PUSH(x) (*++Sp = (x))   /* push x onto the stack */
#define POP()   (*Sp--)         /* get x from the top of the stack */
#define STACK_EMPTY() (Sp < Sstack)     /* true if stack is empty */
#define STACK_FULL()  ((Sp-Sstack+1) >= SSIZE) /* true if stack is full */

/* Allocate new NFA state */
static nfa_t *new_state(void)
{
    static bool first_time = true;
    if (first_time) {
        first_time = false;
        NFA_states = (nfa_t *)calloc(MAX_NFA_STATES, sizeof(*NFA_states));
        if (NFA_states == NULL) {
            fprintf(stderr, "new_state: not enough memroy.\n");
            exit(1);
        }
        Sp = &Sstack[-1];

        /* assign IDs to NFA states, starting from 0. */
        int i;
        for (i = 0; i < MAX_NFA_STATES; i++) {
            NFA_states[i].nfa_id = i;
        }

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
    int id = state->nfa_id;  /* recover ID */
    memset(state, 0, sizeof(*state));
    state->edge = EMPTY;
    state->nfa_id = id;
    if (STACK_FULL()) {
        fprintf(stderr, "discard_state: stack full, abort.\n");
        exit(1);
    }
    PUSH(state);
}

/* destory all the states in a machine */
void destory_thompson(void)
{
    free(NFA_states);
    NFA_states = NULL;
}

/* assign src to dst, dst's resources are freed. */
static void assign_state(nfa_t **dst, const nfa_t *src)
{
    int id = (*dst)->nfa_id;
    if ((*dst)->bitset != NULL) {
        set_del((*dst)->bitset);
    }

    memcpy(*dst, src, sizeof(*src));
    (*dst)->nfa_id = id;
}

/*---------------------------------------------------------------------------*/
/* Just like what we do to NFA states, we'll save accepting strings in a large
 * pool of memory, also, we'll embed the line number of *str* into the saved
 * string, so that ((int*)(p->accept))[-1] is the line number. */
const int MAX_SAVED_STRING = (10 * 1024);
static char *save(char *str)
{
    assert(str != NULL);

    static bool first_time = true;
    static int *strings = NULL;
    static int *savep = NULL; /* current position in string pool */

    if (first_time) {
        /* allocate the string pool */
        savep = strings = (int *)malloc(MAX_SAVED_STRING);
        if (strings == NULL) {
            fprintf(stderr, "save: not enough memory allocating string pool\n");
            exit(1);
        }
        first_time = false;
    }

    *savep++ = 0;   /* save the line number, TODO: involve the actual line
                       number */

    int len = strlen(str);
    if ((char*)savep+len+1 >= (char*)strings+MAX_SAVED_STRING) {
        fprintf(stderr, "save: max size of pool exceeded.\n");
        exit(1);
    }
    strcpy((char*)savep, str);
    char *rval = (char*)savep;
    len += 2;  /* count for the ending '\0' of *str* and move past it */
    savep += (len/sizeof(int)) + ((len % sizeof(int) == 0 ? 0 : 1));

    return rval;
}

/*---------------------------------------------------------------------------*/
/* NFA parser
 * A simple recursive top-down parser that creates a Thompson NFA for a
 * regular expression.
 *
 * Now only stubs.
 * */

/* construct NFA machine. return the state array. */
nfa_t *thompson(char *(*input_func)(void), nfa_t **start, int *max_state)
{
    nfa_t *rval = NULL;
    Input_func = input_func;
    Current_tok = EOS;  /* load the first token */
    advance();
    *max_state = Next_alloc;
    *start = machine();
    return NFA_states;
}

static nfa_t *machine()
{
    ENTER("machine");
    /* machine  ::= ( rule )+ END_OF_INPUT 
     * A machine should have at least one rule. */
    nfa_t *start = NULL;
    nfa_t *p = NULL;

    p = start = new_state(); /* remember that new state's edge is EPSILON */
    p->next1 = rule();

    while(!match(END_OF_INPUT)) {
        /* a machine is a OR of several rules */
        p->next2 = new_state();
        p = p->next2;
        p->next1 = rule();
    }

    LEAVE("machine");
    return start;
}

static nfa_t *rule(void)
{
    ENTER("rule");
    /* rule     ::=  expr  EOS action
                  | ^expr  EOS action
                  |  expr$ EOS action */
    nfa_t *start = NULL;
    nfa_t *end = NULL;
    anchor_t anchor = NONE;

    if (match(AT_BOL)) {
        start = new_state();
        start->edge = '\n';
        anchor = START;
        advance();
        expr(&start->next1, &end);
    } else {
        expr(&start, &end);
    }

    if (match(AT_EOL)) {
        /* pattern followed by a \r or \n, use a character class */
        advance();

        end->next1 = new_state();
        end->edge = CCL;
        end->bitset = set_new();
        if (end->bitset == NULL) {
            fprintf(stderr, "rule(): not enough memory allocating character class\n");
            exit(1);
        }
        set_add(end->bitset, '\n');

        /* TODO: if not in *NIX, add '\r' as well */
        end = end->next1;
        anchor |= END;
    }

    if (!match(EOS)) {
        fprintf(stderr, "rule: expected EOS before action\n");
        exit(1);
    }

    while(isspace(*Input_pos)) { /* skip over blank spaces */
        Input_pos ++;
    }

    end->accept = save(Input_pos);
    end->anchor = anchor;

    advance();  /* skip the EOS token */
    LEAVE("rule");
    return start;
}

static void expr(nfa_t **start, nfa_t **end)
{
    ENTER("expr");
    /* expr     ::= cat_expr ( OR cat_expr )*      ; OR has high precedence
     */

    nfa_t *e2_start = NULL;    /* save state of the (OR cat_expr) part */
    nfa_t *e2_end = NULL;
    nfa_t *p = NULL;

    cat_expr(start, end);

    while(match(OR)) {
        advance();
        cat_expr(&e2_start, &e2_end);

        /* branch for the start states */
        p = new_state();
        p->next1 = *start;
        p->next2 = e2_start;

        *start = p;

        /* merge the end states */
        p = new_state();
        (*end)->next1 = p;
        e2_end->next1 = p;
        *end = p;
    }
    LEAVE("expr");
}

static void cat_expr(nfa_t **start, nfa_t **end)
{
    ENTER("cat_expr");
    /* cat_expr ::= (factor)+
     *          CAT
     * o --> o  ===>   o --> o
     * becomes
     * o --> o --> o
     *
     * i.e. discard e2_start while concatenating.
     * must have at least one factor.
     */
    nfa_t *e2_start = NULL;
    nfa_t *e2_end = NULL;

    if (first_in_cat(Current_tok)) {
        factor(start, end);
    } else {
        fprintf(stderr, "CAT_EXPR: expecting a factor.\n");
        exit(1);
    }

    while(first_in_cat(Current_tok)) {
        factor(&e2_start, &e2_end);

        assign_state(end, e2_start);
        discard_state(e2_start);
        *end = e2_end;
    }
    LEAVE("cat_expr");
}

static bool first_in_cat(enum token t)
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

static void factor(nfa_t **start, nfa_t **end)
{
    ENTER("factor");
    /* factor   ::= term* | term+ | term? | term
     *        +---------+
     * o -->  |o  -->  o| --> o
     *        +---------+
     * `-new_s `-start `-end  `-new_end
     */
    nfa_t *new_start = NULL;
    nfa_t *new_end = NULL;

    term(start, end);
    if (match(CLOSURE) || match(PLUS_CLOSE) || match(OPTIONAL)) {
        new_start = new_state();
        new_end = new_state();
        new_start->next1 = *start;
        (*end)->next1 = new_end;

        if (match(CLOSURE) || match(OPTIONAL)) {
            new_start->next2 = new_end;
        }
        if (match(CLOSURE) || match(PLUS_CLOSE)) {
            (*end)->next2 = *start;
        }

        *start = new_start;
        *end = new_end;
        advance();
    }
    LEAVE("factor");
}

static void term(nfa_t **start, nfa_t **end)
{
    ENTER("term");
    /* term     ::= [string] | [^string] | [] | [^] | . | (expr) | <character>
     */
    if (match(PAREN_OPEN)) {
        advance();
        expr(start, end);
        if (match(PAREN_CLOSE)) {
            advance();
        } else {
            fprintf(stderr, "term: missing parentheses\n");
            exit(1);
        }
    } else {
        /* match [string] [^string] */
        nfa_t *new_start = NULL;
        *start = new_start = new_state();
        *end = (*start)->next1 = new_state();

        if (match(CCL_START)) {
            advance();

            new_start->edge = CCL;
            new_start->bitset = set_new();
            if (new_start->bitset == NULL) {
                fprintf(stderr, "term: not enough memory allocating bitset.\n");
                exit(1);
            }

            bool negtive = false;
            if (match(AT_BOL)) {
                negtive = true;
                advance();
            }

            /* match strings */
            dodash(new_start->bitset);
            if (match(CCL_END)) {
                advance();
            } else {
                fprintf(stderr, "term: ] not matched.\n");
            }

            if (negtive) {
                set_invert(new_start->bitset);
            }

        } else if (match(ANY)) {
            new_start->edge = CCL;
            new_start->edge = CCL;
            new_start->bitset = set_new();
            if (new_start->bitset == NULL) {
                fprintf(stderr, "term: not enough memory allocating bitset.\n");
                exit(1);
            }

            set_add(new_start->bitset, '\n');
            /* TODO: if not in UNIX, add '\r' as well */
            set_invert(new_start->bitset);
            advance();
        } else {
            new_start->edge = Lexeme;
            advance();
        }
    }

    LEAVE("term");
}

static void dodash(set_t *set)
{
    /* match the string compnent in [string] or [^string]
     * note that a-z are interpret as abcd...z etc. */
    int first = 0;

    if (match(DASH)) { /* treat [-...] as literal '-' */
        set_add(set, '-');
        advance();
    }

    while(!match(CCL_END)) {
        if (match(DASH)) {
            advance();
            if (match(CCL_END)) { /* treat [...-] as literal '-' */
                set_add(set, '-');
            } else {
                for (; first <= Lexeme; first++) {
                    set_add(set, first);
                }
            }
        } else {
            first = Lexeme;
            set_add(set, first);
        }
        advance();
    }
}

/*---------------------------------------------------------------------------*/
/* Macro support
 *
 * Macro is a simple replacement of text. thus even nested macros should follow
 * the construt rule after expansion.
 * 
 * Note that the memory allocated for macros might not be freed at
 * all, they're destoryed after the program exit.
 * This design works because zlex is always one-run for any input. */

static hash_t *Macros; /* symbol table for macro definitions */
const int MAX_MACRO_NUM = 31;

/* functions needed by hash table, only wrappers. */
static unsigned hash_str(const void *str)
{
    return hash_sdbm((const char *)str);
}

static int str_cmp(const void *a, const void *b)
{
    return strcmp((const void *)a, (const void *)b);
}

static void destory(void *key, void *val)
{
    if (key) {
        free(key);
    }
    if (val) {
        free(val);
    }
}

static void print_a_macro(const void *key, const void *val)
{
    printf("%-16s--[%s]--\n", (char *)key, (char *)val);
}

/* parse a macro definition and add it to the table
 * If two macros comes in a row, the second one takes precedence.
 * A macro definition has the following form 
 *   name <whitespace> definition [<whitespace>]* */
void new_macro(const char *def)
{
    static bool first_time = true;
    if (first_time) {
        Macros = hash_new(MAX_MACRO_NUM, hash_str, str_cmp);
        if (Macros == NULL) {
            fprintf(stderr, "new_macro: not enough memory allocating macro table");
            exit(1);
        }

        first_time = false;
    }


    const char *name = NULL;
    /* 1. isolate name with definition */
    for (name = def; *def && !isspace(*def); def++) {
        /* pass */
    }

    if (*def == '\0') {
        fprintf(stderr, "new_macro, missing definition part for macro");
        exit(1);
    }

    int name_len = def - name + 1;

    const char *text = NULL;
    const char *edef = NULL; /* end of definition part */

    /* 2. first skip up whitespaces to the definition body */
    while (isspace(*def)) {
        def++;
    }

    text = def;

    /* 3. we should skip the trailing spaces(including newline) */
    while (*def) {
        if (!isspace(*def)) {
            def++;
        } else {
            for (edef = def++; isspace(*def); def++) {
                /* pass */
            }
        }
    }

    if (edef) {
        if (*(edef-1) == '\\') {
            /* if the trailing characters are <\ >, we should not remove it */
            edef++;
        }

        def = edef;
    }

    int text_len = def - text + 1;

    /* 4. allocating memory for name/definition */
    char *name_p = (char *)malloc(name_len * sizeof(*name_p));
    char *text_p = (char *)malloc(text_len * sizeof(*text_p));
    if (name_p == NULL || text_p == NULL) {
        fprintf(stderr, "new_macro: not enough memory allocating memory for name and definition");
        exit(1);
    }

    strncpy(name_p, name, name_len-1);
    name_p[name_len-1] = '\0';
    strncpy(text_p, text, text_len-1);
    text_p[text_len-1] = '\0';
    hash_add(Macros, name_p, text_p);
}

#define MAX_BUF 8192
/* scan the input, recogize a macro, expand it and return, modify input
 * accordingly. macros names are recognized as <{name}>. the input pointer is
 * moved past the closing '}' */
static char *expand_macro(char **input)
{
    static char key[MAX_BUF];
    char *end = NULL;
    char *rval = NULL;
    end = strchr(++(*input), '}');
    if (end == NULL) {
        fprintf(stderr, "expand_macro: bad macro.");
        exit(1);
    } else {
        /* currently, not modifying the input source. */

        strncpy(key, *input, end-*input);
        key[end-*input] = '\0';
        rval = hash_get(Macros, key);

        if (rval == NULL) {
            fprintf(stderr, "expand_macro: no macro definition for '%s'.", *input);
            exit(1);
        }

        end++;
        *input = end;
        return rval;
    }

    return "ERROR"; /* should not reach here */
}

/* print all macros to the stdout */
void printmacs()
{
    if (Macros == NULL) {
        printf("No macros!");
    } else {
        printf("Macros:\n");
        hash_print(Macros, print_a_macro);
    }
}
