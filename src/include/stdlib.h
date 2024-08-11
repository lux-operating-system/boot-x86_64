/*
 * lux - a lightweight unix-like operating system
 * Omar Elghoul, 2024
 * 
 * Boot loader for the x86_64 architecture
 */

#pragma once

#define OCTAL       8
#define DECIMAL     10
#define HEX         16

char *itoa(int, char *, int);
