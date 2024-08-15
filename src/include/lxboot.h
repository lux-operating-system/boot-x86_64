/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#pragma once

#include <stdint.h>
#include <stddef.h>
#include <lxfs.h>

#define PAGING_BASE         0x100000    // 1 MB mark
#define PAGE_SIZE           4096

/* Boot Protocol */

/* this structure is passed from the earlier stage boot loader */
/* see main.asm for this */
typedef struct {
    uint8_t bootDevice;
    MBRPartition partition;
    uint32_t videoAPI;      /* callable routines for BIOS API functions */
    uint32_t diskAPI;
    uint32_t keyboardAPI;
    uint32_t miscAPI;
    uint32_t regs;
} __attribute__((packed)) LXBootInfo;

/* this structure is used to pass info to and from the BIOS */
typedef struct {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;
    uint32_t esi;
    uint32_t edi;
    uint32_t eflags;
} __attribute__((packed)) CPURegisters;

/* for BIOS INT 13h */
typedef struct {
    uint8_t size;
    uint8_t reserved;
    uint16_t count;
    uint16_t offset;
    uint16_t segment;
    uint64_t lba;
} __attribute__((packed)) DiskAddressPacket;

/* E820h Memory Map */
typedef struct {
    uint64_t base;
    uint64_t len;
    uint32_t type;
    uint32_t acpiAttributes;
} __attribute__((packed)) MemoryMap;

#define MEMORY_TYPE_USABLE              1
#define MEMORY_TYPE_RESERVED            2
#define MEMORY_TYPE_ACPI_RECLAIMABLE    3
#define MEMORY_TYPE_ACPI_NVS            4
#define MEMORY_TYPE_BAD                 5

#define MEMORY_ATTRIBUTES_VALID         0x01
#define MEMORY_ATTRIBUTES_NV            0x02

/* this structure is passed to the kernel */
typedef struct {
    uint32_t magic;         // 0x5346584C
    uint32_t version;       // 1

    uint8_t flags;

    /* BIOS-specific info */
    uint8_t biosBootDisk;
    uint8_t bootBootPartitionIndex;
    MBRPartition biosBootPartition;

    /* TODO: UEFI info */
    uint8_t uefiReserved[32];

    /* generic info */
    uint64_t memoryMap;
    uint8_t memoryMapSize;

    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint64_t framebuffer;
    uint8_t redPosition;
    uint8_t redMask;
    uint8_t greenPosition;
    uint8_t greenMask;
    uint8_t bluePosition;
    uint8_t blueMask;

    uint64_t ramdisk;           // pointer
    uint64_t ramdiskSize;

    uint8_t moduleCount;
    uint64_t modules;           // pointer to pointers
    uint64_t moduleSizes;       // pointer to array of uint64_t's

    char arguments[256];        // command-line arguments passed to the kernel
} __attribute__((packed)) KernelBootInfo;

#define BOOT_FLAGS_UEFI     0x01
#define BOOT_FLAGS_GPT      0x02

extern LXBootInfo bootInfo;
extern CPURegisters *biosRegs;

void videoAPI(CPURegisters *);
void diskAPI(CPURegisters *);

/* disk i/o */
int readSectors(void *, uint32_t, int, uint8_t);
int findBootPartition();
uint32_t getPartitionStart(uint8_t, int);

/* memory detection */
int detectMemory();

/* long mode setup */
void pagingSetup();
void lmode(uint32_t);
