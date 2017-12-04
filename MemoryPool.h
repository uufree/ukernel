/*************************************************************************
	> File Name: MemoryPool.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月18日 星期六 12时18分08秒
 ************************************************************************/

#ifndef _MEMORYPOOL_H
#define _MEMORYPOOL_H

#include"print.h"
#include"bitmap.h"
#include"stdint.h"

extern struct Memory memory;

//各种内存池的标志，用于在统一的函数下分配内存
enum PoolFlags
{
    PF_KERNEL_VIRTUAL,
    PF_KERNEL_PHYSICAL,
    PF_USER_VIRTUAL,
    PF_USER_PHYSICAL
};

//页大小
static const int PG_SIZE = 4096;

//虚拟内存池
struct VirtualPool
{
    struct Bitmap bitmap;
    uint32_t addrStart;
    uint32_t poolSize;
};

//物理内存池
struct PhysicalPool
{
    struct Bitmap bitmap;
    uint32_t addrStart;
    uint32_t poolSize;
};

void initVirtualPool(struct VirtualPool* pool_,void* bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_);
void initPhysicalPool(struct PhysicalPool* pool,void* bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_);

uint32_t getPoolAddr(struct Memory* memory_,enum PoolFlags flag,uint32_t count);

#endif
