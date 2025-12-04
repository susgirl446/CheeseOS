#include "string.h"


__attribute__((ms_abi)) void Itoa(INT64 value, CHAR16* Buffer) {
    CHAR16 tmp[32];
    int index = 0;
    BOOLEAN negative = FALSE;

    if (value == 0) {
        Buffer[0] = L'0';
        Buffer[1] = L'\0';
        return;
    }

    if (value < 0) {
        negative = TRUE;
        value = -value;
    }

    while (value != 0) {
        tmp[index++] = L'0' + (value % 10);
        value /= 10;
    }

    int pos = 0;
    if (negative) {
        Buffer[pos++] = L'-';
    }

    for (int i = index - 1; i >= 0; i--) {
        Buffer[pos++] = tmp[i];
    }

    Buffer[pos] = L'\0';
}

__attribute__((ms_abi)) size_t Strlen(const CHAR16* String) {
    size_t len = 0;

    while (String[len] != '\0') {
        len++;
    }

    return len;
}

__attribute__((ms_abi)) void Print(const CHAR16* String) {
    
}

__attribute__((ms_abi)) void U8toChar16(const UINT8* in, CHAR16* out) {
    UINTN i = 0;

    while (in[i] != '\0') {
        out[i] = (CHAR16)in[i];
        i++;
    }

     out[i] = L'\0';
}