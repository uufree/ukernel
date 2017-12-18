/*************************************************************************
	> File Name: string.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月09日 星期四 01时38分18秒
 ************************************************************************/

#include"string.h"

void* memset(void* addr,uint8_t valus,uint32_t size)
{
//    ASSERT(addr != NULL);
    uint8_t* addr_ = (uint8_t*)addr;
    while(size--)
        *addr_++ = valus; 

    return addr;
}

void* memcpy(void* addr,const void* src,uint32_t size)
{
//    ASSERT(addr != NULL && src != NULL);
    uint8_t* addr_ = (uint8_t*)addr;
    const uint8_t* src_ = (uint8_t*)src;
    
    while(size--)
        *addr_++ = *src_++;

    return addr;
}

uint8_t memcmp(const void* addr1,const void* addr2,uint32_t size)
{
//    ASSERT(addr1 != NULL && addr2 != NULL);
    uint32_t* addr1_ = (uint32_t*)addr1;
    uint32_t* addr2_ = (uint32_t*)addr2;
    
    while(size--)
    {
        if(*addr1_ != *addr2_)
            return -1;
        
        ++addr1_;
        ++addr2_;
    } 

    return 0;
}

char* strcpy(char* addr,const char* src)
{
//    ASSERT(addr != NULL && src != NULL);
    char* addr_ = addr;
    
    while((*addr_++ = *src++));

    return addr;
}

uint32_t strlen(const char* str)
{
//    ASSERT(str != NULL);
    const char* str_ = str;
    while(*str_++);
    return (str_ - str - 1);
}

uint8_t strcmp(const char* addr1,const char* addr2)
{
//    ASSERT(addr1 != NULL && addr2 != NULL);
    
    while(*addr1 == *addr2)
    {
        ++addr1;++addr2;
    }
    
    return *addr1 == 0 ? 0 : -1; 
}

char* strchr(const char* addr,uint8_t ch)
{
//    ASSERT(addr != NULL);
    while(*addr != 0)
    {
        if(*addr == ch)
            return (char*)addr;
        ++addr;
    }
    return 0;
}

char* strrchr(const char* addr,uint8_t ch)
{
//    ASSERT(addr != NULL);
    const char* lastChar = addr + strlen(addr);
    
    while(*lastChar != *addr)
    {
        if(*lastChar == ch)
            return (char*)lastChar;
        --lastChar;
    }
    return 0;
}

char* strstr(const char* addr,const char* src)
{
//    ASSERT(addr != NULL && src != NULL);
    while(*addr != 0)
    {
        if(*addr == *src)
        {
            int srcSize = strlen(src);
            char* addr_ = (char*)addr;
            char* src_ = (char*)src;

            for(int i=0;i<srcSize;++i)
            {
                if(*addr_ != *src_)
                    goto read;
            }
            return (char*)addr;
        }
read:
        ++addr;
    }

    return 0;
}

char* strcat(char* addr,const char* src)
{
//    ASSERT(addr != NULL && src != NULL);
    char* addr_ = addr;
    while(*addr_++);
    --addr_;
    while((*addr_++ = *src++));

    return addr;
}

uint32_t strchrs(const char* addr,uint8_t ch)
{
//    ASSERT(addr != NULL);
    uint32_t count = 0;
    const char* addr_ = addr;
    
    while(*addr_++ == ch)
        ++count;
    
    return count;
}

