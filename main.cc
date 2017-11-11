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

extern void printStr(char* message);

namespace mm
{
    void printHello()
    {
        printStr((char*)"Hello World!\n");
    }
}

int main(void)
{
    printStr((char*)"Hello,Kernel!\n");
    mm::printHello();

    initAll();
    
    ASSERT(1==2);

    while(1);
    
    return 0;
}
