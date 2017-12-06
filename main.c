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
  
    for(int i=0;i<10;++i)
    {
        uint32_t* addr = mallocPageInKernelMemory(3);
        printStr((char*)"malloc addr in kernel: 0x");
        printInt((uint32_t)addr);
        printChar('\n');
    }
/*    
    uint32_t* addr1 = mallocPageInKernelMemory(3);
    printStr((char*)"malloc addr1 in kernel: 0x");
    printInt((uint32_t)addr1);
    printChar('\n');
    
    uint32_t* addr2 = mallocPageInKernelMemory(3);
    printStr((char*)"malloc addr2 in kernel: 0x");
    printInt((uint32_t)addr2);
    printChar('\n');
*/    
    while(1);

    return 0;
}
