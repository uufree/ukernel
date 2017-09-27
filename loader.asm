%include "boot.asm"
section loader vstart=loaderBaseAddress

    mov byte [gs:0x10],'L'
    mov byte [gs:0x11],0x07
    mov byte [gs:0x12],'o'
    mov byte [gs:0x13],007
    mov byte [gs:0x14],'a'
    mov byte [gs:0x15],0x07
    mov byte [gs:0x16],'d'
    mov byte [gs:0x17],0x07
    mov byte [gs:0x18],'e'
    mov byte [gs:0x19],0x07
    mov byte [gs:0x1a],'r'
    mov byte [gs:0x1b],0x07

    jmp $
