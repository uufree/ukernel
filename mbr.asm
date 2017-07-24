loaderBaseAddress equ 0x0900
loaderStartSection equ 0x02

SECTION mbr vstart=0x7c00
    
    mov ax,cs
    mov ds,ax
    mov es,ax
    mov ss,ax
    mov es,ax
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
    mov cx,10
    mov ax,0x1301
    mov bx,0x0007
    int 0x10

    mov bx,loaderBaseAddress
    mov eax,loaderStartSection
    mov cx,1
    call readDisk

    jmp loaderBaseAddress

readDisk:
    mov esi,eax
    mov di,cx

;1.设置要读取的扇区数
    mov dx,0x1f2
    mov al,cl
    out dx,al

    mov eax,esi

;2.将LBA地址存入0x1f3~0x1f6
    mov dx,0x1f3
    out dx,al

    mov dx,0x1f4
    mov cl,8
    shr eax,cl
    out dx,al

    mov dx,0x1f5
    shr eax,cl
    out dx,al

    mov dx,0x1f6
    shr eax,cl
    and al,0x0f
    or al,0xe0
    out dx,al

;3.向0x1f7端口写入读命令
    mov dx,0x1f7
    mov al,0x20
    out dx,al

;4.检测硬件状态
notReady:
    nop
    in al,dx
    and al,0x08
    jnz notReady

;5.从端口读数据
    mov dx,0x1f0
    mov ax,di
    mov dx,256
    mul dx
    mov cx,ax
    
read:
    in ax,dx
    mov [bx],ax
    add bx,2
    loop read

    ret

message db "hello,MBR!"
times 510-($-$$) db 0
    db 0x55,0xaa


