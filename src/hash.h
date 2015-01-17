#ifndef HASH_H
#define HASH_H

/*-----------------------------------------------------------------------------
 * Library: hash table
 * Implement a general routine(more or less) of hash table.
 *
 * Most of the API are borrowed from book _Compiler Design in C_
 *---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* type definition */
typedef struct hash_table_ hash_t;
typedef unsigned (*hash_func)(const void *);
typedef int (*cmp_func)(const void *a, const void *b);

/*---------------------------------------------------------------------------*/
/* APIs */

/* create a new hash table, with max size *maxsym*,
 * *hash* is used to compute hash value for an object.
 * *cmp* is to compare two items, return 0 if equal, <0 if a < b, etc. */
hash_t *hash_new(size_t size, hash_func hash, cmp_func cmp);

/* free a table
 * *destory* is used to destory key/value pairs */
void table_free(hash_t *table, void (*destory)(void *key, void *value));

/* add a (key, val) into the hash table */
void hash_add(hash_t *table, void *key, void *val);

/* find the value for *key* in *table */
void *hash_get(hash_t *table, void *key);

/* delete (key, val) from hash table, val is returned. */
void *hash_delete(hash_t *table, const void *key);

/* get the number of elements in the hash table, normally for debug usage */
int hash_elements(hash_t *table);

/* print a hash table */
void hash_print(hash_t *table, void (*print)(const void *key, const void *val));


/*---------------------------------------------------------------------------*/
/* Hash functions */
unsigned hash_djb2(const char *p);
unsigned hash_sdbm(const char *p);

#endif /* end of include guard: HASH_H */
