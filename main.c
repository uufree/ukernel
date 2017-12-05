/*************************************************************************
	> File Name: main.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年10月30日 星期一 20时05分36秒
 ************************************************************************/

#include"print.h"
#include"interrupt.h"
#include"init.h"
#include"debug.h"
#include"memory.h"

int main(void)
{
    printStr((char*)"Hello,Kernel!\n");
    initAll();
        
    uint32_t* addr = mallocPageInKernelMemory(3);
    printStr((char*)"malloc addr in kernel: 0x");
    printInt(*addr);
    
    while(1);

    return 0;
}
