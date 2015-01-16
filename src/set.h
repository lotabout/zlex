#ifndef SET_H
#define SET_H

/*-----------------------------------------------------------------------------
 * Library: SET
 * implement set operations for sets containing non-negtive integers. The set
 * is implemented in bit-wise.
 *
 * Most of the API are borrowed from book _Compiler Design in C_
 *---------------------------------------------------------------------------*/
#include <stdbool.h>

typedef unsigned short _SETTYPE;    /* a single sell, normally should be 16
                                       bits*/
#define _DEFWORDS 8                 /* to make total 8*16 = 128 bit */
extern const int _BITS_IN_WORD;

/*---------------------------------------------------------------------------*/
typedef struct set
{
    size_t nwords;    /* words in the map */
    size_t nbits;     /* number of bits in the map */

    _SETTYPE *map;              /* pointer to the bit map */
    _SETTYPE defmap[_DEFWORDS]; /* default map */
} set_t;

/*---------------------------------------------------------------------------*/
/* API: functions manipulating set object, all function begin with "set_" */

/* generate a new set, abort if no enough memory. */
set_t *set_new(void);

/* destory a set, not the you should set the pointer to NULL after the set is
 * deleted */
void set_del(set_t *old_set);

/* duplicate a set */
set_t *set_dup(set_t *old_set);


#define set_add(...) set_add_members(__VA_ARGS__, -1)
/* add members to the set, negtive number counts for the end of input */
bool set_add_members(set_t *set, int member, ...);

#define set_remove(...) set_remove_members(__VA_ARGS__, -1)
/* remove members from set, negtive number counts for the end of input,
 * if a member is larger than the size of the set, false is returned. */
bool set_remove_members(set_t *set, int member, ...);

/* return the number of elements in a set */
int set_elements(set_t *set);

/* invert bits in a set. In effect, it remove all existing memebers of a set
 * and add all possible memebers that weren't there before.
 * Note that you should expand the set to the largest possible size before
 * calling this function, otherwise elements larger than the current max
 * element will not be added */
void set_invert(set_t *set);

/* return the next member in *set*, this routine should be called several
 * successive times with the same arguments, just like iterators.
 * Note that you should not add/delete members while calling this function. 
 *
 * return -1 if no more memeber */
int set_next_member(set_t *set);

/* return the next empty member in *set*, this routine should be called several
 * successive times with the same arguments, just like iterators.
 * Note that you should not add/delete members while calling this function. 
 *
 * How to use:
 * 1. call set_next_member(NULL) to reset counter;
 * 2. call set_next_member(set) until -1 returns.
 *
 * return -1 if no more memeber */
int set_next_empty(set_t *set);

/* union two sets: save the result into *dst* */
void set_union(set_t *dst, set_t *src);

/* intersection of two sets: save the result into *dst* */
void set_intersect(set_t *dst, set_t *src);

/* src-dst: save a member into *dst* if it is in *dst* and not in *src* */
void set_difference(set_t *dst, set_t *src);

/* overwrite dst with src */
void set_assign(set_t *dst, set_t *src);

/* clear every bit in the set */
void set_clear(set_t *set);

/* set every bit in the set */
void set_fill(set_t *set);

/* clear the set and set the set to its original size. 
 * compare with clear() which clear all the bits without modifying the size */
void set_truncate(set_t *set);

/* return true if *set* is empty */
bool set_is_empty(set_t *set);

/* return true if *bit* is a member of *set* */
bool set_is_member(set_t *set, int bit);

/* check if *sub* is a subset of *set* */
bool set_is_subset(set_t *sub, set_t *set);

/* return true if two set are disjoint(no elements in common) */
bool set_is_disjoint(set_t *sa, set_t *sb);

/* return true if two set intersect(at least one elements in common) */
bool set_is_intersect(set_t *sa, set_t *sb);

/* return true if two set are equal */
bool set_is_equal(set_t *sa, set_t *sb);

/* print the set in human readable */
void set_print(set_t *set);

#endif /* end of include guard: SET_H */

