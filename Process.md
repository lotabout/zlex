This file will serve as the record of the developing process. And design docs.

### Jan 19 2015
Now the construction of NFA works fine. It's time to convert it to DFA. Before
that I need to check out the book for some theory.

Following the book, we will first implement a machine to interpret NFA
machines.

- [ ] assign state numbers to NFA states (for simplicity)
- [ ] interpreting NFA.

### Jan 18 2015
The hash table is done, now we would add macro support. The macro
table is constructed in zlex's lexical analyzer phase. The macro
support for NFA is expand macro if a macro appers in a rule.

- [X] macro support functions.
- [X] macro support in NFA lexical analyzer: advance().

### Jan 17 2015
Now the basic NFA processing routine is done. The next step would be adding
macro support. Note that the macros are added by zlex in preprocessing phase,
similar to constructing symbol table in lexical analyzer in normal compilers.
We will need a new libary: hash tables, if we want to access the macros as
fast as possible.

- [X] library: hash table.

Now I am having trouble deciding the APIs for hash table, mainly
because of two reasons:
1. The APIs of book _Compiler Design in C_ is too specific, made
   assumption about the structure of the items is the hash
   table. Which embeds value into keys.
2. If we made the hash table general enough, the user of hash table
   would have to do memory management all by themselves. Because the
   items in hash table should be `void *`.

Finally decided to choose plan 2.

### Jan 16 2015
The next step is the memory management of NFA states. According to the text
book, `malloc` and `free` is inefficient if we will frequently allocate and
destory objects. Thus we want to allocate a large pool of memory first and
handle the allocation and destory all by ourselves. This routine should be as
simple as possible to maximize efficiency.

- [X] Memory Management of NFA states.
- [X] Memory Management of accept strings.

Next, we should integrate the NFA states to replace the parser stubs.
- [X] replace stubs with real NFAs in the parser.

Now we can actually generate NFA machine, it is time to add routine to test
the machine, For now we want to add a routine to print out the NFA.
- [X] printnfa routine.

### Jan 15 2015
To represent character classes(CCL), we will need SETs. As we will also need
SET when converting NFA to DFA, so we will try to implement a general SET
routine that contains only non-negtive integer states.

- [X] library SET.

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
