/*************************************************************************
	> File Name: debug.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月08日 星期三 22时47分10秒
 ************************************************************************/

#ifndef _DEBUG_H
#define _DEBUG_H

//实现assert的方式,宏中的数据全部为字符串，注意以字符串的方式解析就好喽

void panicSpin(char* filename,int line,const char* func,const char* condition);

#define PANIC(...) panicSpin(__FILE__,__LINE__,__func__,__VA_ARGS__)

#ifdef NDEBUG
    #define ASSERT(CONDITION) (void(0))
#else
    #define ASSERT(CONDITION) PANIC(#CONDITION) 
#endif

#endif
