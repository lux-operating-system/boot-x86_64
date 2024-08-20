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
#include <vbe.h>
#include <acpi.h>

#define KERNEL_BUFFER       (void *)0x100000

LXBootInfo bootInfo;
CPURegisters *biosRegs;
KernelBootInfo kernelBootInfo;

int main(LXBootInfo *boot) {
    memcpy(&bootInfo, boot, sizeof(LXBootInfo));
    biosRegs = (CPURegisters *)boot->regs;

    findBootPartition();
    uint64_t highestPhysicalAddress;
    int memoryMapSize = detectMemory(&highestPhysicalAddress);
    ACPIRSDP *rsdp = findACPIRoot();

    printf("loading kernel...\n");

    // TODO: read and parse the config file instead of hardcoding the kernel
    // this will have to be done in any case to implement module loading eventually
    if(!lxfsRead(bootInfo.bootDevice, partitionIndex, "/lux", KERNEL_BUFFER)) {
        printf("could not load /lux\n");
        while(1);
    }

    uint64_t kernelHighestAddress;
    uint32_t kernelEntry = (uint32_t)loadELF(KERNEL_BUFFER, &kernelHighestAddress);
    if(!kernelEntry) {
        printf("could not parse kernel executable\n");
        while(1);
    }

    // enable high resolution
    VBEMode *videoMode = vbeSetup();

    // this will be passed to the kernel so it has some info to start with
    kernelBootInfo.magic = 0x5346584C;
    kernelBootInfo.version = 1;
    kernelBootInfo.flags = 0;           // BIOS
    kernelBootInfo.biosBootDisk = bootInfo.bootDevice;
    kernelBootInfo.biosBootPartitionIndex = partitionIndex;
    memcpy(&kernelBootInfo.biosBootPartition, &bootInfo.partition, sizeof(MBRPartition));
    kernelBootInfo.kernelHighestAddress = kernelHighestAddress;
    kernelBootInfo.kernelTotalSize = kernelHighestAddress - 0x200000;
    kernelBootInfo.acpiRSDP = (uint64_t)rsdp;
    kernelBootInfo.memoryMap = (uint64_t)memoryMap;
    kernelBootInfo.memoryMapSize = memoryMapSize;

    kernelBootInfo.width = videoMode->width;
    kernelBootInfo.height = videoMode->height;
    kernelBootInfo.bpp = videoMode->bpp;
    kernelBootInfo.framebuffer = videoMode->framebuffer;
    kernelBootInfo.redPosition = videoMode->redPosition;
    kernelBootInfo.redMask = videoMode->redMask;
    kernelBootInfo.greenPosition = videoMode->greenPosition;
    kernelBootInfo.greenMask = videoMode->greenMask;
    kernelBootInfo.bluePosition = videoMode->bluePosition;
    kernelBootInfo.blueMask = videoMode->blueMask;

    kernelBootInfo.ramdisk = 0;         // TODO
    kernelBootInfo.ramdiskSize = 0;     // TODO
    kernelBootInfo.moduleCount = 0;     // TODO
    kernelBootInfo.modules = 0;         // TODO
    kernelBootInfo.moduleSizes = 0;     // TODO
    memset(kernelBootInfo.arguments, 0, 256);

    // this has to be the LAST setup because of buffer overlaps and that we
    // have very limited memory at this stage
    pagingSetup();
    void (*lmode)(uint32_t, uint32_t, KernelBootInfo *) = (void (*))bootInfo.lmode;
    lmode(PAGING_BASE, kernelEntry, &kernelBootInfo);

    // the above function will never return
    return -1;
}