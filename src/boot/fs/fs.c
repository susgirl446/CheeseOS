#include "fs.h"

#include "../globals.h"
#include "../../fs/CheddarFS.h"
#include "../util/mem.h"
#include "../util/string.h"

__attribute__((ms_abi)) struct CheddarFS_SuperBlock* CheddarFS_LoadSuperBlock(struct GPTEntry* Partition) {
    struct CheddarFS_SuperBlock* Superblock;

    gBS->AllocatePool(EfiLoaderData, sizeof(struct CheddarFS_SuperBlock), (VOID**)&Superblock);

    DiskIo->ReadDisk(
        DiskIo,
        BlockIo->Media->MediaId,
        Partition->StartLBA * BlockIo->Media->BlockSize,
        sizeof(struct CheddarFS_SuperBlock),
        Superblock
    );

    if (memcmp(Superblock->Magic, "CHEDDAR", 8) != 0) {
        gST->ConOut->OutputString(gST->ConOut, L"Superblock's magic did not match expected magic\r\n");
        return NULL;
    }

    return Superblock;
}


__attribute__((ms_abi)) struct CheddarFS_Inode* CheddarFS_LoadInode(struct GPTEntry* Partition, struct CheddarFS_SuperBlock* SuperBlock, UINT64 Index) {
    struct CheddarFS_Inode* Inode;

    gBS->AllocatePool(EfiLoaderData, sizeof(struct CheddarFS_Inode), (VOID**)&Inode);

    DiskIo->ReadDisk(
        DiskIo, 
        BlockIo->Media->MediaId,
        (Partition->StartLBA * BlockIo->Media->BlockSize) + (SuperBlock->InodeStartBlock * SuperBlock->BlockSize) + (sizeof(struct CheddarFS_Inode) * Index),
        sizeof(struct CheddarFS_Inode),
        Inode
    );

    return Inode;
}


__attribute__((ms_abi)) UINT8* CheddarFS_LoadData(
    struct GPTEntry* Partition,
    struct CheddarFS_SuperBlock* Superblock,
    struct CheddarFS_Inode* Inode
) {
    UINT8* Data; 

    gBS->AllocatePool(EfiLoaderData, ((Inode->Size + Superblock->BlockSize / 2) / Superblock->BlockSize) * Superblock->BlockSize, (VOID**)&Data);

    UINT64 Offset = 0;

    for (UINT8 i = 0; i < 5; i++) {
        if (Inode->Extents[i].Length != 0) {
            DiskIo->ReadDisk(
                DiskIo,
                BlockIo->Media->MediaId,
                (Partition->StartLBA * BlockIo->Media->BlockSize) + (Superblock->DataStartBlock * Superblock->BlockSize) + (Inode->Extents[i].Start * Superblock->BlockSize),
                Inode->Extents[i].Length * Superblock->BlockSize,
                Data+Offset
            );

            Offset += Inode->Extents[i].Length * Superblock->BlockSize;
        }
    }

    return Data;
}



__attribute__((ms_abi)) CHAR16* CheddarFS_QueryFilename(
    struct GPTEntry* Partition,
    struct CheddarFS_SuperBlock* SuperBlock,
    UINT64 Index
) {
    CHAR16* name;
    UINT8* tmp;

    gBS->AllocatePool(EfiLoaderData, sizeof(struct CheddarFS_Nametable), (VOID**)&tmp);
    gBS->AllocatePool(EfiLoaderData, (sizeof(struct CheddarFS_Nametable) + 1) * 2, (VOID**)&name);

    memset(name, '\0', sizeof(struct CheddarFS_Nametable));

    DiskIo->ReadDisk(
        DiskIo,
        BlockIo->Media->MediaId,
        (Partition->StartLBA * BlockIo->Media->BlockSize) + (SuperBlock->NametableStartBlock * SuperBlock->BlockSize) + (sizeof(struct CheddarFS_Nametable) * Index),
        sizeof(struct CheddarFS_Nametable),
        tmp
    );

    U8toChar16(tmp, name);

    return name;
}