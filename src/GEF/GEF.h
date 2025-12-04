#pragma once

#include "../types.h"

struct GEF_Header {
    UINT8 Magic[4];
    UINT64 Entry;


    /*
     * On-disk layout:
     *  ---------------------
     *  | Header            |
     *  ---------------------
     *  | Code              |
     *  ---------------------
     *  | Read-only data    |
     *  ---------------------
     *  | Data              |
     *  ---------------------
    */

    UINT64 CodeSize;
    UINT64 RoSize;
    UINT64 DataSize;
    UINT64 BssSize;
} __attribute__((packed));
