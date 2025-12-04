#pragma once

#include "../EFI/EFI.h"
#include "../../fs/GPT.h"


__attribute__((ms_abi)) struct CheddarFS_SuperBlock* CheddarFS_LoadSuperBlock(
    struct GPTEntry* Partition);

__attribute__((ms_abi)) struct CheddarFS_Inode* CheddarFS_LoadInode(
    struct GPTEntry* Partition,
    struct CheddarFS_SuperBlock* SuperBlock,
    UINT64 Index);

__attribute__((ms_abi)) UINT8* CheddarFS_LoadData(
    struct GPTEntry* Partition,
    struct CheddarFS_SuperBlock* Superblock,
    struct CheddarFS_Inode* Inode
);


__attribute__((ms_abi)) CHAR16* CheddarFS_QueryFilename(
    struct GPTEntry* Partition,
    struct CheddarFS_SuperBlock* SuperBlock,
    UINT64 Index
);