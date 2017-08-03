SECTION loader vstart=0x0500
message1 db "I'm loader!"
    

    mov si,message1
    mov cx,11
show:
    mov al,[si]
    mov [es:di],al
    inc di
    mov byte [es:di],0x07
    inc di
    inc si
    loop show

jmp $

