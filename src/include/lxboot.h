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

/* Boot Protocol */

/* this structure is passed from the earlier stage boot loader */
/* see main.asm for this */
typedef struct {
    uint8_t bootDevice;
    MBRPartition partition;
    uint32_t videoAPI;      /* callable routines for BIOS API functions */
    uint32_t diskAPI;
    uint32_t keyboardAPI;
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

extern LXBootInfo bootInfo;
extern CPURegisters *biosRegs;

void videoAPI(CPURegisters *);
void diskAPI(CPURegisters *);

/* disk i/o */
int readSectors(void *, uint32_t, int, uint8_t);
int findBootPartition();
uint32_t getPartitionStart(uint8_t, int);
