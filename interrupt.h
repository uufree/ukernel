/*************************************************************************
	> File Name: interrupt.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 21时07分31秒
 ************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include"stdint.h"
#include"io.h"
#include"global.h"
#include"print.h"
#include"thread.h"
#include"debug.h"

//和中断相关的设置

#define PIC_M_CTRL 0x20
#define PIC_M_DATA 0x21
#define PIC_S_CTRL 0xa0
#define PIC_S_DATA 0xa1

#define EFFLAGS_IF 0x00000200
#define IDT_DESC_COUNT 0x21

typedef void* HandleInter;

struct InterDesc
{
    uint16_t funcOffsetLowWord;
    uint16_t selector;
    uint8_t dcount;
    uint8_t attribute;
    uint16_t funcOffsetHighWord;
};

enum InterStatus
{
    INTER_OFF,
    INTER_ON
};

extern HandleInter InterEntryTable[IDT_DESC_COUNT];//和中断相关的数组
char* interName[IDT_DESC_COUNT];
HandleInter IDTTable[IDT_DESC_COUNT];
struct InterDesc IDT[IDT_DESC_COUNT];

void IDTInit();
enum InterStatus interGetStatus();
enum InterStatus interSetStatus(enum InterStatus status);
enum InterStatus interEnable();
enum InterStatus interDisable();
void registerInterHander(uint8_t vec,HandleInter func);

#endif
