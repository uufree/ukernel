/*************************************************************************
	> File Name: timer.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月08日 星期三 20时52分16秒
 ************************************************************************/

#include"timer.h"
#include"print.h"
#include"io.h"

extern void timerInit();

static void frequencyInit(uint8_t port,uint8_t no,uint8_t rwl,uint8_t mode,uint8_t value)
{
    outb(PIT_CONTROL_PORT,(uint8_t)(no << 6 | rwl << 4 | mode << 1));
    outb(port,(uint8_t)value);
    outb(port,(uint8_t)value >> 8);
}

void timerInit()
{
    printStr((char*)"Timer Init Start!\n");
    frequencyInit(CONTRER0_PORT,COUNTER0_NO,READ_WRITE_LATCH,COUNTER_MODE,(uint8_t)COUNTER0_VALUE);
    printStr((char*)"Timer Init Done!\n");
}

