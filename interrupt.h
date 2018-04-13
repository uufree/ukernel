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

typedef void* HandleInter;

enum InterStatus
{
    INTER_OFF,
    INTER_ON
};

void IDTInit();
enum InterStatus interGetStatus();
enum InterStatus interSetStatus(enum InterStatus status);
enum InterStatus interEnable();
enum InterStatus interDisable();
void registerInterHander(uint8_t vec,HandleInter func);

#endif
