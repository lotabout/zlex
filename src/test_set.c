#include <stdio.h>
#include "set.h"


int main(int argc, char *argv[])
{
    int i;
    
    set_t *s1 = set_new();
    set_t *s2 = set_new();
    set_t *s3 = set_new();
    set_t *s4 = set_new();
    set_t *a[40];

    /* add 1024, 2047 to set s1 */
    set_add(s1, 1024, 2047);
    set_print(s1);
    set_invert(s1);
    set_print(s1);

    return 0;
}
