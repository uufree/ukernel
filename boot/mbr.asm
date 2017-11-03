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
    

    mov eax,LOADER_START_SECTOR
    mov bx,LOADER_BASE_ADDR
    mov cx,4
    call readDisk
    jmp LOADER_BASE_ADDR + 0x300

readDisk:
    mov esi,eax
    mov di,cx

;设置要读取的扇区数
    mov dx,0x1f2
    mov al,cl
    out dx,al

    mov eax,esi
;设置LBA地址
    mov dx,0x1f3
    out dx,al

    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    shr eax,cl
    mov dx,0x1f5
    out dx,al

    shr eax,cl
    and al,0x0f
    or al,0xe0
    mov dx,0x1f6
    out dx,al
;向0x1f7端口写命令，0x20
    mov dx,0x1f7
    mov al,0x20
    out dx,al
;监测硬件状态
@notReady:
    nop 
    in al,dx
    and al,0x88
    cmp al,0x08
    jnz @notReady
;从0x1f0端口读取数据
    mov ax,di
    mov dx,256
    mul dx
    mov cx,ax

    mov dx,0x1f0
@goOnRead:
    in ax,dx
    mov [bx],ax
    add bx,2
    loop @goOnRead

    ret

message db "I'm mbr!Welcome come to ukernel!"
end:
    jmp near $
    times 510-($-$$) db 0
    db 0x55,0xaa

