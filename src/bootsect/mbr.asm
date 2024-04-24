
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; mbr.asm: Master Boot Record

[bits 16]
[org 0x100]

; copy the MBR away from 0x7C00 to free space for the partition's boot loder
cli
cld
xor ax, ax      ; a = 0
mov ds, ax
mov es, ax
mov si, 0x7C00  ; src = 0x7C00
mov di, 0x100   ; dst = 0x100
mov cx, 256     ; 512 bytes = 256 words
rep movsw
jmp 0x0000:main

main:
    mov ss, ax
    mov sp, 0x7C00  ; the stack grows downwards
    sti

    mov si, hello
    call print

    cli
    hlt

; print: prints a string
; Parameters: ds:si = pointer to null-terminated string
; Returns: nothing

print:
    pushf
    cld

    .loop:
        lodsb           ; AL = next byte
        test al, al     ; al == 0?
        jz .end
        
        mov ah, 0x0E    ; print character
        int 0x10
        jmp short .loop

    .end:
        popf
        ret

; strings
hello:              db "hello world!", 0

times 510 - ($-$$) db 0
boot_signature:     dw 0xAA55
