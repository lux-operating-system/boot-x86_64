
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

    ; reset the drive
    clc
    xor ah, ah
    int 0x13
    jc .drive_error

    ; load the boot program as specified by its size
    mov cl, [lxfs_id.flags]
    shr cl, 3
    and cl, 0x0F
    inc cl
    movzx ecx, cl           ; ecx = sectors per block

    mov eax, [partition.start]
    add eax, ecx
    mov [dap.lba], eax      ; start of the first block in sectors

    mov ax, 32              ; size of boot blocks
    mul cx                  ; blocks -> sectors

    cmp ax, 127             ; segmentation limit
    jl .setup

    mov ax, 127

.setup:
    mov [dap.count], ax

    ; and read the sectors
    clc
    mov ah, 0x42
    mov dl, [boot_disk]
    mov si, dap
    int 0x13
    jc .drive_error

    ; ensure validity of the program
    push ds
    mov ax, 0x50
    mov ds, ax
    xor si, si
    mov eax, [si]
    pop ds
    cmp eax, 0x5346584C     ; magic number
    jnz .boot_error

    ; now run the boot program
    mov si, partition
    mov dl, [boot_disk]
    jmp 0x0000:0x0540

    .drive_error:
        mov si, drive_error
        jmp print

    .boot_error:
        mov si, boot_error

print:
    cld

    .loop:
        lodsb
        and al, al
        jz .end

        mov ah, 0x0E
        int 0x10
        jmp short .loop
    
    .end:
        sti
        hlt
        jmp .end

; MBR partition entry
partition:
    .flags:         db 0
    .chs_start:     times 3 db 0
    .type:          db 0
    .chs_end:       times 3 db 0
    .start:         dd 0
    .size:          dd 0

boot_disk:          db 0

; disk address packet
align 4
dap:
    .size           db 0x10
    .reserved       db 0
    .count          dw 0
    .offset         dw 0x0000
    .segment        dw 0x0050
    .lba            dq 0

drive_error:        db "disk i/o error", 0
boot_error:         db "boot program invalid", 0

times 510 - ($-$$)  db 0
boot_signature:     dw 0xAA55
