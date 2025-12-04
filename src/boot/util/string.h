#pragma once

#include "../../types.h"


__attribute__((ms_abi)) void Itoa(INT64 value, CHAR16* Buffer);

__attribute__((ms_abi)) size_t Strlen(const CHAR16* String);

__attribute__((ms_abi)) void Print(const CHAR16* String);


__attribute__((ms_abi)) void U8toChar16(const UINT8* in, CHAR16* out);