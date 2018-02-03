/*************************************************************************
	> File Name: io.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月07日 星期二 21时15分21秒
 ************************************************************************/

#ifndef _IO_H
#define _IO_H

//方便向端口读写数据

#include"stdint.h"

static inline void outb(uint16_t port,uint8_t data)
{
    asm volatile ("outb %b0,%w1": :"a"(data),"Nd"(port));
}

static inline void outsw(uint16_t port,void* addr,uint32_t wordCount)
{
    asm volatile ("cld;rep outsw" : "+S"(addr),"+c"(wordCount) : "d"(port));
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t data;
    asm volatile ("inb %w1,%b0" : "=a"(data) : "Nd"(port));
    return data;
}

static inline void insw(uint16_t port,void* addr,uint32_t wordCount)
{
    asm volatile ("cld;rep insw" : "+D"(addr),"+c"(wordCount) : "d"(port) : "memory");
}

#endif
