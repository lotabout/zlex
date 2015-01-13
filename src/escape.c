#include "escape.h"

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
     *	\xDDD	number formed of 1-3 hex digits
     *	\^C	C = any letter. Control code
     *	The above table is copied from the resource of book _Compiler Design
     *	in C_, I'll implement it by myself.
     */

    return 0;
}

