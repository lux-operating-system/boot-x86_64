
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; main.asm: Entry Point

[bits 16]
[org 0x1000]

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
    mov ss, ax
    mov sp, 0x1000

    sti

    ; show signs of life
    mov si, life
    call print

    ; make sure we have a CPU with the required features
    mov eax, 0x80000001
    cpuid
    test edx, 0x20000000    ; amd64 capability bit
    jz error.old_cpu
    test edx, 0x100000      ; no-execute bit
    jz error.no_nx
    ;test edx, 0x800         ; syscall
    ;jz error.no_syscall

    ; and at least 16 MB ram
    clc
    xor cx, cx
    xor dx, dx
    mov ax, 0xE801
    int 0x15
    jc error.no_memory

    jcxz .check_memory

    mov ax, cx
    mov bx, dx

.check_memory:
    and bx, bx              ; bx = memory pages above 16 MB
    jz error.no_memory

    ; enable a20 gate
    clc
    mov ax, 0x2402          ; query
    int 0x15
    jc .fast_a20

    and ah, ah
    jnz .fast_a20

    cmp al, 1               ; already enabled
    jz .a20_done

    ; not yet enabled, enable it now
    clc
    mov ax, 0x2401
    int 0x15
    jc .fast_a20

    and ah, ah
    jz .a20_done

.fast_a20:
    ; this is an alternative if the BIOS functions above don't work
    mov dx, 0x92
    in al, dx
    test al, 2
    jnz .a20_done       ; already enabled

    or al, 2
    out dx, al
    nop                 ; delay the CPU for a cycle

.a20_done:
    ; jump to the protected mode entry point
    call pmode

[bits 32]

    mov esi, boot_info
    jmp pmode_program

[bits 16]

; error handlers
error:

.no_memory:
    mov si, no_memory
    call print
    jmp .hang

.old_cpu:
    mov si, old_cpu
    call print
    jmp .hang

.no_nx:
    mov si, no_nx
    call print
    jmp .hang

.no_syscall:
    mov si, no_syscall
    call print

.hang:
    sti
    hlt
    jmp .hang

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
%include "src/bios.asm"

life:                       db "lux boot program", 13, 10, 0
no_memory:                  db "not enough memory present", 0
old_cpu:                    db "not a 64-bit cpu", 0
no_nx:                      db "cpu doesn't support pae/nx", 0
no_syscall:                 db "cpu doesn't support syscall/sysret instructions", 0

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

                    dd video_api        ; this will allow BIOS calls
                    dd disk_api
                    dd 0                ; keyboard_api
                    dd misc_api
                    dd lmode

                    dd registers

times 0x1000 - ($-$$) db 0                   ; pad out to 0x2000
pmode_program:      incbin "lxboot.core"
