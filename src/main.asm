
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; main.asm: Entry Point

[bits 16]
[org 0x500]

; LXFS Identification Block
lxfs_boot_id:
    .magic:                 dd 0x5346584C
    .architecture:          dd 2        ; x86_64
    .timestamp:             dq 0        ; the formatting utility will write this
    .description:           db "lux", 0
                            times 48 - ($-$$) db 0
    .reserved:              times 16 db 0

; entry point
main:
    ; show signs of life
    mov ah, 0x0E
    mov al, 'x'
    int 0x10

    cli
    hlt
