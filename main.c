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
#include"thread.h"

void threadA(void* arg);
void threadB(void* arg);

int main(void)
{
    printStr((char*)"Hello,Kernel!\n");
    initAll();
  
    threadStart((char*)"threadA",31,threadA,(char*)"argA");     
    threadStart((char*)"threadB",10,threadB,(char*)"argB");
    
    interEnable();
    while(1)
        printStr((char*)"main");

    return 0;
}

void threadA(void* arg)
{
    char* name = arg;

    while(1)
        printStr(name);
}

void threadB(void* arg)
{
    char* name = arg;

    while(1)
        printStr(name);
}
