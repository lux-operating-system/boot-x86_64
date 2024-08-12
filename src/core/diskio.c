/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <stdio.h>
#include <string.h>

#define DISK_BUFFER         0x100   // temporary buffer in low memory

DiskAddressPacket dap;
CPURegisters regs;
int partitionIndex;

void diskAPI(CPURegisters *r) {
    void (*d)(CPURegisters *) = (void (*))bootInfo.diskAPI;
    memcpy(biosRegs, r, sizeof(CPURegisters));
    d(biosRegs);
}

int readSectors(void *dst, uint32_t lba, int count) {
    dap.size = sizeof(DiskAddressPacket);
    dap.reserved = 0;

    for(int i = 0; i < count; i++) {
        dap.count = 1;
        dap.segment = 0;
        dap.offset = DISK_BUFFER;
        dap.lba = lba + i;

        regs.eax = 0x4200;
        regs.edx = bootInfo.bootDevice & 0xFF;
        regs.esi = (uint32_t)&dap;

        diskAPI(&regs);

        if(biosRegs->eflags & 1) {   /* CF indicates error */
            printf("disk i/o error on sector %d drive 0x%02X\n", lba+i, bootInfo.bootDevice);
            while(1);   // hang
        }

        memcpy(dst + (i * 512), (const void *)DISK_BUFFER, 512);
    }

    return count;
}

int findBootPartition() {
    // returns the zero-based index of the boot partition within the boot drive
    readSectors((void *)DISK_BUFFER, 0, 1);
    MBRPartition *partitions = (MBRPartition *)((uint8_t *)DISK_BUFFER + MBR_PARTITION_OFFSET);

    for(int i = 0; i < 4; i++) {
        if(partitions[i].start == bootInfo.partition.start) {
            //printf("found boot partition at index %d\n", i);
            partitionIndex = i;
            return i;
        }
    }

    printf("cannot find boot partition\n");
    while(1);
}