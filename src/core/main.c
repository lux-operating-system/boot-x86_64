/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <string.h>

LXBootInfo bootInfo;

int main(LXBootInfo *boot) {
    memcpy(&bootInfo, boot, sizeof(LXBootInfo));

    print("hello world from a C program!");

    return 0;
}