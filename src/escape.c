#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include "escape.h"

/*---------------------------------------------------------------------------*/
static inline bool is_oct_digit(char c)
{
    return (c >= '0') && (c <= '7');
}

static inline int hex_to_val(char c)
{
    return (isdigit(c) ? c-'0' : (toupper(c)-'A')+10) & 0xF;
}

static inline int oct_to_val(char c)
{
    return (c-'0') & 0x7;
}

static inline int parse_hex(char **input)
{
    /* *input must point to at least one hex digit, otherwise -1 is returned. */
    int rval = 0;
    if (isxdigit(**input)) {
        rval = hex_to_val(**input);
        (*input)++;
    } else {
        return -1;
    }

    if (isxdigit(**input)) {
        rval = (rval << 4) | hex_to_val(**input);
        (*input)++;
    }

    return rval & 0xFF;
}

static inline int parse_oct(char **input)
{
    /* *input must point to at least one oct digit, otherwise -1 is returned. */
    int rval = 0;
    if (is_oct_digit(**input)) {
        rval = oct_to_val(**input);
        (*input)++;
    } else {
        return -1;
    }

    if (is_oct_digit(**input)) {
        rval = (rval << 3) | oct_to_val(**input);
        (*input)++;
    }

    if (is_oct_digit(**input)) {
        rval = (rval << 3) | oct_to_val(**input);
        (*input)++;
    }

    return rval & 0xFF;
}


/*---------------------------------------------------------------------------*/
/* return escaped characters
 * The input position might be modified. */
int escape(char **input)
{
    /* Map escape sequences into their equivalent symbols. Return the equivalent
     * ASCII character. *s is advanced past the escape sequence. If no escape
     * sequence is present, the current character is returned and the string
     * is advanced by one. The following are recognized:
     *
     *	\b	backspace
     *	\f	formfeed
     *	\n	newline
     *	\r	carriage return
     *	\s	space
     *	\t	tab
     *	\e	ASCII ESC character ('\033')
     *	\DDD	number formed of 1-3 octal digits
     *	\xDD	number formed of 1-2 hex digits
     *	\^C	C = any letter. Control code
     *
     *	The above table is copied from the resource of book _Compiler Design
     *	in C_, I'll implement it by myself. Note that the oct/hex value are
     *	limited to [0~255].
     */

    int rval = 0;

    if (**input != '\\') {
        rval = **input;
        goto exit;
    }

    (*input)++;     /* skip the '\\' character */
    switch (toupper(**input)) {
        case '\0':  /* end of input */
            rval = '\\';
            break;
        case 'B':
            rval = '\b';
            break;
        case 'F':
            rval = '\f';
            break;
        case 'N':
            rval = '\n';
            break;
        case 'R':
            rval = '\r';
            break;
        case 'S':
            rval = ' ';
            break;
        case 'T':
            rval = '\t';
            break;
        case 'E':
            rval = '\033';
            break;
        case '^':
            (*input)++;
            rval = toupper(**input) - '@';
            break;
        case 'X':
            (*input)++;
            rval = parse_hex(input);
            (*input)--;
            break;
        default:
            if (!is_oct_digit(**input)) {
                rval = **input;
            } else {
                rval = parse_oct(input);
                (*input)--;
            }
            break;
    }

exit:
    (*input)++;
    return rval;
}

