%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
   
jmp loader_start				
   
GDT_BASE:   dd    0x00000000 
	        dd    0x00000000

CODE_DESC:  dd    0x0000FFFF 
	        dd    DESC_CODE_HIGH4

DATA_STACK_DESC:  dd    0x0000FFFF
		          dd    DESC_DATA_HIGH4

VIDEO_DESC: dd    0x80000007	       
	        dd    DESC_VIDEO_HIGH4  

GDT_SIZE   equ   $ - GDT_BASE
GDT_LIMIT   equ   GDT_SIZE -	1 
times 60 dq 0					 

SELECTOR_CODE equ (0x0001<<3) + TI_GDT + RPL0         
SELECTOR_DATA equ (0x0002<<3) + TI_GDT + RPL0	 
SELECTOR_VIDEO equ (0x0003<<3) + TI_GDT + RPL0	  

gdt_ptr  dw  GDT_LIMIT 
	     dd  GDT_BASE
loadermsg db '2 loader in real.'

loader_start:
    mov sp,LOADER_BASE_ADDR
    mov bp,loadermsg
    mov cx,17
    mov ax,0x1301
    mov bx,0x0007
    mov dx,0x1800
    int 0x10

    mov ax,0xe801
    int 0x15

;低端15M内存
    mov cx,0x400
    mul cx
    shl edx,16
    and eax,0x0000ffff
    or edx,eax
    add edx,0x100000
    mov esi,edx

;16M以上
    xor eax,eax
    mov ax,bx
    mov ecx,0x10000
    mul ecx
    add esi,eax
    mov edx,esi

   ;-----------------  打开A20  ----------------
   in al,0x92
   or al,0000_0010B
   out 0x92,al

   ;-----------------  加载GDT  ----------------
   lgdt [gdt_ptr]


   ;-----------------  cr0第0位置1  ----------------
   mov eax, cr0
   or eax, 0x00000001
   mov cr0, eax

   jmp  SELECTOR_CODE:p_mode_start	

[bits 32]
p_mode_start:
   mov ax, SELECTOR_DATA
   mov ds, ax
   mov es, ax
   mov ss, ax
   mov esp,LOADER_STACK_TOP
   mov ax, SELECTOR_VIDEO
   mov gs, ax
  
   push edx    
   mov byte [gs:160], 'P'

   jmp $


setPage:
;将页目录表清0
    mov ecx,4096
    mov esi,0
 .clearDirPage:
    mov byte [PAGE_DIR_TABLE_POS + esi],0
    inc esi
    loop .clearDirPage

 .createPDE
    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x1000
    mov ebx,eax
    
    or eax,PG_US_U | PG_RW_W | PG_P
    mov [PAGE_DIR_TABLE_POS + 0x0],eax      ;设置第0个页目录项指向第一个页表
    mov [PAGE_DIR_TABLE_POS + 0xc00],eax    ;设置第768个页目录项指向第一个页表

    sub,eax,0x1000
    mov [PAGE_DIR_TABLE_POS + 4092],eax     ;设置最后一个页目录项指向页目录项自身

;创建低地址1M空间的地址映射到第一个页表中
    mov ecx,256
    mov esi,0
    xor edx,edx
    mov edx,PG_US_U | PG_RW_W | PG_P
 .createPTE
    mov [ebx + esi * 4],edx
    add edx,4096
    inc esi
    loop .createPTE
  
;将页目录表的768--1022表项指向固定的页表   
    mov eax,PAGE_DIR_TABLE_POS
    add eax,0x2000
    or eax,PG_US_U | PG_RW_W | PG_P
    mov ebx,PAGE_DIR_TABLE_POS
    mov ecx,254
    mov esi,769
 .createKernelPDE
    mov [ebx + esi * 4],eax
    inc esi
    add,eax,0x1000
    loop .createKernelPDE
