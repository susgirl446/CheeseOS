#include "../src/types.h"
#include "../src/fs/CheddarFS.h"


#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <linux/fs.h>


// Utility function for bitmap
static inline void FlipBit(UINT8* Bitmap, UINT64 Index) {
    Bitmap[Index / 8] ^= (1 << (7 - (Index % 8)));
}

static inline BOOLEAN GetBit(UINT8* Bitmap, UINT64 Index) {
    return (Bitmap[Index / 8] >> (7 - (Index % 8))) & 1;
}

// Returns starting block
UINT64 FindFreeBlocks(struct CheddarFS_SuperBlock* Superblock, UINT8* Bitmap, UINT64 count) {
    UINT64 consecutiveBlocks = 0;
    UINT64 start = 0;

    for (UINT64 i = 0; i < Superblock->BlockCount; i++) {
        if (GetBit(Bitmap, i) == 0) {
            if (consecutiveBlocks == 0) start = i;
            consecutiveBlocks++;
            if (consecutiveBlocks == count) return start;
        } else {
            consecutiveBlocks = 0;
        }
    }


    return UINT64_MAX; // no viable region found
}


int main(int argc, char** argv) {
    const char *Partition = argv[1];



    if (!(argc == 3 && strcmp(argv[2], "-y") == 0)) {
        printf("--------------------------------------------\n");
        printf("This will anything present on the disk! Proceed [y/N] ");
        char answer;
        scanf("%c", &answer);
        if (answer == 'n' || answer == '\n') {
            printf("Ok. Quitting\n");
            return 0;
        }
    }

    printf("--------------------------------------------\n");
    printf("Creating CheddarFS on %s\n", Partition);

    int fd = open(Partition, O_RDWR);


    // Superblock
    struct CheddarFS_SuperBlock* sb = calloc(1, sizeof(struct CheddarFS_SuperBlock));


    memcpy(sb->Magic,  "CHEDDAR", 8);

    sb->Version[0] = 1;
    sb->Version[1] = 0;
    sb->Version[2] = 0;

    sb->BlockSize = 4096;

    UINT64 partitionSize = 0;
    ioctl(fd, BLKGETSIZE64, &partitionSize);

    sb->BlockCount = partitionSize / sb->BlockSize;




    struct CheddarFS_BlockBitmap* bm = malloc(sizeof(struct CheddarFS_BlockBitmap));
    bm->Bitmap = calloc(1, ((sb->BlockCount + 7) / 8)); // Round up
    


    FlipBit(bm->Bitmap, 0); // Since superblock takes up the first block
    
    UINT64 BitmapBlocksUsed = ((sb->BlockCount + 7) / 8 + sb->BlockSize - 1) / sb->BlockSize;

    for (UINT64 i = 1; i <= BitmapBlocksUsed; i++) { // Since index 0 is superblock
        FlipBit(bm->Bitmap, i);
    }

    sb->BitmapStartBlock = 1; // always at block 1

    sb->InodeStartBlock = sb->BitmapStartBlock + 1;
    sb->InodeCount = partitionSize / 16384; // 1 Inode per 16KB
    sb->RootInode = 0;
    struct CheddarFS_Inode* inodes = calloc(1, sizeof(struct CheddarFS_Inode) * sb->InodeCount);



    for (UINT64 i = 0; i < sb->InodeCount; i++) {
        inodes[i].Flags |= 3 << 6; // Set all inodes to unused
    }


    UINT64 InodeBlocksUsed = (sizeof(struct CheddarFS_Inode) * sb->InodeCount + sb->BlockSize - 1) / sb->BlockSize;

    sb->NametableStartBlock  = sb->InodeStartBlock + InodeBlocksUsed;



    for (UINT64 j = sb->InodeStartBlock; j < (sb->InodeStartBlock + InodeBlocksUsed); j++) {
        FlipBit(bm->Bitmap, j);
    }




    UINT64 Remaining = sb->InodeCount;
    UINT64 Offset = (sb->InodeStartBlock + ((sizeof(struct CheddarFS_Inode) * sb->InodeCount + sb->BlockSize - 1) / sb->BlockSize)) * sb->BlockSize;
    while (Remaining > 0) {
        UINT64 ChunkSize = Remaining > 1000000 ? 1000000 : Remaining;

        struct CheddarFS_Nametable* chunk = calloc(1, sizeof(struct CheddarFS_Nametable) * ChunkSize);
        for (UINT64 i = 0; i < ChunkSize; i++) {
            memset(chunk[i].name, 0, 256);
        }

        pwrite(fd, chunk, sizeof(struct CheddarFS_Nametable) * ChunkSize, Offset);

        Offset += sizeof(struct CheddarFS_Nametable) * ChunkSize;
        Remaining -= ChunkSize;
    
        free(chunk);
    }

    UINT64 NametableUsedBlocks = (sizeof(struct CheddarFS_Nametable) * sb->InodeCount + sb->BlockSize - 1) / sb->BlockSize;

    for (UINT64 j = sb->NametableStartBlock; j < (sb->NametableStartBlock + NametableUsedBlocks); j++) {
        FlipBit(bm->Bitmap, j);
    }

    sb->DataStartBlock = sb->NametableStartBlock + NametableUsedBlocks;


    // TEMPORARY TEST

    // Clear and set to directory
    inodes[sb->RootInode].Flags &= ~(3 << 6);
    inodes[sb->RootInode].Flags |= 1 << 6;

    // find empty block
    for (UINT64 i = 0; i < sb->BlockCount; i++) {
        if (GetBit(bm->Bitmap, i) == 0) {
            inodes[sb->RootInode].Extents[0].Start = i;
            inodes[sb->RootInode].Extents[0].Length = 1;
            FlipBit(bm->Bitmap, i);
            break;
        }
    }


    inodes[1].Flags &= ~(3 << 6);
    inodes[1].Flags |= 1 << 6;

    inodes[2].Flags &= ~(3 << 6);
    inodes[2].Flags |= 0 << 6;

    for (UINT64 i = 0; i < sb->BlockCount; i++) {
        if (GetBit(bm->Bitmap, i) == 0) {
            inodes[1].Extents[0].Start = i;
            inodes[1].Extents[0].Length = 1;
            FlipBit(bm->Bitmap, i);
            break;
        }
    }

    for (UINT64 i = 0; i < sb->BlockCount; i++) {
        if (GetBit(bm->Bitmap, i) == 0) {
            inodes[2].Extents[0].Start = i;
            inodes[2].Extents[0].Length = 1;
            FlipBit(bm->Bitmap, i);
            break;
        }
    }

    long timestamp = (long)time(NULL);
    inodes[2].CreationDate = timestamp;
    inodes[2].ModificationDate = timestamp;

    printf("Creation: %lu\n", timestamp);

    inodes[2].Size = 4096;

    char* data = malloc(4096);

    for (int i = 0; i < 50; i++) {
        data[i] = 'a';
    }
    for (int i = 50; i < 100; i++) {
        data[i] = 'b';
    }
    for (int i = 100; i < 150; i++) {
        data[i] = 'c';
    }
    for (int i = 150; i < 200; i++) {
        data[i] = 'd';
    }
    for (int i = 200; i < 254; i++) {
        data[i] = 'e';
    }

    data[4095] = '\0';

    pwrite(fd, data, 4096, (sb->DataStartBlock + inodes[2].Extents[0].Start) * sb->BlockSize);

    char name[256] = "hello.txt\0";

    pwrite(fd, name, 256, (sb->NametableStartBlock * sb->BlockSize) + (2 * 256));


    UINT64* rootInodeData = calloc(1, 4096);

    rootInodeData[0] = 2;
    inodes[sb->RootInode].Size = sizeof(UINT64);

    pwrite(fd, rootInodeData, 4096, (sb->DataStartBlock + inodes[sb->RootInode].Extents[0].Start) * sb->BlockSize);

    // end





    pwrite(fd, sb, sizeof(struct CheddarFS_SuperBlock), 0);
    printf("Superblock written\n");

    pwrite(fd, bm->Bitmap, ((sb->BlockCount + 7) / 8), sb->BitmapStartBlock * sb->BlockSize);
    printf("Bitmap written. Blocks in bitmap: %llu. Blocks used by bitmap: %llu\n", sb->BlockCount, BitmapBlocksUsed);

    pwrite(fd, inodes, sizeof(struct CheddarFS_Inode) * sb->InodeCount, sb->InodeStartBlock * sb->BlockSize);
    printf("Inodes written. Inode count: %llu. Blocks used by inode's: %llu\n", sb->InodeCount, (sizeof(struct CheddarFS_Inode) * sb->InodeCount + sb->BlockSize - 1) / sb->BlockSize);

    printf("Nametable written. Blocks used by nametable: %llu\n", NametableUsedBlocks);



    free(sb);
    free(bm);
    free(inodes);

    close(fd);

    printf("--------------------------------------------\n");
    

    return 0;
}
