/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <lxfs.h>
#include <elf.h>

#define KERNEL_BUFFER       (void *)0x100000

LXBootInfo bootInfo;
CPURegisters *biosRegs;
KernelBootInfo kernelBootInfo;

int main(LXBootInfo *boot) {
    memcpy(&bootInfo, boot, sizeof(LXBootInfo));
    biosRegs = (CPURegisters *)boot->regs;

    findBootPartition();
    detectMemory();
    pagingSetup();

    printf("loading kernel...\n");

    // TODO: read and parse the config file instead of hardcoding the kernel
    // this will have to be done in any case to implement module loading eventually
    if(!lxfsRead(bootInfo.bootDevice, partitionIndex, "/lux", KERNEL_BUFFER)) {
        printf("could not load /lux\n");
        while(1);
    }

    uint32_t kernelEntry = (uint32_t)loadELF(KERNEL_BUFFER);
    if(!kernelEntry) {
        printf("could not parse kernel executable\n");
        while(1);
    }

    void (*lmode)(uint32_t, uint32_t, KernelBootInfo *) = (void (*))bootInfo.lmode;
    lmode(PAGING_BASE, kernelEntry, &kernelBootInfo);

    return 0;
}