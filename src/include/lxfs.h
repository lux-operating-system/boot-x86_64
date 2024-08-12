/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

/* File System Structures */

typedef struct {
    uint8_t flags;
    uint8_t chsStart[3];
    uint8_t id;
    uint8_t chsEnd[3];
    uint32_t start;
    uint32_t size;
} __attribute__((packed)) MBRPartition;

#define MBR_PARTITION_OFFSET        446
#define MBR_FLAG_BOOTABLE           0x80
#define MBR_ID_LXFS                 0xF3

typedef struct {
    uint8_t bootCode1[4];
    uint32_t identifier;
    uint64_t volumeSize;
    uint64_t rootBlock;
    uint8_t parameters;
    uint8_t version;
    uint8_t name[16];
    uint8_t reserved[6];

    // more boot code follows
} __attribute__((packed)) LXFSIdentification;

#define LXFS_MAGIC                  0x5346584C  // 'LXFS', little endian
#define LXFS_VERSION                0x01

#define LXFS_ID_BOOTABLE            0x01
#define LXFS_ID_SECTOR_SIZE_SHIFT   1
#define LXFS_ID_SECTOR_SIZE_MASK    0x03
#define LXFS_ID_BLOCK_SIZE_SHIFT    3
#define LXFS_ID_BLOCK_SIZE_MASK     0x0F

#define LXFS_BLOCK_FREE             0x0000000000000000
#define LXFS_BLOCK_ID               0xFFFFFFFFFFFFFFFC
#define LXFS_BLOCK_BOOT             0xFFFFFFFFFFFFFFFD
#define LXFS_BLOCK_TABLE            0xFFFFFFFFFFFFFFFE
#define LXFS_BLOCK_EOF              0xFFFFFFFFFFFFFFFF

typedef struct {
    uint64_t createTime;
    uint64_t modTime;
    uint64_t accessTime;
    uint64_t sizeEntries;
    uint64_t sizeBytes;
    uint64_t reserved;
} __attribute__((packed)) LXFSDirectoryHeader;

typedef struct {
    uint16_t flags;

    uint16_t owner;
    uint16_t group;
    uint16_t permissions;
    uint64_t size;

    uint64_t createTime;
    uint64_t modTime;
    uint64_t accessTime;

    uint64_t block;
    uint16_t entrySize;
    uint8_t reserved[14];
    uint8_t name[512];
} __attribute__((packed)) LXFSDirectoryEntry;

#define LXFS_DIR_VALID              0x0001
#define LXFS_DIR_TYPE_SHIFT         1
#define LXFS_DIR_TYPE_MASK          0x03
#define LXFS_DIR_DELETED            0x1000

#define LXFS_DIR_TYPE_FILE          0x00
#define LXFS_DIR_TYPE_DIR           0x01
#define LXFS_DIR_TYPE_SOFT_LINK     0x02
#define LXFS_DIR_TYPE_HARD_LINK     0x03

/* implementation-specific constants */
#define LXFS_BLOCK_BUFFER               0x70000
#define LXFS_DIRECTORY_BUFFER           0x78000

size_t readBlock(uint8_t, int, uint64_t, size_t, void *);
