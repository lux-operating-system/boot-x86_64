
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; stub.asm: Stub for main core component

[bits 32]

section .stub
global _start
_start:
    extern main

    cld
    push esi
    mov eax, main
    mov ebp, esp
    call eax

    cli
    hlt
