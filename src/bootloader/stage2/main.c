#include "stdio.h"
#include "stdint.h"


void _cdecl cstart_(uint16_t bootDrive) {
    puts("Hello world from Second Stage Bootloader written in C");
    for (;;); // hlt 
}

