/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <stdio.h>
#include <string.h>

MemoryMap memoryMap[32];
static CPURegisters regs;

void miscAPI(CPURegisters *r) {
    void (*m)(CPURegisters *) = (void (*))bootInfo.miscAPI;
    memcpy(biosRegs, r, sizeof(CPURegisters));
    m(biosRegs);
}

int detectMemory(uint64_t *highest) {
    int c = 0;
    uint64_t addr = 0;

    regs.ebx = 0;

    for(c = 0; c < 32; c++) {
        regs.eax = 0xE820;
        regs.edx = 0x534D4150;
        regs.ecx = 24;
        regs.edi = (uint32_t)(&memoryMap[c]);
        miscAPI(&regs);

        if(biosRegs->eax != 0x534D4150 || biosRegs->eflags & 1) {
            if(c) {
                break;
            } else {
                printf("unable to detect memory\n");
                while(1);
            }
        }

        if((biosRegs->ecx & 0xFF) < 24) {
            memoryMap[c].acpiAttributes = MEMORY_ATTRIBUTES_VALID;
        }

        regs.ebx = biosRegs->ebx;
        if(!regs.ebx) break;
    }

    printf("memory map contains %d entries\n", c);

    for(int i = 0; i < c; i++) {
        if((memoryMap[i].base + memoryMap[i].len) > addr) {
            addr = memoryMap[i].base + memoryMap[i].len;
        }
    }

    *highest = addr;
    return c;
}

