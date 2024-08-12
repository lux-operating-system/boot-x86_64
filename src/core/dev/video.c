/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <string.h>

void videoAPI(CPURegisters *regs) {
    void (*v)(CPURegisters *) = (void (*))bootInfo.videoAPI;
    memcpy(biosRegs, regs, sizeof(CPURegisters));
    v(biosRegs);
}