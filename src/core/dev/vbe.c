/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <vbe.h>
#include <stdio.h>
#include <string.h>

VBEController controller;
VBEMode mode;
VBEMonitor monitor;
uint16_t *modes;
static CPURegisters regs;

int vbeSetup() {
    // check if VESA BIOS is supported at all
    memcpy(&controller.signature, "VBE2", 4);
    regs.eax = 0x4F00;
    regs.edi = (uint32_t)&controller;
    videoAPI(&regs);

    if((biosRegs->eax & 0xFFFF) != 0x004F || memcmp(&controller.signature, "VESA", 4)) {
        printf("vbe: failed to query display controller, status 0x%04X\n", biosRegs->eax & 0xFFFF);
        while(1);
    }

    // now attempt to get monitor info
    uint16_t preferredWidth, preferredHeight;
    regs.eax = 0x4F15;
    regs.ebx = 1;
    regs.ecx = 0;
    regs.edx = 0;
    regs.edi = (uint32_t)&monitor;
    videoAPI(&regs);

    if((biosRegs->eax & 0xFFFF) != 0x004F) {
        printf("vbe: failed to get monitor info, status 0x%04X\n", biosRegs->eax & 0xFFFF);
        preferredWidth = 1024;      // dirty default but what else can we do atp
        preferredHeight = 768;
    } else {
        preferredWidth = monitor.timing[0].hActiveLow;
        preferredWidth |= (monitor.timing[0].hActiveBlankHigh & 0xF0) << 4;

        preferredHeight = monitor.timing[0].vActiveLow;
        preferredHeight |= (monitor.timing[0].vActiveBlankHigh & 0xF0) << 4;

        printf("vbe: preferred resolution is %dx%d\n", preferredWidth, preferredHeight);
    }

    return 0;
}