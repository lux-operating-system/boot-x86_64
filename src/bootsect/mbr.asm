
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

    mov [boot_disk], dl
    sti

    ; check for a valid partition
    mov si, partitions
    mov cx, 4

    .loop:
        mov al, [si]
        test al, 0x80   ; bootable bit
        jnz .boot

        add si, 16
        dec cx
        jnz .loop

        mov si, no_active
        jmp print
    
    .boot:
        ; load the sector specified by the partition
        push si             ; preserve the partition
        mov di, dap
        mov eax, [si+8]     ; sector number
        mov [dap.lba], eax

        ; reset the drive
        clc
        xor ax, ax
        mov dl, [boot_disk]
        int 0x13
        jc .disk_error

        ; read sectors
        mov ah, 0x42
        mov si, dap
        mov dl, [boot_disk]
        int 0x13
        jc .disk_error

        ; done
        pop si
        mov dl, [boot_disk]

        jmp 0x0000:0x7C00
    
    .disk_error:
        mov si, disk_error
        jmp print

; print: prints a string and halts
; Parameters: ds:si = pointer to null-terminated string
; Returns: nothing

print:
    cld

    .loop:
        lodsb           ; AL = next byte
        test al, al     ; al == 0?
        jz .end
        
        mov ah, 0x0E    ; print character
        int 0x10
        jmp short .loop

    .end:
        sti
        hlt
        jmp .end

; disk address packet
align 4
dap:
    .size           db 0x10
    .reserved       db 0
    .count          dw 1
    .offset         dw 0x7C00
    .segment        dw 0x0000
    .lba            dq 0

boot_disk:          db 0

; strings
no_active:          db "no active partition found", 0
disk_error:         db "disk i/o error", 0

times 446 - ($-$$)  db 0

; partition table, this will be overwritten by the partition software
partitions:

    times 64 db 0

times 510 - ($-$$)  db 0
boot_signature:     dw 0xAA55
