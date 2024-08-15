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
static uint16_t *modes;
static CPURegisters regs;

VBEMode *vbeSetup() {
    // check if VESA BIOS is supported at all
    memcpy(&controller.signature, "VBE2", 4);
    regs.eax = 0x4F00;
    regs.edi = (uint32_t)&controller;
    videoAPI(&regs);

    if((biosRegs->eax & 0xFFFF) != 0x004F || memcmp(&controller.signature, "VESA", 4)) {
        printf("vbe: failed to query display controller, status 0x%04X\n", biosRegs->eax & 0xFFFF);
        while(1);
    }

    modes = (uint16_t *)((uint32_t)(controller.modeSegment << 4) + controller.modeOffset);

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

    VBEMode *mode = vbeSetMode(preferredWidth, preferredHeight, 32);
    if(mode) return mode;

    // here we somehow failed to set preferred resolution, so try again
    preferredWidth = 1024;
    preferredHeight = 768;
    mode = vbeSetMode(preferredWidth, preferredHeight, 32);
    if(mode) return mode;

    printf("vbe: failed to set screen resolution\n");
    while(1);
}

VBEMode *vbeSetMode(uint16_t w, uint16_t h, uint8_t bpp) {
    printf("vbe: attempt to set screen resolution %dx%dx%d\n", w, h, bpp);

    for(int i = 0; modes[i] != 0xFFFF; i++) {
        // query the BIOS for each mode one by one
        regs.eax = 0x4F01;
        regs.ecx = modes[i] & 0xFFFF;
        regs.edi = (uint32_t)&mode;
        videoAPI(&regs);

        if((biosRegs->eax & 0xFFFF) != 0x004F) {
            printf("vbe: failed to query mode 0x%04X, status 0x%04X\n", modes[i], regs.eax & 0xFFFF);
            return NULL;
        }

        //printf("vbe: mode 0x%04X: %dx%dx%d\n", modes[i], mode.width, mode.height, mode.bpp);

        if(mode.width == w && mode.height == h && mode.bpp == bpp) {
            // found the mode to use
            printf("vbe: found mode 0x%04X\n", modes[i]);
            
            regs.eax = 0x4F02;
            regs.ebx = modes[i] & ~(VBE_ENABLE_CRTC);
            regs.ebx |= VBE_ENABLE_LINEAR_FB;
            regs.edi = 0;
            videoAPI(&regs);

            if((biosRegs->eax & 0xFFFF) != 0x004F) {
                printf("vbe: failed to set mode 0x%04X, status 0x%04X\n", modes[i], regs.eax & 0xFFFF);
                return NULL;
            }

            return &mode;
        }
    }

    return NULL;
}