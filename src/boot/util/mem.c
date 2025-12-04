#include "mem.h"
#include <stddef.h>

__attribute__((ms_abi)) void* memcpy(void* dest, const void* src, size_t n) {
    UINT8* d = dest;
    const UINT8* s = src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}


__attribute__((ms_abi)) int memcmp(const void *ptr1, const void *ptr2, size_t n) {
    const unsigned char *a = (const unsigned char *)ptr1;
    const unsigned char *b = (const unsigned char *)ptr2;

    for (size_t i = 0; i < n; i++) {
        if (a[i] != b[i]) {
            return (int)a[i] - (int)b[i];
        }
    }
    return 0;
}



__attribute__((ms_abi)) void memcat(void* out, const void *ptr1, const void *ptr2, size_t n1, size_t n2) {
    memcpy(out, ptr1, n1);
    memcpy((UINT8*)out + n1, ptr2, n2);
}


void memset(void* dest, UINT8 c, size_t n) {
    for (size_t i = 0; i < n; i++) {
        ((UINT8*)dest)[i] = c;
    }
}