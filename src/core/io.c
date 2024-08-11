/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>

void videoAPI(CPURegisters *regs) {
    void (*v)(CPURegisters *) = (void (*))bootInfo.videoAPI;
    v(regs);
}

void print(const char *s) {
    CPURegisters regs;
    for(; *s; s++) {
        if(*s == '\n') {
            regs.eax = 0x0E0D;  // crlf
            videoAPI(&regs);
            regs.eax = 0x0E0A;
            videoAPI(&regs);
        } else {
            regs.eax = 0x0E00 | (*s & 0xFF);
            videoAPI(&regs);
        }
    }
}