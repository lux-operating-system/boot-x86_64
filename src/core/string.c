/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <string.h>
#include <stdint.h>

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *dstc = (uint8_t *)dst;
    uint8_t *srcc = (uint8_t *)src;

    for(size_t i = 0; i < n; i++) {
        dstc[i] = srcc[i];
    }

    return dst;
}