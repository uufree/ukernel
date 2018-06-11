#include "interrupt.h"
#include "stdint.h"
#include "global.h"
#include "io.h"
#include "print.h"

#define PIC_M_CTRL 0x20	/*这里用的可编程中断控制器是8259A,主片的控制端口是0x20*/
#define PIC_M_DATA 0x21	       /*主片的数据端口是0x21*/
#define PIC_S_CTRL 0xa0	       /*从片的控制端口是0xa0*/
#define PIC_S_DATA 0xa1	       /*从片的数据端口是0xa1*/

/*至少需要支持到int 0x80*/
#define IDT_DESC_CNT 0x81      /*目前总共支持的中断数*/

#define EFLAGS_IF   0x00000200       /*eflags寄存器中的if位为1*/
#define GET_EFLAGS(EFLAG_VAR) asm volatile("pushfl; popl %0" : "=g" (EFLAG_VAR))

extern uint32_t syscall_handler(void);

/*中断门描述符结构体*/
struct gate_desc 
{
    uint16_t    func_offset_low_word;
    uint16_t    selector;
    uint8_t     dcount;   
    uint8_t     attribute;
    uint16_t    func_offset_high_word;
};

static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function);

/*中断描述符表*/
static struct gate_desc idt[IDT_DESC_CNT];
/*中断名称*/
char* intr_name[IDT_DESC_CNT];		    
/*中断程序处理数组*/
intr_handler idt_table[IDT_DESC_CNT];
/*在kernel.asm中定义的中断入口地址*/
extern intr_handler intr_entry_table[IDT_DESC_CNT];	    

/* 初始化可编程中断控制器8259A */
static void pic_init(void) 
{
    /*初始化主片 */
    outb (PIC_M_CTRL, 0x11);   /*ICW1: 边沿触发,级联8259, 需要ICW4.*/
    outb (PIC_M_DATA, 0x20);   /*ICW2: 起始中断向量号为0x20,也就是IR[0-7] 为 0x20 ~ 0x27*/
    outb (PIC_M_DATA, 0x04);   /*ICW3: IR2接从片.*/ 
    outb (PIC_M_DATA, 0x01);   /*ICW4: 8086模式, 正常EOI*/

    /*初始化从片 */
    outb (PIC_S_CTRL, 0x11);   /*ICW1: 边沿触发,级联8259, 需要ICW4.*/
    outb (PIC_S_DATA, 0x28);   /*ICW2: 起始中断向量号为0x28,也就是IR[8-15] 为 0x28 ~ 0x2F*/
    outb (PIC_S_DATA, 0x02);   /*ICW3: 设置从片连接到主片的IR2引脚*/
    outb (PIC_S_DATA, 0x01);   /*ICW4: 8086模式, 正常EOI*/
   
   /*只打开时钟中断,其它全部关闭 */
   /*打开时钟中断主要是为了线程调度*/
    outb (PIC_M_DATA, 0xfe);
    outb (PIC_S_DATA, 0xff);
    
    /*打开硬盘相应中断*/
    outb(PIC_M_DATA,0xf8);
    outb(PIC_S_DATA,0xbf);

    print_str((char*)"   pic_init done\n");
}

/* 创建中断门描述符 */
static void make_idt_desc(struct gate_desc* p_gdesc, uint8_t attr, intr_handler function) 
{ 
    /*因为没有成熟的地址保护，可以直接做内存地址到uint32_t的转换*/
    p_gdesc->func_offset_low_word = (uint32_t)function & 0x0000FFFF;
    p_gdesc->selector = SELECTOR_K_CODE;
    p_gdesc->dcount = 0;
    p_gdesc->attribute = attr;
    p_gdesc->func_offset_high_word = ((uint32_t)function & 0xFFFF0000) >> 16;
}

/*初始化中断描述符表*/
static void idt_desc_init(void) 
{
    int lastindex = IDT_DESC_CNT - 1;
    for(int i = 0; i < IDT_DESC_CNT; i++)
        make_idt_desc(&idt[i], IDT_DESC_ATTR_DPL0, intr_entry_table[i]); 
    
    /*为int 0x80准备单独的系统调用*/
    /*单独处理系统调用,系统调用对应的中断门dpl为3*/
    make_idt_desc(&idt[lastindex], IDT_DESC_ATTR_DPL3,syscall_handler);
    print_str((char*)"   idt_desc_init done\n");
}

