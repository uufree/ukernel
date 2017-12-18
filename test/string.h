/*************************************************************************
	> File Name: string.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 01时17分44秒
 ************************************************************************/

#ifndef _STRING_H
#define _STRING_H

#include"stdint.h"

void* memset(void* addr,uint8_t valus,uint32_t size);
void* memcpy(void* addr,const void* src,uint32_t size);
uint8_t memcmp(const void* addr1,const void* addr2,uint32_t size);
    
char* strcpy(char* addr,const char* src);
uint32_t strlen(const char* str);
uint8_t strcmp(const char* addr1,const char* addr2);
char* strchr(const char* addr,uint8_t ch);
char* strrchr(const char* addr,uint8_t ch);
char* strstr(const char* addr,const char* src);
char* strcat(char* addr,const char* src);
uint32_t strchrs(const char* addr,uint8_t ch);

#endif
