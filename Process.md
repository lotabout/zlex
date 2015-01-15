This file will serve as the record of the developing process. And design docs.

### Jan 15 2015
To represent character classes(CCL), we will need SETs. As we will also need
SET when converting NFA to DFA, so we will try to implement a general SET
routine that contains only non-negtive integer states.

- [ ] library SET.

### Jan 13 2015
The key point here is to first parse regular expressions into NFA machines. So
the first part of the develop will be adding NFA support.

First comes with the grammer of lex rules(from book _Compiler design in C_ and
modified a little bit)
```
machine  ::= ( rule )* END_OF_INPUT

rule     ::=  expr  EOS action 
           | ^expr  EOS action  ; expression anchored at the beginning of line
           |  expr$ EOS action  ; expression anchored at the end of line

actions  ::= <tabs> <string of characters>  ; i.e. all that remains
           | epsilon                        ; i.e. nothing

expr     ::= cat_expr ( OR cat_expr )*      ; OR has high precedence

cat_expr ::= (factor)+

factor   ::= term* | term+ | term? | term

term     ::= [string] | [^string] | [] | [^] | . | (expr) | <character>

string   ::= <chracter>+
```
note that '[]' and '[^]' are non-standard. '[]' matches white spaces and '[^]'
matches everything but white spaces.

1. [X] I would start with the input procedure which converts the input string
into a tokens.
2. [X] Basic Parser routines.
