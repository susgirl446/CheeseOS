#pragma once

#include "../types.h"


struct CheddarFS_SuperBlock {
    UINT8 Magic[8];             // 'CHEDDAR\0' 
    UINT8 Version[3];           
    UINT16 BlockSize;
    UINT64 BlockCount;
    UINT64 InodeCount;
    UINT64 RootInode;
    
    UINT64 BitmapStartBlock;
    UINT64 InodeStartBlock;
    UINT64 NametableStartBlock;
    UINT64 DataStartBlock;

    UINT8 Reserved[443];
} __attribute__((packed));

struct CheddarFS_BlockBitmap {
    UINT8* Bitmap;
} __attribute__((packed));


struct CheddarFS_Extent {
    UINT64 Start;
    UINT32 Length;
} __attribute__((packed));

struct CheddarFS_Inode {
    /*
     * Bit 6-7 Type (File: 00, Dir: 01, Symlink: 10, Unused Inode: 11)
     * Bit 5 Execute (Not executable: 0, Executable: 1)
     * Bit 4 Read-only (Writeable: 0, Read-only: 1)
     * Bit 0-3 Unused
    */
    UINT8 Flags;
    UINT64 Size;
    UINT64 CreationDate;
    UINT64 ModificationDate;
    struct CheddarFS_Extent Extents[6];
    UINT8 Reserved[3];
} __attribute__((packed));




struct CheddarFS_Nametable {
    UINT8 name[256];
} __attribute__((packed));