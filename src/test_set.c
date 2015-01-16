#include <stdio.h>
#include "set.h"

/* utility to test library set:
 * Most of the test cases are borrowed from book _Compiler Design in C_ */


int main(int argc, char *argv[])
{
    int i;
    
    set_t *s1 = set_new();
    set_t *s2 = set_new();
    set_t *s3 = set_new();
    set_t *s4 = set_new();

    /* add 1024, 2047 to set s2 */
    printf("--- Adding 1024, 2047, 222 into s2\n");
    set_add(s2, 1024, 2047, 222);
    set_print(s2);
    printf("--- removing 222 from s2\n");
    set_remove(s2, 222);
    set_print(s2);
    printf("--- invert s2\n");
    set_invert(s2);
    set_print(s2);
    printf("--- invert again\n");
    set_invert(s2);
    set_print(s2);
    printf("--- removing 1024, 2047 from s2\n");
    set_remove(s2, 1024, 2047);
    set_print(s2);

/*---------------------------------------------------------------------------*/
    for (i = 0; i <= 1024; ++i) {
        set_add(s1, i);
        if (!set_is_member(s1, i)) {
            fprintf(stderr, "initial:<%d> not in set and it should be.\n", i);
        }
    }
/*---------------------------------------------------------------------------*/
    for (i = 0; i <= 1024; ++i) {
        if (!set_is_member(s1, i)) {
            fprintf(stderr, "verify:<%d> not in set and it should be.\n", i);
        }
    }

    for (i = 0; i <= 1024; ++i) {
        set_remove(s1, i);
        if (set_is_member(s1, i)) {
            fprintf(stderr, "initial:<%d> is in set and it should not be.\n", i);
        }
    }

    for (i = 0; i <= 1024; ++i) {
        if (set_is_member(s1, i)) {
            fprintf(stderr, "verify:<%d> is in set and it should not be.\n", i);
        }
    }

/*---------------------------------------------------------------------------*/
    printf("Testing Equality between s1 and s2 before truncate\n");
    printf(">>> s1 == s2 --- %s\n", set_is_equal(s1, s2) ? "OK" : "Error");

    set_truncate(s1);
    printf("Testing Equality between s1 and s2 AFTER truncate\n");
    printf(">>> s1 == s2 --- %s\n", set_is_equal(s1, s2) ? "OK" : "Error");
/*---------------------------------------------------------------------------*/
    /* now all sets should be (null) */

    set_add(s1, 1);
    set_add(s2, 1);
    set_add(s3, 1);
    set_add(s1, 517);
    set_add(s2, 517);


    printf(">>> s1 == s2 --- %s\n", set_is_equal(s1, s2) ? "OK" : "Error");

    set_remove(s2, 517);

    printf(">>> s1 != s2 --- %s\n", !set_is_equal(s1, s2) ? "OK" : "Error");
    printf(">>> s2 != s1 --- %s\n", !set_is_equal(s2, s1) ? "OK" : "Error");
    printf(">>> s1 != s3 --- %s\n", !set_is_equal(s1, s3) ? "OK" : "Error");
    printf(">>> s3 == s2 --- %s\n", set_is_equal(s3, s2) ? "OK" : "Error");
    printf(">>> s2 == s3 --- %s\n", set_is_equal(s2, s3) ? "OK" : "Error");

/*---------------------------------------------------------------------------*/
    set_add(s1, 3);
    set_add(s1, 6);
    set_add(s1, 9);
    set_add(s1, 12);
    set_add(s1, 15);
    set_add(s1, 16);
    set_add(s1, 19);
    set_add(s3, 18);

    printf("s1 = ");
    set_print(s1);
    printf("s2 = ");
    set_print(s2);
    printf("s3 = ");
    set_print(s3);
    printf("s4 = ");
    set_print(s4);

    s2 = set_dup(s1);
    printf(">>> dupset --- %s\n", set_is_equal(s1, s2) ? "OK" : "Error");

    printf(">>> s1 != empty --- %s\n", !set_is_empty(s1) ? "OK" : "Error");
    printf(">>> s3 != empty --- %s\n", !set_is_empty(s3) ? "OK" : "Error");
    printf(">>> s4 == empty --- %s\n", set_is_empty(s4) ? "OK" : "Error");

    printf(">>> s1 & s3 NOT disjoint --- %s\n",
           !set_is_disjoint(s1, s3) ? "OK" : "ERROR");
    printf(">>> s1 & s4 NOT disjoint --- %s\n",
           !set_is_disjoint(s1, s4) ? "OK" : "ERROR");

    printf(">>> s1 & s3 NOT intersect --- %s\n",
           set_is_intersect(s1, s3) ? "OK" : "ERROR");
    printf(">>> s1 & s4 NOT intersect --- %s\n",
           set_is_intersect(s1, s4) ? "OK" : "ERROR");

    printf(">>> s1 IS a subset of s1 --- %s\n",
           set_is_subset(s1, s1) ? "OK" : "ERROR");
    printf(">>> s3 IS a subset of s3 --- %s\n",
           set_is_subset(s3, s3) ? "OK" : "ERROR");
    printf(">>> s4 IS a subset of s4 --- %s\n",
           set_is_subset(s4, s4) ? "OK" : "ERROR");
    printf(">>> s1 IS NOT a subset of s3 --- %s\n",
           !set_is_subset(s1, s3) ? "OK" : "ERROR");
    printf(">>> s1 IS NOT a subset of s4 --- %s\n",
           !set_is_subset(s1, s4) ? "OK" : "ERROR");
    printf(">>> s3 IS NOT a subset of s1 --- %s\n",
           !set_is_subset(s3, s1) ? "OK" : "ERROR");
    printf(">>> s3 IS NOT a subset of s4 --- %s\n",
           !set_is_subset(s3, s4) ? "OK" : "ERROR");
    printf(">>> s4 IS a subset of s1 --- %s\n",
           set_is_subset(s4, s1) ? "OK" : "ERROR");
    printf(">>> s4 IS a subset of s3 --- %s\n",
           set_is_subset(s4, s3) ? "OK" : "ERROR");


    printf("--- Adding 18 to s1:\n");
    set_add(s1, 18);
    printf(">>> s1 IS a subset of s1 --- %s\n",
           set_is_subset(s1, s1) ? "OK" : "ERROR");
    printf(">>> s3 IS a subset of s3 --- %s\n",
           set_is_subset(s3, s3) ? "OK" : "ERROR");
    printf(">>> s1 IS NOT a subset of s3 --- %s\n",
           !set_is_subset(s1, s3) ? "OK" : "ERROR");
    printf(">>> s3 IS a subset of s1 --- %s\n",
           set_is_subset(s3, s1) ? "OK" : "ERROR");

    printf("--- removing 18 to s1:\n");
    set_remove(s1, 18);

    printf("--- s1 = ");
    set_print(s1);
    printf("--- s3 = ");
    set_print(s3);

    set_assign(s2, s3);
    set_union(s2, s1);
    printf("--- s1 | s3 = ");
    set_print(s2);

    set_assign(s2, s3);
    set_intersect(s2, s1);
    printf("--- s1 & s3 = ");
    set_print(s2);

    set_assign(s2, s3);
    set_difference(s2, s1);
    printf("--- s1 - s3 = ");
    set_print(s2);

    set_truncate( s2 );
    printf(">>> s2 == empty --- %s\n", set_is_empty(s2) ? "OK" : "ERROR");

    set_invert(s2);
    printf("s2 inverted = ");
    set_print(s2);

    set_clear(s2);
    printf("s2 cleared = ");
    set_print(s2);

    set_fill(s2);
    printf("s2 filled = ");
    set_print(s2);

    printf("s1=");set_print(s1);
    printf("s3=");set_print(s3);
    printf("s4=");set_print(s4);

    return 0;
}
