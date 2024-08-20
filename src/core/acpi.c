/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <acpi.h>
#include <stdio.h>
#include <string.h>
#include <stddef.h>

static ACPIRSDP *findRSDPRange(uint8_t *start, size_t count) {
    // 16-byte-aligned boundaries
    for(size_t i = 0; i < count; i += 16) {
        if(!memcmp(start + i, "RSD PTR ", 8)) {     // not strcmp because the null terminator isn't there
            return (ACPIRSDP *)(start + i);
        }
    }

    return NULL;
}

static ACPIRSDP *verifyChecksum(ACPIRSDP *rsdp) {
    uint8_t *bytes = (uint8_t *)rsdp;
    uint8_t v = 0;
    for(int i = 0; i < RSDP_LEGACY_SIZE; i++) {
        v += bytes[i];
    }

    if(!v) {
        printf("acpi: legacy checksum 0x%02X passed\n", rsdp->checksum);
    } else {
        printf("acpi: legacy checksum 0x%02X failed; ignoring ACPI tables\n", rsdp->checksum);
        return NULL;
    }

    if(rsdp->revision >= 2) {
        // additional verification for the extended fields
        for(int i = 0; i < RSDP_EXTENDED_SIZE; i++) {
            v += bytes[i + RSDP_LEGACY_SIZE];
        }

        if(!v) {
            printf("acpi: extended checksum 0x%02X passed\n", rsdp->extendedChecksum);
        } else {
            printf("acpi: extended checksum 0x%02X failed; ignoring ACPI tables\n", rsdp->extendedChecksum);
            return NULL;
        }
    }

    return rsdp;
}

ACPIRSDP *findACPIRoot() {
    // search for 'RSD PTR ' on 16-byte-aligned boundaries in the EBDA from
    // 0x80000-0x9FFFF, and if not found search again from 0xE0000-0xFFFFF
    ACPIRSDP *rsdp = findRSDPRange((uint8_t *)0x80000, 0x1FFFF);
    if(!rsdp) rsdp = findRSDPRange((uint8_t *)0xE0000, 0x1FFFF);
    if(rsdp) {
        printf("acpi: found RSDP revision %d at 0x%05X\n", rsdp->revision, (uint32_t)rsdp);
        rsdp = verifyChecksum(rsdp);
    } else {
        printf("acpi: RSDP was not found\n");
    }

    return rsdp;
}