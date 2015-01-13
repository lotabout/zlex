/*-----------------------------------------------------------------------------
 * nfa.h -- header file containning all the global information about NFA
 *---------------------------------------------------------------------------*/

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
