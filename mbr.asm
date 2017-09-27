%include "boot.asm"
section mbr vstart=0x7c00

    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov fs,ax
    mov sp,0x7c00

;清屏
    mov ax,0x0600
    mov bx,0x0700
    mov cx,0
    mov dx,0x184f
    int 0x10

;获取光标位置    
    mov ah,3
    mov bh,0
    int 0x10

;打印字符串
    mov ax,message
    mov bp,ax
    mov cx,3
    mov ax,0x1301
    mov bx,0x0007
    int 0x10

    mov eax,loaderBaseAddress
    mov ebx,loaderStartSector
    mov cx,1
    call readDisk
    
    jmp loaderBaseAddress

;------------------------------------
;读取磁盘的数据
;------------------------------------

readDisk:
    mov esi,eax
    mov di,cx

;设置待操作的扇区数
    mov dx,0x1f2
    mov al,cl
    out dx,al

;往LBA写入扇区起始地址
    mov dx,0x1f3
    out dx,al

    mov cl,8
    shr eax,cl
    mov dx,0x1f4
    out dx,al

    shr eax,cl
    mov dx,0x1f5
    out dx,al


;往device寄存器写入LBA的后四位以及相应的设置
    shr eax,cl
    and al,0x0f
    or al,0xe0
    mov dx,0x1f6
    out dx,al

;往command写入启动命令
    mov dx,0x1f7
    mov al,0x20
    out dx,al

;观察硬件状态
    notReady:
        nop 
        in al,dx
        and al,0x88
        cmp al,0x08
        jnz notReady

;读取数据
    mov ax,di
    mov dx,256
    mul dx
    mov cx,ax

    mov dx,0x1f0
    
    goOnRead:
        in ax,dx
        mov [ebx],ax
        add bx,2
        loop goOnRead
    
    ret

    message db "MBR"
    times 510-($-$$) db 0
    db 0x55,0xaa
