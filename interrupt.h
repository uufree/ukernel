/*************************************************************************
	> File Name: interrupt.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 21时07分31秒
 ************************************************************************/

#ifndef _INTERRUPT_H
#define _INTERRUPT_H

#include"stdint.h"

extern "C"
{
    typedef void* handleIntr;
    void idtInit();
}

#endif
