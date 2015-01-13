#include <stdio.h>

#include "escape.h"

int main(int argc, char *argv[])
{
    char *buffer = "\\b\\f\\n\\r\\s\\t\\e\\007\\x64\\^M";
    char *input = buffer;

    int expected[] = {'\b', '\f', '\n', '\r', ' ', '\t', 
    '\033', '\007', '\x64', '\x0d'};

    int len = sizeof(expected)/sizeof(expected[0]);

    int result[200];
    int *p = result;
    while(*input) {
        *p++ = escape(&input);
    }

    int i;
    for (i = 0; i < len; ++i) {
        if (result[i] != expected[i]) {
            printf("Case %d: Expected '%c', got '%c'\n", i, expected[i],
                   result[i]);
        } else {
            printf("Case %d: OK\n", i);
        }
    }
    return 0;
}
