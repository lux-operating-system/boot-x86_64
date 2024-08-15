/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <lxboot.h>
#include <stdint.h>
#include <stdio.h>

#define PAGE_PRESENT            0x01
#define PAGE_RW                 0x02
#define PAGE_USER               0x04
#define PAGE_WRITE_THROUGH      0x08
#define PAGE_DISABLE_CACHE      0x10
#define PAGE_SIZE_EXTENSION     0x80

void pagingSetup() {
    // creates page tables starting at PAGING_BASE
    // identity-maps the first 1 GiB of memory to give the kernel a reasonable
    // amount of space to start with
    
    // x86_64 uses 4-levels of paging: PML4 -> PDP -> PD -> PT
    // each level contains 512 pointers to the next level
    // assuming standard 4 KiB pages:
    //  PT      512*4 KiB = 2048 KiB
    //  PD      512*2 MiB = 1 GiB
    //  PDP     512*1 GiB = 512 GiB
    //  PML4    512*512 GiB (not even gonna bother to calculate this)
    // this is only here to reduce my own confusion as i try to build this
    // 2 MiB pages would go in the PD, 1 GiB pages would go in the PDP, etc

    // we will build 1 PML4, 1 PDP, and 4 PDs using 2 MiB pages to map 4 GiB
    // we will omit the PT and leave it to the kernel to rebuild everything
    // TODO: check if ALL x86_64 CPUs are obligated to implement 2 MiB pages
    printf("paging: setup at 0x%08X\n", PAGING_BASE);

    uint64_t *pml4 = (uint64_t *)(PAGING_BASE);
    uint64_t *pdp = (uint64_t *)(PAGING_BASE + PAGE_SIZE);
    uint64_t *pd = (uint64_t *)(PAGING_BASE + (2*PAGE_SIZE));

    // start by clearing out everything
    for(int i = 0; i < 512; i++) {
        pml4[i] = 0;
        pdp[i] = 0;
        pd[i] = 0;
    }

    pml4[0] = (uint64_t)pdp | PAGE_PRESENT | PAGE_RW;

    for(int i = 0; i < 4; i++) {
        pdp[i] = (uint64_t)pd + (i * PAGE_SIZE) | PAGE_PRESENT | PAGE_RW;
    }

    uint64_t addr = 0;
    for(int i = 0; i < 2048; i++) {
        //pd[i] = (i * 0x200000);
        // commented the above to avoid 64-bit mult while still in 32-bit mode
        pd[i] = addr;
        pd[i] |= PAGE_PRESENT | PAGE_RW | PAGE_SIZE_EXTENSION;
        addr += 0x200000;
    }
}
