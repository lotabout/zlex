#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdarg.h>

#include "set.h"

/*-----------------------------------------------------------------------------
 * Implementation file fo library set 
 *
 * The map is like:
 *
 * -------------+------------------+--------------+---------
 * |15 ... 2 1 0|31     ...   17 16|    ...  33 32| ...
 * -------------+------------------+--------------+---------
 * `---set->map
 *
 *---------------------------------------------------------------------------*/

#define SET_EQUAL 0
#define SET_INTERSECT 1
#define SET_DISJOINT 2

const int _BITS_IN_WORD = sizeof(_SETTYPE) * 8;
/*---------------------------------------------------------------------------*/
/* Helper Functions */


/* enlarge a set's map to a certain words 
 * need: the number of _SETTYPE needed. */
static void enlarge(set_t *set, size_t need)
{
    if (set == NULL || set->nwords >= need) {
        return;
    }

    _SETTYPE *new_map = (_SETTYPE *)malloc(need * sizeof(*new_map));
    if (new_map == NULL) {
        fprintf(stderr, "enlarge: not enough memory allocating new map\n");
        exit(1);
    }
    memcpy(new_map, set->map, set->nwords * sizeof(_SETTYPE));
    memset(new_map+set->nwords, 0, (need-set->nwords) * sizeof(_SETTYPE));

    if (set->map != set->defmap) {
        free(set->map);
    }

    set->map = new_map;
    set->nwords = need;
    set->nbits = set->nwords * _BITS_IN_WORD;
}

/* manipulate *bit* in *set* with *op* */
static bool dobit(set_t *set, int bit, char op)
{
    int row = bit / _BITS_IN_WORD;
    _SETTYPE mask = 1 << (bit % _BITS_IN_WORD);
    switch (op) {
        case '|':   /* set the bit */
            set->map[row] |= mask;
            break;
        case '&':   /* clear the bit */
            set->map[row] &= ~mask;
            break;
        case '=':   /* check if it is set */
            return (set->map[row] & mask) != 0;
            break;
        default:
            break;
    }
}

/* perform binary operation in two set depending on *op* */
static bool set_op(set_t *dst, set_t *src, char op)
{
    /* 1. size(dst) should >= size(src) */
    if (dst->nwords < src->nwords) {
        enlarge(dst, src->nwords);
    }

    _SETTYPE *s = src->map;
    _SETTYPE *d = dst->map;
    size_t size_src = src->nwords;
    size_t tail = dst->nwords - src->nwords; /* dst is bigger */

    switch (op) {
        case '|':   /* union */
            while(size_src-- > 0) {
                *d++ |= *s++;
            }
            break;
        case '&':   /* intersect */
            while(size_src-- > 0) {
                *d++ &= *s++;
            }
            while(tail > 0) {
                *d++ = 0;
            }
            break;
        case '-':   /* difference */
            while(size_src-- > 0) {
                *d = ((*d)^(*s)) & (*d);
                d++;
                s++;
            }
            break;
        case '=':   /* assignment */
            while(size_src-- > 0) {
                *d++ = *s++;
            }
            while(tail-- > 0) {
                *d++ = 0;
            }
            break;
        default:
            break;
    }
}

/* test the relation between two set:
 * return:
 * SET_EQUAL if two set are equal
 * SET_INTERSECT if at least they have at least one common element
 * SET_DISJOINT if no common element */
static bool set_test(set_t *dst, set_t *src)
{
    if (src->nwords > dst->nwords) {
        /* make sure that len(s) <= len(dst) */
        set_t *tmp = src;
        src = dst;
        dst = src;
    }

    _SETTYPE *s = src->map;
    _SETTYPE *d = dst->map;

    size_t size_src = src->nwords;
    size_t tail = dst->nwords - src->nwords;

    int rval = SET_EQUAL;

    for (; size_src > 0; size_src--, s++, d++) {
        if (*s != *d) {
            /* now the set if not equal */
            if ((*s & *d) != 0) {
                return SET_INTERSECT;
            } else {
                /* cannot decide yet */
                rval = SET_DISJOINT;
            }
        }
    }

    for (; tail > 0; tail--, d++) {
        if (*d != 0) {
            /* no matter it was DISJOINT or EQUAL, now it is disjoint */
            return SET_DISJOINT;
        }
    }

    return rval;
}

