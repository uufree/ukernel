%include"boot.inc"
    
section mbr vstart=0x7c00
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00
    mov ax,0xb800
    mov gs,ax

;清屏
    mov ax,0600h
    mov bx,0700h
    mov cx,0
    mov dx,184fh
    int 10h

;显示字符串
    mov si,message
    mov di,0
    mov cx,end-message
@g:
    mov al,[si]
    mov [gs:di],al
    inc di
    mov byte [gs:di],0x07
    inc di
    inc si
    loop @g
    

;    mov eax,LOADER_START_SECTOR
;    mov bx,LOADER_BASE_ADDR
;    mov cx,4
;    call readDisk
;    jmp LOADER_BASE_ADDR + 0x300

message db "I'm mbr!Welcome come to ukernel!"
end:
    jmp near $
    times 510-($-$$) db 0
    db 0x55,0xaa












