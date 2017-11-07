/*************************************************************************
	> File Name: interrupt.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 21时07分31秒
 ************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include"stdint.h"

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

struct InterDesc
{
    uint16_t funcOffsetLowWord;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t funcOffsetHighWord;
};

extern "C"
{
    typedef void* handleInter;
    void IDTInit();
}

static const int IDT_DESC_COUNT = 0x21;
extern handleInter InterEntryTable[IDT_DESC_COUNT];

#endif
