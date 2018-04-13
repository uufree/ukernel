/*************************************************************************
	> File Name: interrupt.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 21时10分13秒
 ************************************************************************/

#include"interrupt.h"

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define EFLAGS_IF 0x00000200
#define IDT_DESC_COUNT 0x21

static inline uint32_t GET_FLAGS()
{
    uint32_t flags;
    asm volatile ("pushfl;popl %0": "=g"(flags));
    return flags;
}

struct InterDesc
{
    uint16_t funcOffsetLowWord;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t funcOffsetHighWord;
};


extern HandleInter InterEntryTable[IDT_DESC_COUNT];//和中断相关的数组
char* interName[IDT_DESC_COUNT];
HandleInter IDTTable[IDT_DESC_COUNT];
static struct InterDesc IDT[IDT_DESC_COUNT];

static void handleGeneralInter(uint8_t vec)
{
    if(vec==0x27 && vec==0x2f)
        return;

    printStr((char*)"\n\n******************************\n\n");
    printStr((char*)"vec: 0x");
    printInt(vec);
    printStr(interName[vec]);
    printStr((char*)"\n");
    printStr((char*)"\n\n******************************\n\n");
    
    while(1);
}

void exceptionInit()
{
    for(int i=0;i<IDT_DESC_COUNT;++i)
    {
        IDTTable[i] = (void*)handleGeneralInter;
        interName[i] = (char*)"unknow";
    }
    
    interName[0] = (char*)"#DE Divide Error";
    interName[1] = (char*)"#DB Debug Exception";
    interName[2] = (char*)"NMI Interrupt";
    interName[3] = (char*)"#BP BreakPoit Exception";
    interName[4] = (char*)"#OF Overflow Exception";
    interName[5] = (char*)"#BR BOUND Range Exceeded Exception";
    interName[6] = (char*)"#UD Invalied Opcode Exception";
    interName[7] = (char*)"#NM Decive Not Avaliable Exception";
    interName[8] = (char*)"#DF Double Fault Exception";
    interName[9] = (char*)"Coprocessor Segement Overrun";
    interName[10] = (char*)"#TS Invalid TSS Exception";
    interName[11] = (char*)"#NP Segement Not Present";
    interName[12] = (char*)"#SS Stack Fault Exception";
    interName[13] = (char*)"#GP General Protection Exception";
    interName[14] = (char*)"#PF Page-Fault Exception";
//    interName[15] = (char*)"#MF Divide Error";
    interName[16] = (char*)"#MF x87 FPU Floating-Point Error";
    interName[17] = (char*)"#AC Aligment Check Exception";
    interName[18] = (char*)"#MC Machine-Check Exception";
    interName[19] = (char*)"#XF SIMD Floating-Point Exception";
}

static void PICInit()
{
    outb(PIC_M_CTRL,0x11);
    outb(PIC_M_DATA,0x20);
    outb(PIC_M_DATA,0x04);
    outb(PIC_M_DATA,0x01);

    outb(PIC_S_CTRL,0x11);
    outb(PIC_S_DATA,0x28);
    outb(PIC_S_DATA,0x02);
    outb(PIC_S_DATA,0x01);

    outb(PIC_M_DATA,0xfe);
    outb(PIC_S_DATA,0xff);

    printStr((char*)"PIC Init Done!\n");
}

static inline void makeIDTDesc(struct InterDesc* desc,uint8_t attr,HandleInter function)
{
    desc->funcOffsetLowWord = (uint32_t)function & 0x0000FFFF;
    desc->selector = SELECTOR_K_CODE;
    desc->dcount = 0;
    desc->attribute = attr;
    desc->funcOffsetHighWord = ((uint32_t)function & 0xFFFF0000) >> 16;
}

void IDTDescInit()
{
    for(int i=0;i<IDT_DESC_COUNT;++i)
        makeIDTDesc(&IDT[i],IDT_DESC_ATTR_DPL0,InterEntryTable[i]);
    
    printStr((char*)"IDT Desc Init Done!\n");
}

void IDTInit()
{
    printStr((char*)"IDT Init Start!\n");
    IDTDescInit();
    exceptionInit();
    PICInit();

    uint64_t IDTOperator = ((sizeof(IDT) - 1) | (((uint64_t)(uint32_t)IDT << 16)));
    asm volatile ("lidt %0" : : "m"(IDTOperator));
    printStr((char*)"IDT Init Done!\n");
}

enum InterStatus interEnable()
{
    enum InterStatus oldStatus;
    if(INTER_ON == interGetStatus())
    {
        oldStatus = INTER_ON;
    }
    else
    {
        oldStatus = INTER_OFF;
        asm volatile("sti");   //开中断
    }
    return oldStatus;
}

enum InterStatus interDisable()
{
    enum InterStatus oldStatus;
    if(INTER_OFF == interGetStatus())
        oldStatus = INTER_OFF;
    else
    {
        oldStatus = INTER_ON;
        asm volatile ("cli" : : : "memory");
    }
    return oldStatus;
}

enum InterStatus interGetStatus()
{
    uint32_t flags = GET_FLAGS();
    return (flags & EFLAGS_IF) ? INTER_ON : INTER_OFF;
}

enum InterStatus interSetStatus(enum InterStatus status)
{
    return status & INTER_ON ? interEnable() : interDisable();
}

void registerInterHander(uint8_t vec,HandleInter func)
{
    IDTTable[vec] = func;
}

