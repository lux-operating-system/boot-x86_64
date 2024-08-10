
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; bootsec.asm: Boot Sector for LXFS

[bits 16]
[org 0]

jmp short stub
times 4 - ($-$$) db 0

lxfs_id:
    .magic:         db "LXFS"
    .size:          dq 0        ; these values will be written by the formatter
    .root_block:    dq 0
    .flags:         db 0
    .version:       db 0x01
    .name:          times 16 db 0
    .reserved:      times 6 db 0

stub:
    ; preserve partition entry
    cli
    mov ax, 0x4000
    mov ss, ax
    xor sp, sp

    push ds
    push si

    ; relocate to 0x4000:0x0000
    cld
    xor bx, bx
    mov ds, bx
    mov di, bx
    mov si, 0x7C00
    mov es, ax      ; es = ax = 0x4000
    mov cx, 256
    rep movsw

    jmp 0x4000:main

main:
    ; restore MBR info
    pop si
    pop ds
    mov di, partition
    mov cx, 8
    rep movsw

    mov ds, ax      ; ds = ax = 0x4000
    mov [boot_disk], dl

    sti

    mov ah, 0x0E
    mov al, 'y'
    int 0x10

    cli
    hlt


; MBR partition entry
partition:
    .flags:         db 0
    .chs_start:     times 3 db 0
    .type:          db 0
    .chs_end:       times 3 db 0
    .start:         dd 0
    .size:          dd 0

boot_disk:          db 0

times 510 - ($-$$)  db 0
boot_signature:     dw 0xAA55
