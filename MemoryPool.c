/*************************************************************************
	> File Name: MemoryPool.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2017年11月18日 星期六 12时25分20秒
 ************************************************************************/

#include"MemoryPool.h"
#include"debug.h"
#include"memory.h"
#include"print.h"
#include"string.h"

void initVirtualPool(struct VirtualPool* pool,uint32_t bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_)
{
    bitmapInit(&pool->bitmap,bitmapBaseAddr_,length_,PG_SIZE);
    pool->addrStart = addrStart_;
    pool->poolSize = poolSize_;
//    printStr((char*)"\nVirtualPool:\n");
//    printBitmapMessage(&pool->bitmap);
}

void initPhysicalPool(struct PhysicalPool* pool,uint32_t bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_)
{
    bitmapInit(&pool->bitmap,bitmapBaseAddr_,length_,PG_SIZE);
    pool->addrStart = addrStart_;
    pool->poolSize = poolSize_;
//    printStr((char*)"\nPhysicalPool:\n");
//    printBitmapMessage(&pool->bitmap);
}

//2017.12.06
//bug1
//这块在分配内核物理地址的时候有问题..
//貌似是位图在分配之后没有及时的标记..
//建议重新测试下为位图的功能..
//测试方法为：首先测试模拟虚拟内存的分配，一次性分配三页
//其次模拟物理内存的分配，逐渐的分配三次..
//得出结论：位图搜索算法在获取1个位置的时候，会出现不标记的现象
//已解决..
//
//bug2
//多次分配kernelPhysicalMemory,第9次分配时会出错
//bitmapScan分配出错...
uint32_t getPoolAddr(struct Memory* memory_,enum PoolFlags flag,uint32_t count)
{
    int idx = 0;
    uint32_t addr = 0;
    
    switch(flag)
    {
        case PF_KERNEL_VIRTUAL:
            idx = bitmapScan(&memory_->kernelMemory.kernelVPool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = memory_->memoryMeesage.kernelVirStart + idx * PG_SIZE;
            break;
        case PF_KERNEL_PHYSICAL:
//            printStr((char*)"1\n");
            idx = bitmapScan(&memory_->kernelMemory.kernelPPool.bitmap,count);
//            printStr((char*)"2\n");
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