/* 通用的中断处理函数,一般用在异常出现时的处理 */
static void general_intr_handler(uint8_t vec_nr) 
{
    /*处理中断芯片上伪中断情形，无需处理，直接返回*/
    if(vec_nr == 0x27 || vec_nr == 0x2f)
        return;		

    /*将光标置为0,从屏幕左上角清出一片打印异常信息的区域,方便阅读 */
    set_cursor(0);
    int cursor_pos = 0;
    while(cursor_pos < 320) 
    {
        print_char(' ');
        cursor_pos++;
    }

    set_cursor(0);	 /*重置光标为屏幕左上角*/
    print_str((char*)"!!!!!!!      excetion message begin  !!!!!!!!\n");
    set_cursor(88);	 /*从第2行第8个字符开始打印*/
    print_str(intr_name[vec_nr]);
    
    /*若为pa个fault，将缺失的地址打印出来*/
    /*之后可以围绕着这一块做修改*/
    if(vec_nr == 14) 
    {	
        int page_fault_vaddr = 0; 
        /*cr2存放着缺失的地址*/
        asm ("movl %%cr2, %0" : "=r" (page_fault_vaddr));
        print_str((char*)"\npage fault addr is ");print_int(page_fault_vaddr); 
    }
    print_str((char*)"\n!!!!!!!      excetion message end    !!!!!!!!\n");
    while(1);
}

/*完成一般中断处理函数注册及异常名称注册 */
static void exception_init(void) 
{
    for (int i = 0; i < IDT_DESC_CNT; i++) 
    {

        idt_table[i] = general_intr_handler;/*假设所有的中断函数为通用处理*/
        intr_name[i] = (char*)"unknown";	/*调整所有的中断情形为unknown*/ 
    }

    intr_name[0] = (char*)"#DE Divide Error";
    intr_name[1] = (char*)"#DB Debug Exception";
    intr_name[2] = (char*)"NMI Interrupt";
    intr_name[3] = (char*)"#BP Breakpoint Exception";
    intr_name[4] = (char*)"#OF Overflow Exception";
    intr_name[5] = (char*)"#BR BOUND Range Exceeded Exception";
    intr_name[6] = (char*)"#UD Invalid Opcode Exception";
    intr_name[7] = (char*)"#NM Device Not Available Exception";
    intr_name[8] = (char*)"#DF Double Fault Exception";
    intr_name[9] = (char*)"Coprocessor Segment Overrun";
    intr_name[10] = (char*)"#TS Invalid TSS Exception";
    intr_name[11] = (char*)"#NP Segment Not Present";
    intr_name[12] = (char*)"#SS Stack Fault Exception";
    intr_name[13] = (char*)"#GP General Protection Exception";
    /*之后着重处理Page-Fault*/
    intr_name[14] = (char*)"#PF Page-Fault Exception";
    // intr_name[15] 第15项是intel保留项，未使用
    intr_name[16] = (char*)"#MF x87 FPU Floating-Point Error";
    intr_name[17] = (char*)"#AC Alignment Check Exception";
    intr_name[18] = (char*)"#MC Machine-Check Exception";
    intr_name[19] = (char*)"#XF SIMD Floating-Point Exception";
}

enum intr_status intr_enable() 
{
    enum intr_status old_status;
    if(INTR_ON == intr_get_status()) 
    {
        old_status = INTR_ON;
        return old_status;
    } 
    else 
    {
        old_status = INTR_OFF;
        asm volatile("sti");	 /*开中断*/
        return old_status;
    }
}

enum intr_status intr_disable() 
{     
    enum intr_status old_status;
    if(INTR_ON == intr_get_status()) 
    {
        old_status = INTR_ON;
        asm volatile("cli" : : : "memory"); /*关中断*/
        return old_status;
    } 
    else 
    {
        old_status = INTR_OFF;
        return old_status;
    }
}

enum intr_status intr_set_status(enum intr_status status) 
{return status & INTR_ON ? intr_enable() : intr_disable();}

enum intr_status intr_get_status() 
{
    uint32_t eflags = 0; 
    GET_EFLAGS(eflags);
    return (EFLAGS_IF & eflags) ? INTR_ON : INTR_OFF;
}

void register_handler(uint8_t vector_no, intr_handler function) 
{idt_table[vector_no] = function;}

void idt_init() 
{
    print_str((char*)"idt_init start\n");
    idt_desc_init();	   /*初始化中断描述符表*/
    exception_init();	   /*异常名初始化并注册通常的中断处理函数*/
    pic_init();		       /*初始化8259A*/

    /*加载idt*/
    uint64_t idt_operand = ((sizeof(idt) - 1) | ((uint64_t)(uint32_t)idt << 16));
    asm volatile("lidt %0" : : "m" (idt_operand));
    print_str((char*)"idt_init done\n");
}
