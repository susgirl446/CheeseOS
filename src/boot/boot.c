#include "EFI/EFI.h"
#include "../fs/CheddarFS.h"
#include "globals.h"
#include "../types.h"
#include "util/mem.h"
#include "util/string.h"
#include "../fs/GPT.h"
#include "fs/fs.h"
#include "../GEF/GEF.h"





UINT64 U8toU64(UINT8 bytes[8]) {
    return ((UINT64)bytes[0])       |
           ((UINT64)bytes[1] << 8)  |
           ((UINT64)bytes[2] << 16) |
           ((UINT64)bytes[3] << 24) |
           ((UINT64)bytes[4] << 32) |
           ((UINT64)bytes[5] << 40) |
           ((UINT64)bytes[6] << 48) |
           ((UINT64)bytes[7] << 56);
}



EFI_STATUS EFIAPI efi_main(
    EFI_HANDLE ImageHandle,
    EFI_SYSTEM_TABLE* SystemTable
) {
    gST = SystemTable;
    gBS = SystemTable->BootServices; 
    gRS =  SystemTable->RuntimeServices;

    gST->ConOut->ClearScreen(gST->ConOut);


    gST->ConOut->OutputString(SystemTable->ConOut, L"Hello World!\r\n");


    EFI_HANDLE* handles;
    UINTN numHandles;
    gBS->LocateHandleBuffer(
        ByProtocol,
        &gEfiBlockIoProtocolGuid,
        NULL,
        &numHandles,
        &handles
    );

    EFI_HANDLE DiskHandle = NULL;

    for (UINTN i = 0; i < numHandles; i++) {
        gBS->OpenProtocol(
            handles[i],
            &gEfiBlockIoProtocolGuid,
            (VOID**)&BlockIo,
            ImageHandle,
            NULL,
            EFI_OPEN_PROTOCOL_GET_PROTOCOL
        );

        if (BlockIo->Media && BlockIo->Media->LogicalPartition == FALSE) {
            DiskHandle = handles[i];
            break; // found the full disk
        }
    }

    EFI_STATUS Status = gBS->OpenProtocol(
        DiskHandle,
        &gEfiDiskIoProtocolGuid,
        (VOID**)&DiskIo,
        ImageHandle,
        NULL,
        EFI_OPEN_PROTOCOL_GET_PROTOCOL
    );

    if (EFI_ERROR(Status)) {
        gST->ConOut->OutputString(SystemTable->ConOut, L"OpenProtocol failed\n");
        return 1;
    }

    struct GPT gpt;

    DiskIo->ReadDisk(
        DiskIo,
        BlockIo->Media->MediaId,
        BlockIo->Media->BlockSize * 1,  // LBA 1
        sizeof(struct GPT),
        &gpt
    );

    struct GPTEntry* entries;

    gBS->AllocatePool(EfiLoaderData, gpt.PartitionEntrySize * gpt.PartitionEntries, (VOID**)&entries);

    DiskIo->ReadDisk(
        DiskIo,
        BlockIo->Media->MediaId,
        BlockIo->Media->BlockSize * gpt.StartPartitionEntryLBA,
        gpt.PartitionEntrySize * gpt.PartitionEntries,
        (VOID*)entries
    );

   


    struct GPTEntry entry;
    for (UINTN i = 0; i < gpt.PartitionEntries; i++) {
        UINT8* Buffer;
        gBS->AllocatePool(EfiLoaderData, 4096, (VOID**)&Buffer);


        DiskIo->ReadDisk(DiskIo, BlockIo->Media->MediaId, entries[i].StartLBA * BlockIo->Media->BlockSize, 4096, Buffer);

        if(memcmp(Buffer,  "CHEDDAR", 8) == 0) {
            gST->ConOut->OutputString(gST->ConOut, L"Found CHEDDAR magic\r\n");
            entry = entries[i];
            gBS->FreePool(Buffer);
            break;
        }

        gBS->FreePool(Buffer);

    }

    
    struct CheddarFS_SuperBlock* SuperBlock = CheddarFS_LoadSuperBlock(&entry);
    struct CheddarFS_Inode* RootInode = CheddarFS_LoadInode(&entry, SuperBlock, SuperBlock->RootInode);


    UINT8* RootInodeData = CheddarFS_LoadData(&entry, SuperBlock, RootInode);

    struct CheddarFS_Inode* inode = CheddarFS_LoadInode(
        &entry,
        SuperBlock,
        U8toU64(RootInodeData)
    );


    UINT8* InodeData = CheddarFS_LoadData(&entry, SuperBlock, inode);

    CHAR16* tmp;
    gBS->AllocatePool(EfiLoaderData, 4096, (VOID**)&tmp);


    U8toChar16(InodeData, tmp);


    gST->ConOut->OutputString(gST->ConOut, L"\r\n");
    gST->ConOut->OutputString(gST->ConOut, tmp);
    gST->ConOut->OutputString(gST->ConOut, L"\r\n");
    
    EFI_INPUT_KEY key;
    while (gST->ConIn->ReadKeyStroke(SystemTable->ConIn, &key) != EFI_SUCCESS) {}

    gRS->ResetSystem(EfiResetShutdown, EFI_SUCCESS, 0, 0);

    return 0;
}