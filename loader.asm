%include "boot.asm"
section loader vstart=loaderBaseAddress
loaderStackTop equ loaderBaseAddress

jmp loaderStart

GDT_BASE: 
    dd 0x00000000
    dd 0x00000000

DESC_CODE:
    dd 0x0000ffff
    dd DESC_CODE_HIGHT_4B

DESC_DATA:
    dd 0x0000ffff
    dd DESC_DATA_HIGHT_4B
    
GDT_SIZE equ $ - GDT_BASE
GDT_LIMIT equ GDT_SIZE - 1

times dq 0

SELECTOR_CODE equ (0x0001 << 3) + TI_GDT + RPL0
SELECTOR_DATA equ (0x0002 << 3) + TI_GDT + RPL0

GDT_PTR dw GDT_LIMIT
        dd GDT_BASE

loaderMessage db "loader in real"

loaderStart:
    mov sp,loaderBaseAddress 
    mov bp,loaderMessage
    mov cx,14
    mov ax,0x1301
    mov bx,0x0007
    mov dx,0x1800
    int 0x10

;准备进入保护模式
    in al,0x92
    or al,00000010b
    out 0x92,al

    lgdt [GDT_PTR]

    mov eax,cr0
    or eax,0x00000001
    mov cr0,eax

    jmp dword DESC_CODE:start


start:
    mov [gs:0x60],'P'
    mov [gs:0x61],0x07
































