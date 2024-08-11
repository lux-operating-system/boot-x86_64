/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <stdio.h>
#include <string.h>

DiskAddressPacket dap;
CPURegisters regs;

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
        dap.offset = 0x100;     // temporary buffer
        dap.lba = lba + i;

        regs.eax = 0x4200;
        regs.edx = bootInfo.bootDevice & 0xFF;
        regs.esi = (uint32_t)&dap;

        diskAPI(&regs);

        if(biosRegs->eflags & 1) {   /* CF indicates error */
            printf("disk i/o error on sector %d drive 0x%02X\n", lba+i, bootInfo.bootDevice);
            return i;
        }

        memcpy(dst + (i * 512), (const void *)0x100, 512);
    }

    return count;
}