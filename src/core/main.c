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

LXBootInfo bootInfo;
CPURegisters *biosRegs;

int main(LXBootInfo *boot) {
    memcpy(&bootInfo, boot, sizeof(LXBootInfo));
    biosRegs = (CPURegisters *)boot->regs;

    printf("hello world from a C program!\n");
    printf("testing printf: %d 0x%08X\n", 12345, 0xABCDEF);
    printf("testing atoi: %d\n", atoi("123"));

    return 0;
}