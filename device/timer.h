/*************************************************************************
	> File Name: timer.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月08日 星期三 20时51分08秒
 ************************************************************************/

#ifndef _TIMER_H
#define _TIMER_H

#include"stdint.h"

#define IRQ0_FREQUENCY 100
#define INPUT_FREQUENCY 1193180
#define COUNTER0_VALUE INPUT_FREQUENCY / IRQ0_FREQUENCY
#define CONTRER0_PORT 0x40
#define COUNTER0_NO 0
#define COUNTER_MODE 2
#define READ_WRITE_LATCH 3
#define PIT_CONTROL_PORT 0x43

extern "C"
{
    void timerInit();
}

#endif
