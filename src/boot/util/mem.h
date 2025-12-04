#pragma once

#include "../../types.h"



__attribute__((ms_abi)) void* memcpy(void* dest, const void* src, size_t n);
__attribute__((ms_abi)) int memcmp(const void *ptr1, const void *ptr2, size_t n);
__attribute__((ms_abi)) void memcat(void* out,const void *ptr1, const void *ptr2, size_t n1, size_t n2);
void memset(void* dest, UINT8 c, size_t n);