
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
    mov ss, ax
    mov sp, 0x500

    sti

    mov ax, [.return]
    jmp ax

.return:                dw 0

[bits 32]

; lmode: switches the CPU to 64-bit long mode
; void lmode(uint32_t paging, uint32_t entry, KernelBootInfo *k)

align 4
lmode:
    pop eax                     ; discard return address as we will never return
    pop eax                     ; paging
    mov cr3, eax
    pop eax                     ; entry
    mov [.entry], eax
    pop eax                     ; k
    mov [.k], eax

    ; we'll need to switch back to 16-bit mode for a moment to notify the BIOS
    call rmode

[bits 16]

    mov eax, 0xEC00
    mov ebx, 2
    int 0x15

    ; now we can switch back to 32-bit mode and continue the setup
    call pmode

[bits 32]

    ; configure the CPU
    mov eax, 0x6A0              ; enable SSE, PAE, and global pages
    mov cr4, eax

    mov eax, cr0
    or eax, 0x00010000          ; enable write protection
    and eax, 0xBFFFFFFF         ; enable global caching
    mov eax, cr0

    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x100               ; enable 64-bit mode
    wrmsr

    mov eax, cr0
    or eax, 0x80000000          ; enable paging
    mov cr0, eax

    jmp 0x28:.next              ; reset code segment

[bits 64]

.next:
    ; by now we're in true 64-bit mode
    mov rax, 0x30
    mov ss, rax
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax
    mov rbx, 0xFFFFFFFF
    and rsp, rbx            ; zero extension

    mov eax, [.entry]
    and rax, rbx
    mov edi, [.k]
    and rdi, rbx

    cld
    call rax                ; kernel entry point taking KernelBootInfo * as a parameter

.hang:
    ; the kernel should never return
    cli
    hlt
    jmp .hang

align 4
.entry:                 dd 0
.k:                     dd 0

; Global Descriptor Table
align 16
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

    ; 64-bit code descriptor
    gdt_code64:
        .limit:         dw 0xFFFF
        .base_lo:       dw 0x0000
        .base_mi:       db 0x00
        .access:        db 0x9A     ; present, segment, executable, read access
        .flags:         db 0xAF     ; page granularity, 64-bit
        .base_hi:       db 0x00

    ; 64-bit data descriptor
    gdt_data64:
        .limit:         dw 0xFFFF
        .base_lo:       dw 0x0000
        .base_mi:       db 0x00
        .access:        db 0x92     ; present, segment, non-executable, write access
        .flags:         db 0xAF     ; page granularity, 64-bit
        .base_hi:       db 0x00

gdt_end:

align 16
gdtr:
    .limit:             dw gdt_end - gdt - 1
    .base:              dq gdt