/*---------------------------------------------------------------------------*/
/* generate a new set, abort if no enough memory. */
set_t *set_new(void)
{
    set_t *set = (set_t *)malloc(sizeof(*set));
    if (set == NULL) {
        fprintf(stderr, "set_new: not enough memory allocating set object\n");
        exit(1);
    }
    memset(set, 0, sizeof(*set));
    set->nwords = _DEFWORDS;
    set->nbits  =  _DEFWORDS * _BITS_IN_WORD;
    set->map = set->defmap;

    return set;
}

/* destory a set, not the you should set the pointer to NULL after the set is
 * deleted */
void set_del(set_t *old_set)
{
    if (old_set->map != old_set->defmap) {
        free(old_set->map);
    }

    free(old_set);
}

/* duplicate a set */
set_t *set_dup(set_t *old_set)
{
    set_t *new_set = (set_t *)malloc(sizeof(*new_set));
    if (new_set == NULL) {
        fprintf(stderr, "set_dup: not enough memory allocating set object\n");
        exit(1);
    }
    memset(new_set, 0, sizeof(*new_set));
    new_set->nwords = old_set->nwords;
    new_set->nbits = old_set->nbits;

    if (old_set->map == old_set->defmap) {
        new_set->map = new_set->map;
    } else {
        new_set->map = (_SETTYPE *)malloc(sizeof(_SETTYPE)* old_set->nwords);
        if (new_set == NULL) {
            fprintf(stderr, "set_dup: not enough memory allocating set object\n");
            exit(1);
        }
    }
    memcpy(new_set->map, old_set->map, old_set->nwords * sizeof(_SETTYPE));
    return new_set;
}

/* add members to the set, negtive number counts for the end of input */
bool set_add_members(set_t *set, int member, ...)
{
    va_list ap;
    va_start(ap , member);

    while(member >= 0) {
        if (member >= set->nbits) {
            size_t need = (member / _BITS_IN_WORD) + 1;
            enlarge(set, need);
        }
        dobit(set, member, '|');
        
        member = va_arg(ap, int);
    }

    va_end(ap);
    return true;
}

/* remove members from set, negtive number counts for the end of input*/
bool set_remove_members(set_t *set, int member, ...)
{
    va_list ap;
    va_start(ap, member);
    while(member >= 0) {
        if (member >= set->nbits) {
            va_end(ap);
            return false;
        }
        dobit(set, member, '&');

        va_arg(ap, int);
    }
    va_end(ap);
    return true;
}

/* return the number of elements in a set */
int set_elements(set_t *set)
{
    int i;
    int elements = 0;
    for (i = 0; i < set->nwords; ++i) {
        _SETTYPE word;
        for (word = set->map[i]; word != 0; word >>= 1) {
            if (word & 0x01) {
                elements ++;
            }
        }
    }

    return elements;
}

/* invert bits in a set. In effect, it remove all existing memebers of a set
 * and add all possible memebers that weren't there before.
 * Note that you should expand the set to the largest possible size before
 * calling this function, otherwise elements larger than the current max
 * element will not be added */
void set_invert(set_t *set)
{
    _SETTYPE *p = set->map;
    _SETTYPE *end = set->map + set->nwords;
    for (p = set->map; p < end; p++) {
        *p = ~*p;
    }
}

/* return the next member in *set*, this routine should be called several
 * successive times with the same arguments, just like iterators.
 * Note that you should not add/delete members while calling this function. 
 *
 * How to use:
 * 1. call set_next_member(NULL) to reset counter;
 * 2. call set_next_member(set) until -1 returns.
 *
 * return -1 if no more memeber */
int set_next_member(set_t *set)
{
    static int current_member = 0;
    static set_t *old_set = NULL;
    if (set == NULL) {
        old_set = 0;
        return -1;
    }

    if (set != old_set) {
        current_member = -1;
        old_set = set;

        _SETTYPE *run;
        for (run = set->map; *run == 0; run++) {
            current_member += _BITS_IN_WORD;
        }
    }

    do {
        current_member++;
        if (dobit(set, current_member, '=')) {
            return current_member;
        }
    } while(current_member < set->nbits);

    return -1;
}

