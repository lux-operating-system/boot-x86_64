
; lux - a lightweight unix-like operating system
; Omar Elghoul, 2024
; 
; Boot loader for x86_64
; mbr.asm: Master Boot Record

[bits 16]
[org 0x100]

; test sign of life
mov ah, 0x0E
mov al, 'x'
int 0x10

cli
hlt

times 510 - ($-$$) db 0
dw 0xAA55

