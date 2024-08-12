/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

/* Read-only Minimalist LXFS Implementation */

#include <lxboot.h>
#include <lxfs.h>

// helper functions
unsigned int getBlockSize(uint8_t disk, int partition) {
    // TODO: cache this to improve performance and reduce i/o operations
    uint32_t partitionStart = getPartitionStart(disk, partition);
    readSectors((void *)LXFS_BLOCK_BUFFER, partitionStart, 1, disk);
    LXFSIdentification *id = (LXFSIdentification *)LXFS_BLOCK_BUFFER;

    return ((id->parameters >> 3) & 0xF) + 1;
}

unsigned int getSectorSize(uint8_t disk, int partition) {
    uint32_t partitionStart = getPartitionStart(disk, partition);
    readSectors((void *)LXFS_BLOCK_BUFFER, partitionStart, 1, disk);
    LXFSIdentification *id = (LXFSIdentification *)LXFS_BLOCK_BUFFER;

    uint8_t shift = (id->parameters >> 1) & 3;
    return (512 << shift);
}

uint64_t getRootDirectory(uint8_t disk, int partition) {
    uint32_t partitionStart = getPartitionStart(disk, partition);
    readSectors((void *)LXFS_BLOCK_BUFFER, partitionStart, 1, disk);
    LXFSIdentification *id = (LXFSIdentification *)LXFS_BLOCK_BUFFER;

    return id->rootBlock;
}

size_t readBlock(uint8_t disk, int partition, uint64_t start, size_t count, void *buffer) {
    uint32_t partitionStart = getPartitionStart(disk, partition);
    uint32_t blockSize = getBlockSize(disk, partition);
    readSectors(buffer, (start*blockSize)+partitionStart, count*blockSize, disk);
    return count;
}

uint64_t getNextBlock(uint8_t disk, int partition, uint32_t block) {
    int blockSizeBytes = getBlockSize(disk, partition) * getSectorSize(disk, partition);
    uint32_t tableBlock = block / (blockSizeBytes / 8);
    tableBlock += 33;       // skip to the actual table blocks
    uint32_t tableIndex = block % (blockSizeBytes / 8);

    readBlock(disk, partition, tableBlock, 1, (void *)LXFS_DIRECTORY_BUFFER);

    uint64_t *data = (uint64_t *)LXFS_DIRECTORY_BUFFER;
    return data[tableIndex];
}

uint64_t readNextBlock(uint8_t disk, int partition, uint64_t block, void *buffer) {
    readBlock(disk, partition, block, 1, buffer);
    return getNextBlock(disk, partition, block);
}