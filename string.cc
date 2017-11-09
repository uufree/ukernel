/*************************************************************************
	> File Name: string.cc
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 01时38分18秒
 ************************************************************************/

#include"string.h"
#include"debug.h"

extern void memset(void* addr,uint8_t valus,uint32_t size);
extern void memcpy(void* addr,const void* src,uint32_t size);
extern uint8_t memcmp(const void* addr1,const void* addr2,uint32_t size);
    
extern void strcpy(char* addr,const char* src);
extern uint32_t strlen(const char* strlen);
extern uint8_t strcmp(const char* addr1,const char* addr2);
extern char* strchr(const char* addr,uint8_t ch);
extern char* strrchr(const char* addr,uint8_t ch);
extern char* strstr(const char* addr,const char* src);
extern void strcat(char* addr,const char* src);
extern uint32_t strchrs(const char* addr,uint8_t ch);

void memset(void* addr,uint8_t valus,uint32_t size)
{
    ASSERT(addr != nullptr);
    uint32_t* addr_ = (uint32_t)addr;
    while(--size)
        *addr_++ = valus; 
}

void memcpy(void* addr,const void* src,uint32_t size)
{
    ASSERT(addr != nullptr && src != nullptr);
    uint32_t* addr_ = (uint32_t)addr;
    const uint32_t* src_ = (uint32_t)src;
    
    while(--size)
        *addr_++ = *src_++;
}

uint8_t memcmp(const void* addr1,const void* addr2,uint32_t size)
{
    ASSERT(addr1 != nullptr && addr2 != nullptr);
    uint32_t* addr1_ = (uint32_t)addr1;
    uint32_t* addr2_ = (uint32_t)addr2;
    
    while(--size)
    {
        if(*addr1_ != *addr2_)
            return *addr1_ > *addr2_ ? 1 : -1;
        
        ++addr1_;
        ++addr2_;
    } 

    return 0;
}

void strcpy(char* addr,const char* src)
{
    ASSERT(addr != nullptr && src != nullptr);
    char* addr_ = addr;
    
    while((*addr_++ = *src++));
}

uint32_t strlen(const char* str)
{
    ASSERT(str != nullptr);
    const char* str_ = str;
    while(*str_++);
    return (str_ - str - 1);
}


