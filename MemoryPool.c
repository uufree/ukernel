/*************************************************************************
	> File Name: MemoryPool.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月18日 星期六 12时25分20秒
 ************************************************************************/

//VirtualPool operator

#include"MemoryPool.h"
#include"debug.h"

void initVirtualPool(struct VirtualPool* pool,void* bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_)
{
    bitmapInit(&pool->bitmap,bitmapBaseAddr_,length_,PG_SIZE);
    pool->addrStart = addrStart_;
    pool->poolSize = poolSize_;
}

void initPhysicalPool(struct PhysicalPool* pool,void* bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_)
{
    bitmapInit(&pool->bitmap,bitmapBaseAddr_,length_,PG_SIZE);
    pool->addrStart = addrStart_;
    pool->poolSize = poolSize_;
}

uint32_t getPoolAddr(enum PoolFlags flag,uint32_t count)
{
    int idx = 0;
    uint32_t addr = 0;

    ASSERT(count < physicalPool.bitmap.length);
    ASSERT(count < virtualPool.bitmap.length);
    
    switch(flag)
    {
        case PF_KERNEL:
            break;
        case PF_USER:
            break;
        case PF_VIRTUAL:
            idx = bitmapScan(&virtualPool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = virtualPool.addrStart + idx * PG_SIZE;
            break;
        case PF_PHYSICAL:
            idx = bitmapScan(&physicalPool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = physicalPool.addrStart + idx * PG_SIZE;
            break;
        default:
            
            break;
    }
    
    return addr;
}

