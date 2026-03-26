#include <stdio.h>

unsigned int strlen(const char *s) {
    unsigned int len = 0;
    while (*s++) {
        len++;
    }
    return len;
}

void memcpy(void *dest, const void *src, unsigned int n) {
    char *d = dest;
    const char *s = src;
    for (unsigned int i = 0; i < n; i++) {
        d[i] = s[i];
    }
}