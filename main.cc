/*************************************************************************
	> File Name: main.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年10月30日 星期一 20时05分36秒
 ************************************************************************/

#include"print.h"

extern void printChar(uint8_t ch);
extern void printStr(char* message);
extern void printInt(uint32_t number);

int main(void)
{
    printChar('u');
    printChar('u');
    printChar('c');
    printChar('h');
    printChar('e');
    printChar('n');
    printChar('\n');

    printStr((char*)"Hello,Kernel!\n");
    
    printInt(67);

    while(1);
    
    return 0;
}
