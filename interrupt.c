/*************************************************************************
	> File Name: interrupt.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 21时10分13秒
 ************************************************************************/

#include"io.h"
#include"stdint.h"
#include"global.h"
#include"interrupt.h"
#include"print.h"

static struct InterDesc IDT[IDT_DESC_COUNT];

//引用extern "C"
extern void IDTInit();

void PICInit()
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

static inline void makeIDTDesc(struct InterDesc* desc,uint8_t attr,handleInter function)
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
    PICInit();

    uint64_t IDTOperator = ((sizeof(IDT) - 1) | (((uint64_t)(uint32_t)IDT << 16)));
    asm volatile ("lidt %0" : : "m"(IDTOperator));
    printStr((char*)"IDT Init Done!\n");
}










