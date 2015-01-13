#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>

#include "nfa.h"
#include "escape.h"

/*---------------------------------------------------------------------------*/
/* token map: characters -> tokens
 * all are single character lexeme.
 * code from book _Compiler Design in C_ */
static enum token Tokmap[] = {
/* ^@  ^A  ^B  ^C  ^D  ^E  ^F  ^G  ^H  ^I  ^J  ^K  ^L  ^M  ^N */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/* ^O  ^P  ^Q  ^R  ^S  ^T  ^U  ^V  ^W  ^X  ^Y  ^Z  ^[  ^\  ^] */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/* ^^  ^_ SPACE !   !   "   #   $        %   &   ' */
    L,  L,  L,  L,  L,  L,  L,  AT_EOL,  L,  L,  L,
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
/*  @   a   b   c   d   e   f   g   h   i   j   k   l   m   n */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
/*  o   p   q   r   s   t   u   v   w   x   y   z */
    L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,
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
        } while((*Input_pos) != '\0'); /* skip over blank lines */
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
