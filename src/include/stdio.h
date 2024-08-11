/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#pragma once

#include <stdarg.h>

void print(const char *);
int putchar(int);
int puts(const char *);
int printf(const char *, ...);
int vprintf(const char *, va_list);