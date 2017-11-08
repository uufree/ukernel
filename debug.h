/*************************************************************************
	> File Name: debug.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月08日 星期三 22时47分10秒
 ************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

void panicSpin(char* filename,int line,const char* func,const char* condition);

#define PANIC(...) panicSpin(__FILE__,__LINE__,__func__,__VA_ARGS__)

#ifdef NDEBUG
    #define ASSERT(CONDITION) (void(0))
#else
    #define ASSERT(CONDITION) PANIC(#CONDITION) 
#endif

#endif
