
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

times 64 - ($-$$) db 0

; entry point
main:
    ; preserve information from the previous stage, again
    cli
    cld
    xor ax, ax
    mov es, ax
    mov di, boot_info
    mov [es:di], dl     ; boot disk
    inc di
    mov cx, 8
    rep movsw           ; partition

    mov ds, ax
    mov ax, 0x7E0
    mov ss, ax
    xor sp, sp          ; setup a large stack at 0x07E0:0xFFFF

    sti

    ; show signs of life
    mov si, life
    call print

    cli
    hlt

print:
    cld

    .loop:
        lodsb
        and al, al
        jz .end
        mov ah, 0x0E
        int 0x10
        jmp .loop
    
    .end:
        ret

%include "src/mode.asm"

life:                       db "lux boot program", 13, 10, 0

; this structure will be passed on to the next stage
boot_info:
    .disk           db 0

partition:
    .flags:         db 0
    .chs_start:     times 3 db 0
    .type:          db 0
    .chs_end:       times 3 db 0
    .start:         dd 0
    .size:          dd 0
