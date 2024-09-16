/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

/* partial implementation of the standard C library for convenience */
/* really to help with debugging and to make this a little less obnoxious */

void *memcpy(void *dst, const void *src, size_t n) {
    uint8_t *dstc = (uint8_t *)dst;
    uint8_t *srcc = (uint8_t *)src;

    for(size_t i = 0; i < n; i++) {
        dstc[i] = srcc[i];
    }

    return dst;
}

size_t strlen(const char *s) {
    size_t i = 0;
    for(; *s; i++) {
        s++;
    }
    return i;
}

char *strcpy(char *dst, const char *src) {
    return (char *)memcpy(dst, src, strlen(src)+1);
}

char *itoa(int n, char *buffer, int radix) {
    if(!radix || radix > HEX) return NULL;

    if(!n) {
        buffer[0] = '0';
        buffer[1] = 0;
        return buffer;
    }

    int length = 0;

    while(n) {
        // convert digit by digit and then reverse the string
        int digit = n % radix;

        if(digit >= 10) {
            buffer[length] = 'a' + digit - 10;
        } else {
            buffer[length] = '0' + digit;
        }

        length++;
        n /= radix;
    }

    buffer[length] = 0;   // null terminator

    // now reverse the string
    if(length >= 2) {
        for(int i = 0; i < length/2; i++) {
            char tmp = buffer[i];
            buffer[i] = buffer[length-i-1];
            buffer[length-i-1] = tmp;
        }
    }

    return buffer;
}

int atoi(const char *s) {
    int v = 0;
    int len = 0;

    while(s[len] >= '0' && s[len] <= '9') {
        len++;      // didn't use strlen so we can only account for numerical characters
    }

    if(!len) return 0;
    if(len == 1) return (int) s[0] - '0';

    int multiplier = 1;
    for(int i = 1; i < len; i++) {
        multiplier *= 10;
    }

    for(int i = 0; i < len; i++) {
        int digit = s[i] - '0';
        v += (digit * multiplier);
        multiplier /= 10;
    }

    return v;
}

void *memset(void *dst, int v, size_t n) {
    uint8_t *dstc = (uint8_t *)dst;
    for(size_t i = 0; i < n; i++) {
        dstc[i] = v;
    }
    return dst;
}

int strcmp(const char *s1, const char *s2) {
    while(*s1 == *s2) {
        if(!*s1) return 0;

        s1++;
        s2++;
    }

    return *s1 - *s2;
}

int memcmp(const void *d1, const void *d2, size_t n) {
    uint8_t *d1c = (uint8_t *)d1;
    uint8_t *d2c = (uint8_t *)d2;

    for(size_t i = 0; i < n; i++) {
        if(*d1c != *d2c) {
            return *d1c - *d2c;
        }

        d1c++;
        d2c++;
    }

    return 0;
}