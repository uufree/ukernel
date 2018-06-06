/*************************************************************************
	> File Name: memory_pool.c
	> Author: uuchen
	> Mail: 1319081676@qq.com
	> Created Time: 2018年06月06日 星期三 16时57分11秒
 ************************************************************************/

#include"memory_pool.h"
#include"debug.h"
#include"memory.h"
#include"print.h"
#include"string.h"

void init_virtual_pool(struct VirtualPool* pool,uint32_t bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_)
{
    bitmap_init(&pool->bitmap,bitmapBaseAddr_,length_,PG_SIZE);
    pool->addr_start = addrStart_;
    pool->pool_size = poolSize_;
//    printStr((char*)"\nVirtualPool:\n");
//    printBitmapMessage(&pool->bitmap);
}

void init_physical_pool(struct PhysicalPool* pool,uint32_t bitmapBaseAddr_,uint32_t length_,uint32_t addrStart_,uint32_t poolSize_)
{
    bitmap_init(&pool->bitmap,bitmapBaseAddr_,length_,PG_SIZE);
    pool->addr_start = addrStart_;
    pool->pool_size = poolSize_;
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
uint32_t get_pool_addr(struct Memory* memory_,enum PoolFlags flag,uint32_t count)
{
    int idx = 0;
    uint32_t addr = 0;
    
    switch(flag)
    {
        case PF_KERNEL_VIRTUAL:
            idx = bitmap_scan(&memory_->kernel_memory.kernel_vir_pool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = memory_->memory_message.kernel_vir_start + idx * PG_SIZE;
            break;
        case PF_KERNEL_PHYSICAL:
//            printStr((char*)"1\n");
            idx = bitmap_scan(&memory_->kernel_memory.kernel_phy_pool.bitmap,count);
//            printStr((char*)"2\n");
            if(idx == -1)
                return 0;
            addr = memory_->memory_message.kernel_phy_start + idx * PG_SIZE;
            break;
        case PF_USER_VIRTUAL:
            break;
        case PF_USER_PHYSICAL:
            idx = bitmap_scan(&memory_->user_memory.user_phy_pool.bitmap,count);
            if(idx == -1)
                return 0;
            addr = memory_->memory_message.user_phy_start + idx * PG_SIZE;
            break;
        default:
            
            break;
    }
    
    return addr;
}


