
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; mode.asm: CPU Mode Switch

[bits 16]

; pmode: switches the CPU to 32-bit protected mode
; params: none
; returns: nothing

pmode:
    pop ax      ; return address
    mov [.return], ax

    cli
    mov eax, gdtr
    lgdt [eax]

    mov eax, cr0
    or eax, 1
    mov cr0, eax

    jmp 0x08:.next

[bits 32]

.next:
    ; setup remaining segments
    mov eax, 0x10
    mov ss, eax
    mov ds, eax
    mov es, eax
    mov fs, eax
    mov gs, eax
    mov esp, 0x80000    ; 32-bit stack will be at 0x80000

    mov ax, [.return]
    movzx eax, ax
    jmp eax

.return:                dw 0

; rmode: switches the CPU to 16-bit real mode
; params: none
; returns: nothing

rmode:
    pop eax         ; return access
    mov [.return], ax

    jmp 0x18:.next

[bits 16]

.next:
    ; here we're in 16-bit protected mode
    mov eax, cr0
    and al, 0xFE
    mov cr0, eax
    jmp 0x0000:.rmode

.rmode:
    ; 16-bit real mode
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ax, 0x7E0
    mov ss, ax
    xor sp, sp

    sti

    mov ax, [.return]
    jmp ax

.return:                dw 0

; Global Descriptor Table
align 8
gdt:
    ; null descriptor
    gdt_null:
        dq 0

    ; 32-bit code descriptor
    gdt_code32:
        .limit:         dw 0xFFFF
        .base_lo:       dw 0x0000
        .base_mi:       db 0x00
        .access:        db 0x9A     ; present, segment, executable, read access
        .flags:         db 0xCF     ; page granularity, 32-bit
        .base_hi:       db 0x00

    ; 32-bit data descriptor
    gdt_data32:
        .limit:         dw 0xFFFF
        .base_lo:       dw 0x0000
        .base_mi:       db 0x00
        .access:        db 0x92     ; present, segment, non-executable, write access
        .flags:         db 0xCF     ; page granularity, 32-bit
        .base_hi:       db 0x00

    ; 16-bit code descriptor
    gdt_code16:
        .limit:         dw 0xFFFF
        .base_lo:       dw 0x0000
        .base_mi:       db 0x00
        .access:        db 0x9A     ; present, segment, executable, read access
        .flags:         db 0x0F     ; byte granularity, 16-bit
        .base_hi:       db 0x00

    ; 16-bit data descriptor
    gdt_data16:
        .limit:         dw 0xFFFF
        .base_lo:       dw 0x0000
        .base_mi:       db 0x00
        .access:        db 0x92     ; present, segment, non-executable, write access
        .flags:         db 0x0F     ; byte granularity, 16-bit
        .base_hi:       db 0x00

gdt_end:

align 8
gdtr:
    .limit:             dw gdt_end - gdt - 1
    .base:              dq gdt