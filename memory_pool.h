/*************************************************************************
	> File Name: memory_pool.h
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 16时55分22秒
 ************************************************************************/

#ifndef _MEMORY_POOL_H
#define _MEMORY_POOL_H

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
    uint32_t addr_start;
    uint32_t pool_size;
};

//物理内存池
struct PhysicalPool
{
    struct Bitmap bitmap;
    uint32_t addr_start;
    uint32_t pool_size;
};

void init_virtual_pool(struct VirtualPool* pool_,uint32_t  bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_);
void init_physical_pool(struct PhysicalPool* pool,uint32_t  bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_);

uint32_t get_pool_addr(struct Memory* memory_,enum PoolFlags flag,uint32_t count);

#endif
