/*************************************************************************
	> File Name: debug.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月08日 星期三 22时52分23秒
 ************************************************************************/

#include"stdint.h"
#include"debug.h"
#include"print.h"
#include"interrupt.h"

extern void panicSpin(char* filename,int line,const char* func,const char* condition);

void panicSpin(char* filename,int line,const char* func,const char* condition)
{
    interDisable();
    printStr((char*)"\n\n\n----!!!ERROR!!!----\n\n\n");
    printStr((char*)"Filename: ");
    printStr(filename);
    printChar('\n');
    
    printStr((char*)"Line: ");
    printInt(line);
    printChar('\n');
    
    printStr((char*)"Function: ");
    printStr(const_cast<char*>(func));
    printChar('\n');
    
    printStr((char*)"Condition: ");
    printStr(const_cast<char*>(condition));
    printChar('\n');

    while(1);
}


