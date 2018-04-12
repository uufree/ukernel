/*************************************************************************
	> File Name: init.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 22时57分56秒
 ************************************************************************/

#include"init.h"
#include"print.h"
#include"interrupt.h"
#include"timer.h"
#include"memory.h"
#include"thread.h"

void initAll()
{
    IDTInit();
    timerInit();
    initMemory();
    threadListInit();

    printStr((char*)"Init All Done!\n");
}


