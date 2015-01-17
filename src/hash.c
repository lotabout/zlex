#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <limits.h>
#include <stdlib.h>
#include "hash.h"

/*---------------------------------------------------------------------------*/
/* hash table implementations */

struct hash_table_{
     struct link{
          struct link *next;
          void *key;
          void *val;
     } **buckets;  /* pointer to the actual array */
     int size;     /* size of the hash table */
     int elements; /* number of elements */

     hash_func hash; /* function to compute hash from key */
     cmp_func cmp;   /* function to compare different keys */
};

/* create a new hash table, with max size *maxsym*,
 * *hash* is used to compute hash value for an object.
 * *cmp* is to compare two items, return 0 if equal, <0 if a < b, etc. */
hash_t *hash_new(size_t size, hash_func hash, cmp_func cmp)
{
    hash_t *table = NULL;
    static int primes[] = {509, 509, 1021, 2053, 4093, 8191, 16381, 32771,
                           65521, INT_MAX};
    int i;
    /* find the proper prime size that is larger than size */
    for (i = 1; primes[i] < size; i++) {
        /* pass */
    }
    table = (hash_t *)malloc(sizeof(*table) + primes[i-1]*sizeof(table->buckets[0]));
    if (table == NULL) {
        fprintf(stderr, "hash_new: not enough memory allocating hash table.\n");
        exit(1);
    }
    table->hash = hash;
    table->cmp = cmp;
    table->size = primes[i-1];
    table->elements = 0;
    table->buckets = (struct link **)(table+1);

    memset(table->buckets, 0, table->size*sizeof(table->buckets[0]));

    return table;
}

/* free a table
 * *destory* is used to destory key/value pairs */
void table_free(hash_t *table, void (*destory)(void *key, void *value))
{
    if (table == NULL) {
        return;
    }

    /* free all the elements */
    if (table->elements > 0) {
        int i;
        struct link *p, *next;
        for (i = 0; i < table->size; i++) {
            for (p=table->buckets[i]; p; p=next) {
                next = p->next;
                if (destory != NULL) {
                    destory(p->key, p->val);
                }
                free(p); /* free the node */
            }
        }
    }
    free(table);
}

/* add a (key, val) into the hash table */
void hash_add(hash_t *table, void *key, void *val)
{
    unsigned hash_val = table->hash(key) % table->size;

    /* generate a new node to store the pair (key, val) */
    struct link *p = (struct link *)malloc(sizeof(*p));
    if (p == NULL) {
        fprintf(stderr, "hash_add: not enough memory allocating new node.\n");
        exit(1);
    }

    p->key = key;
    p->val = val;
    p->next = table->buckets[hash_val];
    table->buckets[hash_val] = p;
    table->elements++;
}    

/* find the value for *key* in *table */
void *hash_get(hash_t *table, void *key)
{
    unsigned hash_val = table->hash(key) % table->size;

    struct link *p = table->buckets[hash_val];
    for (; p; p = p->next) {
        if (table->cmp(key, p->key) == 0) {
            return p->val;
        }
    }
    return NULL;
}

/* delete (key, val) from hash table, val is returned. */
void *hash_delete(hash_t *table, const void *key)
{
    unsigned hash_val = table->hash(key) % table->size;

    struct link *p = table->buckets[hash_val];
    struct link **prev = &table->buckets[hash_val];
    void *rval = NULL;
    for (; p; p = p->next) {
        if (table->cmp(key, p->key) == 0) {
            rval = p->val;
            break;
        }
        prev = &p->next;
    }

    if (p) {
        *prev = p->next;
        free(p);
        table->elements--;
    }

    return rval;
}

/* get the number of elements in the hash table, normally for debug usage */
int hash_elements(hash_t *table)
{
    return table->elements;
}

/* print a hash table */
void hash_print(hash_t *table, void (*print)(const void *key, const void *val))
{
    int i;
    struct link *p = NULL;
    for (i = 0; i < table->size; i++) {
        p = table->buckets[i];
        if (p == NULL) {
            continue;
        }

        printf("table[%d] contains: \n", i);
        for (; p; p = p->next) {
            printf("--> ");
            print(p->key, p->val);
            printf("\n");
        }
    }
}

/*---------------------------------------------------------------------------*/
/* Hash functions for strings
 * check: http://www.cse.yorku.ca/~oz/hash.html */
unsigned hash_djb2(const char *p)
{
    unsigned hash_val = 5381;
    while (*p) {
        hash_val = ((hash_val << 5) + hash_val) + *p; /* hash_val * 33 + c */
        hash_val %= UINT_MAX;
        p++;
    }
    return hash_val;
}

unsigned hash_sdbm(const char *p)
{
    unsigned hash_val = 0;
    while (*p) {
        hash_val = *p + (hash_val << 6) + (hash_val << 16) - hash_val;
        hash_val %= UINT_MAX;
        p++;
    }
    return hash_val;
}
