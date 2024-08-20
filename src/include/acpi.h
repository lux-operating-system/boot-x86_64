/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#pragma once

#include <stdint.h>

#define RSDP_LEGACY_SIZE        20
#define RSDP_EXTENDED_SIZE      (sizeof(ACPIRSDP) - RSDP_LEGACY_SIZE)

typedef struct {
    // revision == 0
    char signature[8];      // 'RSD PTR '
    uint8_t checksum;
    char oem[6];
    uint8_t revision;
    uint32_t rsdt;

    // revision >= 2
    uint32_t length;
    uint64_t xsdt;
    uint8_t extendedChecksum;
    uint8_t reserved[3];
} __attribute__((packed)) ACPIRSDP;

ACPIRSDP *findACPIRoot();
