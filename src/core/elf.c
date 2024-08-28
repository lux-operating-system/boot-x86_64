/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

/* Minimal implementation of the Executable and Linkable Format */
/* This is used to load the kernel's code and data into memory */

#include <elf.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/*
 * loadELF(): loads the sections of an ELF file
 * params: binary - pointer to the ELF header
 * params: highest - pointer to where to store kernel's highest address
 * returns: absolute address of the entry point, zero on fail
 */

uint64_t loadELF(const void *binary, uint64_t *highest) {
    uint8_t *ptr = (uint8_t *)binary;
    ELFFileHeader *header = (ELFFileHeader *)ptr;

    uint64_t addr = 0;

    if(header->magic[0] != 0x7F || header->magic[1] != 'E' ||
    header->magic[2] != 'L' || header->magic[3] != 'F') {
        printf("elf: file doesn't contain ELF magic number\n");
        return 0;
    }

    if(header->isaWidth != ELF_ISA_WIDTH_64) {
        printf("elf: file is not 64-bit\n");
        return 0;
    }

    if(header->isa != ELF_ARCHITECTURE_X86_64 || header->endianness != ELF_LITTLE_ENDIAN) {
        printf("elf: file is not an x86_64 binary\n");
        return 0;
    }

    if(header->type != ELF_TYPE_EXEC) {
        printf("elf: file is not executable\n");
        return 0;
    }

    // now load the segments
    if(!header->headerEntryCount) {
        printf("elf: file contains no program headers\n");
        return 0;
    }

    //printf("elf: total of %d %s present, loading...\n", header->headerEntryCount, header->headerEntryCount == 1 ? "segment" : "segments");
    ELFProgramHeader *prhdr = (ELFProgramHeader *)(ptr + header->headerTable);
    for(int i = 0; i < header->headerEntryCount; i++) {
        //printf(" %d: ", i);
        if(prhdr->segmentType == ELF_SEGMENT_TYPE_LOAD) {
            //printf("load %d file/%d memory -> 0x%08X", (uint32_t)prhdr->fileSize, (uint32_t)prhdr->memorySize, (uint32_t)prhdr->virtualAddress);

            // for now virtual=physical because we haven't yet enabled paging
            // for the same reason we're also ignoring the exec/read/write perms
            memset((void *)(uint32_t)prhdr->virtualAddress, 0, prhdr->memorySize);
            memcpy((void *)(uint32_t)prhdr->virtualAddress, (const void *)(uint32_t)(binary + prhdr->fileOffset), prhdr->fileSize);

            // take note of the highest address
            if((prhdr->virtualAddress + prhdr->memorySize) > addr) {
                addr = prhdr->virtualAddress + prhdr->memorySize;
            }
        } else {
            printf("unimplemented header type %d, aborting...\n", prhdr->segmentType);
            return 0;
        }

        //printf("\n");

        prhdr = (ELFProgramHeader *)(prhdr + header->headerEntrySize);
    }

    //printf("elf: entry point is at 0x%08X\n", header->entryPoint);
    //printf("elf: highest address used by kernel is at 0x%08X\n", addr);
    *highest = addr;
    return header->entryPoint;
}
