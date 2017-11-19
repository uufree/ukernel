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

enum PoolFlags
{
    PF_KERNEL_VIRTUAL = 1,
    PF_KERNEL_PHYSICAL = 2,
    PF_USER_VIRTUAL = 3,
    PF_USER_PHYSICAL = 4
};

static const int PG_SIZE = 4096;

//VirtualPool data and operator
struct VirtualPool
{
    Bitmap bitmap;
    uint32_t addrStart;
    uint32_t poolSize;
};

//PhysicalPool data and operator
struct PhysicalPool
{
    Bitmap bitmap;
    uint32_t addrStart;
    uint32_t poolSize;
};

void initVirtualPool(struct VirtualPool* pool_,void* bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_);
void initPhysicalPool(struct PhysicalPool* pool,void* bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_);

uint32_t getPoolAddr(struct Memory* memory,enum PoolFlags flag,uint32_t count);

#endif
