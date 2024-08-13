
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; bios.asm: BIOS API Calls from Protected Mode

[bits 32]

; Generic API wrapper that self-modifies

bios_api:
    pop eax
    mov [return_backup], eax
    call rmode

[bits 16]

    mov esi, registers
    mov eax, [esi]
    mov ebx, [esi+4]
    mov ecx, [esi+8]
    mov edx, [esi+12]
    mov edi, [esi+20]
    mov esi, [esi+16]

    clc

.m:   db 0xCD, 0x00   ; int instruction for self-modifying code that saves space

    mov ebp, registers
    mov [ebp], eax
    mov [ebp+4], ebx
    mov [ebp+8], ecx
    mov [ebp+12], edx
    mov [ebp+16], esi
    mov [ebp+20], edi
    pushfd
    pop eax
    mov [ebp+24], eax

    call pmode

[bits 32]

    mov eax, [return_backup]
    jmp eax

.stack:             dd 0
.frame:             dd 0

; void video_api()

video_api:
    pusha
    mov [stack_backup], esp
    mov [frame_backup], ebp

    mov di, bios_api.m
    mov byte [di+1], 0x10        ; int 0x10

    call bios_api

    mov esp, [stack_backup]
    mov ebp, [frame_backup]
    popa
    ret

; void disk_api()

disk_api:
    pusha
    mov [stack_backup], esp
    mov [frame_backup], ebp

    mov di, bios_api.m
    mov byte [di+1], 0x13        ; int 0x13

    call bios_api

    mov esp, [stack_backup]
    mov ebp, [frame_backup]
    popa
    ret

; void disk_api()

misc_api:
    pusha
    mov [stack_backup], esp
    mov [frame_backup], ebp

    mov di, bios_api.m
    mov byte [di+1], 0x15        ; int 0x15

    call bios_api

    mov esp, [stack_backup]
    mov ebp, [frame_backup]
    popa
    ret

align 4
stack_backup:           dd 0
frame_backup:           dd 0
return_backup:          dd 0

align 4
registers:
    .eax            dd 0
    .ebx            dd 0
    .ecx            dd 0
    .edx            dd 0
    .esi            dd 0
    .edi            dd 0
    .eflags         dd 0