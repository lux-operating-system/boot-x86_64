
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; bios.asm: BIOS API Calls from Protected Mode

[bits 32]

; void video_api(CPURegisters *r)

video_api:
    mov eax, [esp+4]
    mov [.regs], eax

    pusha
    mov [.stack], esp
    mov [.frame], ebp

    call rmode

[bits 16]

    mov esi, [.regs]
    mov eax, [esi]
    mov ebx, [esi+4]
    mov ecx, [esi+8]
    mov edx, [esi+12]
    mov edi, [esi+20]
    mov esi, [esi+16]

    int 0x10

    mov ebp, [.regs]
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

    mov esp, [.stack]
    mov ebp, [.frame]
    popa
    ret

.stack:             dd 0
.frame:             dd 0
.regs:              dd 0

; void disk_api(CPURegisters *r)

disk_api:
    mov eax, [esp+4]
    mov [.regs], eax

    pusha
    mov [.stack], esp
    mov [.frame], ebp

    call rmode

[bits 16]

    mov esi, [.regs]
    mov eax, [esi]
    mov ebx, [esi+4]
    mov ecx, [esi+8]
    mov edx, [esi+12]
    mov edi, [esi+20]
    mov esi, [esi+16]

    int 0x13

    mov ebp, [.regs]
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

    mov esp, [.stack]
    mov ebp, [.frame]
    popa
    ret

.stack:             dd 0
.frame:             dd 0
.regs:              dd 0
