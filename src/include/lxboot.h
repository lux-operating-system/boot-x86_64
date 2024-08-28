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
    uint32_t lmode;         /* pointer to void lmode(uint32_t paging, uint32_t entry) */
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
    uint8_t biosBootPartitionIndex;
    MBRPartition biosBootPartition;

    /* TODO: UEFI info */
    uint8_t uefiReserved[32];

    /* generic info */
    uint64_t kernelHighestAddress;
    uint64_t kernelTotalSize;
    
    uint64_t acpiRSDP;
    uint64_t highestPhysicalAddress;
    uint64_t memoryMap;
    uint8_t memoryMapSize;

    uint16_t width;
    uint16_t height;
    uint8_t bpp;
    uint64_t framebuffer;
    uint32_t pitch;
    uint8_t redPosition;
    uint8_t redMask;
    uint8_t greenPosition;
    uint8_t greenMask;
    uint8_t bluePosition;
    uint8_t blueMask;

    uint64_t ramdisk;           // pointer
    uint64_t ramdiskSize;

    uint8_t moduleCount;
    uint64_t modules[16];       // array of pointers
    uint64_t moduleSizes[16];

    uint64_t lowestFreeMemory;  // pointer to the end of highest ramdisk/module, aka lowest free memory

    char arguments[256];        // command-line arguments passed to the kernel
} __attribute__((packed)) KernelBootInfo;

#define BOOT_FLAGS_UEFI     0x01
#define BOOT_FLAGS_GPT      0x02

extern LXBootInfo bootInfo;
extern CPURegisters *biosRegs;

void videoAPI(CPURegisters *);
void diskAPI(CPURegisters *);

/* disk i/o */
extern int partitionIndex;      // boot partition
int readSectors(void *, uint32_t, int, uint8_t);
int findBootPartition();
uint32_t getPartitionStart(uint8_t, int);

/* configuration, modules, and ramdisk */
#define CONFIG_MAX_NAME         32
#define CONFIG_MAX_KERNEL       32
#define CONFIG_MAX_ARGUMENTS    256
#define CONFIG_MAX_MODULES      1024

typedef struct {
    size_t size;
    int count;      // how many boot options
    
    // these represent a selection
    char disk[CONFIG_MAX_NAME];
    char name[CONFIG_MAX_NAME];
    char kernel[CONFIG_MAX_KERNEL];
    char ramdisk[CONFIG_MAX_KERNEL];
    char arguments[CONFIG_MAX_ARGUMENTS];
    char modules[CONFIG_MAX_MODULES];

    int moduleCount;
} BootConfig;

int loadConfig(const char *);
BootConfig *selectBootOption(int);

/* memory detection */
int detectMemory(uint64_t *);
extern MemoryMap memoryMap[];

/* long mode setup */
void pagingSetup();
void lmode(uint32_t, uint32_t, KernelBootInfo *);
