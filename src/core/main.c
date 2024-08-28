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

static uint64_t forcePageAlignment(uint64_t addr) {
    while(addr & 0xFFF) {
        addr++;
    }

    return addr;
}

int main(LXBootInfo *boot) {
    memcpy(&bootInfo, boot, sizeof(LXBootInfo));
    biosRegs = (CPURegisters *)boot->regs;

    findBootPartition();
    uint64_t highestPhysicalAddress;
    int memoryMapSize = detectMemory(&highestPhysicalAddress);
    ACPIRSDP *rsdp = findACPIRoot();

    /* load the config file */
    loadConfig("/lxboot.conf");

    // TODO: display a menu letting the user choose a boot option

    BootConfig *option = selectBootOption(0);
    if(!option) {
        printf("unable to select boot option\n");
        while(1);
    }

    printf("booting %s...\n", option->name);

    // load the kernel
    printf("loading kernel %s...\n", option->kernel);

    if(!lxfsRead(bootInfo.bootDevice, partitionIndex, option->kernel, KERNEL_BUFFER)) {
        printf("could not load %s\n", option->kernel);
        while(1);
    }

    uint64_t lowestUsableAddress, kernelHighestAddress;
    uint32_t kernelEntry = (uint32_t)loadELF(KERNEL_BUFFER, &kernelHighestAddress);
    if(!kernelEntry) {
        printf("could not parse kernel executable\n");
        while(1);
    }

    lowestUsableAddress = kernelHighestAddress;

    // load the ramdisk if present
    uint64_t ramdisk = 0;
    size_t ramdiskSize = 0;
    if(strlen(option->ramdisk)) {
        printf("loading ramdisk %s...\n", option->ramdisk);

        ramdisk = forcePageAlignment(lowestUsableAddress);
        if(!lxfsRead(bootInfo.bootDevice, partitionIndex, option->ramdisk, (void *)ramdisk)) {
            printf("could not load %s\n", option->ramdisk);
            while(1);
        }

        ramdiskSize = lxfsSize(bootInfo.bootDevice, partitionIndex, option->ramdisk);

        lowestUsableAddress = ramdisk + ramdiskSize;
    }

    // load modules if present
    char module[CONFIG_MAX_MODULES];
    uint64_t moduleAddress;
    uint64_t moduleSize;
    if(option->moduleCount) {
        for(int i = 0; i < option->moduleCount; i++) {
            if(!copyModule(module, option->modules, i)) {
                printf("failed to load modules\n");
                while(1);
            }

            printf("loading module %d of %d: %s...\n", i+1, option->moduleCount, module);
            
            moduleAddress = forcePageAlignment(lowestUsableAddress);

            strcpy((char *)moduleAddress, module);

            if(!lxfsRead(bootInfo.bootDevice, partitionIndex, module, (void *)moduleAddress + strlen(module) + 1)) {
                printf("could not load %s\n", module);
                while(1);
            }

            moduleSize = lxfsSize(bootInfo.bootDevice, partitionIndex, module);
            lowestUsableAddress = moduleAddress + moduleSize + strlen(module) + 1;

            kernelBootInfo.modules[i] = moduleAddress;
            kernelBootInfo.moduleSizes[i] = moduleSize;
        }
    }

    // enable high resolution
    VBEMode *videoMode = vbeSetup();

    // this will be passed to the kernel so it has some info to start with
    memset(&kernelBootInfo, 0, sizeof(KernelBootInfo));
    kernelBootInfo.magic = 0x5346584C;
    kernelBootInfo.version = 1;
    kernelBootInfo.flags = 0;           // BIOS
    kernelBootInfo.biosBootDisk = bootInfo.bootDevice;
    kernelBootInfo.biosBootPartitionIndex = partitionIndex;
    memcpy(&kernelBootInfo.biosBootPartition, &bootInfo.partition, sizeof(MBRPartition));
    kernelBootInfo.kernelHighestAddress = kernelHighestAddress;
    kernelBootInfo.kernelTotalSize = kernelHighestAddress - 0x200000;
    kernelBootInfo.acpiRSDP = (uint64_t)rsdp;
    kernelBootInfo.highestPhysicalAddress = highestPhysicalAddress;
    kernelBootInfo.memoryMap = (uint64_t)memoryMap;
    kernelBootInfo.memoryMapSize = memoryMapSize;

    kernelBootInfo.width = videoMode->width;
    kernelBootInfo.height = videoMode->height;
    kernelBootInfo.bpp = videoMode->bpp;
    kernelBootInfo.framebuffer = videoMode->framebuffer;
    kernelBootInfo.pitch = videoMode->pitch;
    kernelBootInfo.redPosition = videoMode->redPosition;
    kernelBootInfo.redMask = videoMode->redMask;
    kernelBootInfo.greenPosition = videoMode->greenPosition;
    kernelBootInfo.greenMask = videoMode->greenMask;
    kernelBootInfo.bluePosition = videoMode->bluePosition;
    kernelBootInfo.blueMask = videoMode->blueMask;

    kernelBootInfo.ramdisk = ramdisk;
    kernelBootInfo.ramdiskSize = ramdiskSize;

    kernelBootInfo.moduleCount = option->moduleCount;
    kernelBootInfo.lowestFreeMemory = forcePageAlignment(lowestUsableAddress);

    strcpy(kernelBootInfo.arguments, option->kernel);
    strcpy(kernelBootInfo.arguments + strlen(kernelBootInfo.arguments), " ");
    strcpy(kernelBootInfo.arguments + strlen(kernelBootInfo.arguments), option->arguments);

    // this has to be the LAST setup because of buffer overlaps and that we
    // have very limited memory at this stage
    pagingSetup();
    void (*lmode)(uint32_t, uint32_t, KernelBootInfo *) = (void (*))bootInfo.lmode;
    lmode(PAGING_BASE, kernelEntry, &kernelBootInfo);

    // the above function will never return
    return -1;
}