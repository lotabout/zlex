#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


#include "hash.h"

#define MAX_KEY_LEN 20
#define MAX_VAL_LEN 50
#define TEST_CASES 1000

unsigned hash_str(const void *str)
{
    return hash_sdbm((const char *)str);
}

int str_cmp(const void *a, const void *b)
{
    return strcmp((const void *)a, (const void *)b);
}

void destory(void *key, void *val)
{
    return;
}

void pair_print(void *key, void *val)
{
    printf("%s = %s", (char *)key, (char *)val);
}

char key[TEST_CASES][MAX_KEY_LEN];
char val[TEST_CASES][MAX_VAL_LEN];

int main(int argc, char *argv[])
{
    srand(time(NULL));
    /* generate random keys and vals */
    int i;
    for (i = 0; i < TEST_CASES; i++) {
        int j;
        for (j=0; j<MAX_KEY_LEN; j++) {
            key[i][j] = rand() % 26 + 'a';
        }
        key[i][MAX_KEY_LEN-1] = '\0';
        for (j=0; j<MAX_VAL_LEN; j++) {
            val[i][j] = rand() % 26 + 'a';
        }
        val[i][MAX_VAL_LEN-1] = '\0';
    }

    hash_t *table = hash_new(TEST_CASES/3, hash_str, str_cmp);

    /* add them all and then delete them all */
    for (i = 0; i < TEST_CASES; i++) {
        hash_add(table, (void *)key[i], (void *)val[i]);
        if (hash_elements(table) != i+1) {
            printf("The size of table after adding %d-th element --- Error", i);
            exit(1);
        }
    }

    for (i=TEST_CASES-1; i>=0; i--) {
        hash_delete(table, (const void *)key[i]);
        if (hash_elements(table) != i) {
            printf("The size of table after deleting %d-th element --- Error", i);
            exit(1);
        }
    }

    /* now the table should be empty */
    void *p = NULL;

    /* add them all and fetch all elements */
    for (i = 0; i < TEST_CASES; i++) {
        hash_add(table, (void *)key[i], (void *)val[i]);
    }

    for (i=TEST_CASES-1; i>=0; i--) {
        p = hash_get(table, (void *)key[1]);
        if (p == NULL || p != (void *)val[1]) {
            printf("hash_get, return the wrong value --- Error");
            exit(1);
        }
    }

    p = hash_delete(table, (void *)key[1]);
    if (p == NULL || p != (void *)val[1]) {
        printf("hash_delete, return the wrong value --- Error");
        exit(1);
    }

    p = hash_delete(table, (void *)key[1]);
    if (p != NULL) {
        printf("hash_delete, return the wrong value --- Error");
        exit(1);
    }

    for (i=TEST_CASES-1; i>=0; i--) {
        hash_delete(table, (const void *)key[i]);
    }

    hash_add(table, (void *)key[1], (void *)val[1]);
    hash_add(table, (void *)key[28], (void *)val[28]);
    hash_add(table, (void *)key[512], (void *)val[512]);
    hash_add(table, (void *)key[999], (void *)val[999]);
    hash_add(table, (void *)key[71], (void *)val[71]);
    hash_print(table, pair_print);
    
    return 0;
}

