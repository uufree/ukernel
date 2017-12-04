/*************************************************************************
	> File Name: MemoryPool.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月18日 星期六 12时25分20秒
 ************************************************************************/

//VirtualPool operator

#include"MemoryPool.h"
#include"debug.h"
#include"memory.h"

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

uint32_t getPoolAddr(struct Memory* memory_,enum PoolFlags flag,uint32_t count)
{
    int idx = 0;
    uint32_t addr = 0;

    ASSERT(count < physicalPool.bitmap.length);
    ASSERT(count < virtualPool.bitmap.length);
    
    switch(flag)
    {
        case PF_KERNEL_VIRTUAL:
            idx = bitmapScan(&memory_->kernelMemory.kernelVPool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = memory_->memoryMeesage.kernelVirStart + idx * PG_SIZE;
            break;
        case PF_KERNEL_PHYSICAL:
            idx = bitmapScan(&memory_->kernelMemory.kernelPPool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = memory_->memoryMeesage.kernelPhyStart + idx * PG_SIZE;
            break;
        case PF_USER_VIRTUAL:
            break;
        case PF_USER_PHYSICAL:
            idx = bitmapScan(&memory_->userMemory.userPPool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = memory_->memoryMeesage.userPhyStart + idx * PG_SIZE;
            break;
        default:
            
            break;
    }
    
    return addr;
}

