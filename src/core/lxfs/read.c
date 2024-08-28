/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

/* Read-only Minimalist LXFS Implementation */

#include <lxfs.h>
#include <stdio.h>

bool lxfsRead(uint8_t disk, int partition, const char *path, void *buffer) {
    printf("lxfs: reading %s from disk 0x%02X partition %d...\n", path, disk, partition);
    LXFSDirectoryEntry *entry = (LXFSDirectoryEntry *)LXFS_DIRECTORY_BUFFER;
    if(!lxfsFindPath(disk, partition, path, entry)) return false;

    // cannot read directories the way we read files
    if(((entry->flags >> LXFS_DIR_TYPE_SHIFT) & LXFS_DIR_TYPE_MASK) != LXFS_DIR_TYPE_FILE) return false;

    uint64_t block = getNextBlock(disk, partition, entry->block);
    size_t count = 0;
    int blockSizeBytes = getBlockSize(disk, partition) * getSectorSize(disk, partition);

    while(block != LXFS_BLOCK_EOF) {
        //printf("lxfs: reading block %d\n", block);
        block = readNextBlock(disk, partition, block, buffer + (blockSizeBytes * count));
        count++;
    }

    return count;   // aka true if we read anything at all
}