/* return the next empty member in *set*, this routine should be called several
 * successive times with the same arguments, just like iterators.
 * Note that you should not add/delete members while calling this function. 
 *
 * How to use:
 * 1. call set_next_member(NULL) to reset counter;
 * 2. call set_next_member(set) until -1 returns.
 *
 * return -1 if no more memeber */
int set_next_empty(set_t *set)
{
    static int current_member = 0;
    static set_t *old_set = NULL;
    if (set == NULL) {
        old_set = 0;
        return -1;
    }

    if (set != old_set) {
        current_member = -1;
        old_set = set;

        _SETTYPE *run;
        for (run = set->map; *run == ~0; run++) {
            current_member += _BITS_IN_WORD;
        }
    }

    do {
        current_member++;
        if (!dobit(set, current_member, '=')) {
            return current_member;
        }
    } while(current_member < set->nbits);

    return -1;
}

/* union two sets: save the result into *dst* */
void set_union(set_t *dst, set_t *src)
{
    set_op(dst, src, '|');
}

/* intersection of two sets: save the result into *dst* */
void set_intersect(set_t *dst, set_t *src)
{
    set_op(dst, src, '&');
}

/* src-dst: save a member into *dst* if it is in *dst* and not in *src* */
void set_difference(set_t *dst, set_t *src)
{
    set_op(dst, src, '-');
}

/* overwrite dst with src */
void set_assign(set_t *dst, set_t *src)
{
    set_op(dst, src, '=');
}

/* clear every bit in the set */
void set_clear(set_t *set)
{
    memset(set->map, 0, set->nwords * sizeof(_SETTYPE));
}

/* set every bit in the set */
void set_fill(set_t *set)
{
    memset(set->map, ~0, set->nwords * sizeof(_SETTYPE));
}

/* return true if *set* is empty */
bool set_is_empty(set_t *set)
{
    return set_elements(set) == 0;
}

/* return true if *bit* is a member of *set* */
bool set_is_member(set_t *set, int bit)
{
    if (bit >= set->nbits) {
        return false;
    }
    return dobit(set, bit, '=');
}

/* check if *sub* is a subset of *set* */
bool set_is_subset(set_t *sub, set_t *set)
{
    assert(sub != NULL && set != NULL);

    size_t tail = sub->nwords - set->nwords; /* size(src) > size(dst) */
    size_t len = set->nwords > sub->nwords ? sub->nwords : set->nwords;

    _SETTYPE *ss = sub->map;
    _SETTYPE *sp = set->map;

    while(len-- > 0) {
        /* if (sub-parent) != 0, then sub is not a subset of parent */
        if (((*ss ^ *sp) & *ss) != 0) {
            return false;
        }
        ss++;
        sp++;
    }

    while(len > 0) {
        /* sub is longer, the longer part should be all zero */
        if (*ss++ != 0) {
            return false;
        }
    }

    return true;
}

/* return true if two set are disjoint(no elements in common) */
bool set_is_disjoint(set_t *sa, set_t *sb)
{
    return set_test(sa, sb) == SET_DISJOINT;
}

/* return true if two set intersect(at least one elements in common) */
bool set_is_intersect(set_t *sa, set_t *sb)
{
    return set_test(sa, sb) == SET_INTERSECT;
}

/* return true if two set are equal */
bool set_is_equal(set_t *sa, set_t *sb)
{
    return set_test(sa, sb) == SET_EQUAL;
}

/* print the set in human readable */
void set_print(set_t *set)
{
    if (set == NULL) {
        printf("(null)\n");
        return;
    }

    /* if the set is too full, print the missing ones, else print the existing
     * ones */
    int member_one_line = 0;
    const int MAX_MEMBER_LINE = 10;

    int elements = set_elements(set);
    int (*get_next)(set_t *);

    if (elements*2 < set->nbits) {
        get_next = set_next_member;
        printf("set[%d/%d]: ", elements, set->nbits);
    } else {
        get_next = set_next_empty;
        printf("set[%d/%d]: All except: ", elements, set->nbits);
    }

    get_next(NULL);
    int member;
    while(((member = get_next(set)) != -1)
          && member < set->nbits) {
        if (member_one_line % MAX_MEMBER_LINE == 0) {
            printf("\n->");
            member_one_line = 1;
        }
        printf("%d ", member);
        member_one_line ++;
    }
    printf("\n");
    get_next(NULL);
}
