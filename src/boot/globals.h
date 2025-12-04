#pragma once


#include "EFI/EFI.h"


extern EFI_SYSTEM_TABLE* gST;
extern EFI_BOOT_SERVICES* gBS;
extern EFI_RUNTIME_SERVICES* gRS;

extern EFI_DISK_IO_PROTOCOL* DiskIo;
extern EFI_BLOCK_IO_PROTOCOL* BlockIo;

extern EFI_GUID gEfiBlockIoProtocolGuid;
extern EFI_GUID gEfiDiskIoProtocolGuid;
