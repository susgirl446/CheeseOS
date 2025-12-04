#pragma once

#include "../types.h"


// https://wiki.osdev.org/GPT
struct GPT {
    UINT8 Signature[8]; // Should be "EFI PART"
    UINT32 Revision;
    UINT32 HeaderSize;
    UINT32 Checksum;
    UINT32 Reserved;
    UINT64 LBA;
    UINT64 AlternateLBA;
    UINT64 FirstBlock;
    UINT64 LastBlock;
    UINT8 GUID[16];
    UINT64 StartPartitionEntryLBA;
    UINT32 PartitionEntries;
    UINT32 PartitionEntrySize;
    UINT32 PartitionEntryArrayChecksum;
    UINT8 reserved2[420];
} __attribute__((packed));

struct GPTEntry {
    UINT8 Type[16];
    UINT8 GUID[16];
    UINT64 StartLBA;
    UINT64 EndLBA;
    UINT64 Attributes;
    UINT8 Name[72];
} __attribute__((packed));

